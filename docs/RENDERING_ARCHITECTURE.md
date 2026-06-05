# EmberGUI Rendering Architecture

## Core Concepts

### Shape
Defines the **form** of a widget — no visual information whatsoever.

```cpp
RoundedRect { width, height, radius }
Rect        { width, height }
Circle      { radius }
```

### Material
Defines **what a surface looks like** — brush + effects only. No geometry knowledge.
A material is completely reusable across any shape.

```cpp
SolidMaterial    { color }
GradientMaterial { color_a, color_b, direction }
AnimatedGradientMaterial { color_a, color_b, speed, glow }
```

### Layer
Pairs a **geometry variant** (which part of the shape to draw) with a **Material** (how it looks).
This is the bridge between shape and material.

```
Layer { Fill,          SolidMaterial { white }         }  → paints the interior
Layer { Stroke(2px),   AnimatedGradientMaterial { ... } }  → paints the border ring
```

### Widget
Has a **Shape** and an ordered list of **Layers**.

```
button.shape  = RoundedRect { width:120, height:40, radius:8 }
button.layers = [
    Layer { Fill,        SolidMaterial { white }         },
    Layer { Stroke(2px), AnimatedGradientMaterial { ... } },
]
```

### Composite Material
A Material that contains an ordered list of (geometry-variant, sub-material) pairs.
Lets you define reusable multi-layer styles and assign them to any widget.

```
glowingBorder = CompositeMaterial {
    (Fill,        SolidMaterial { white }),
    (Stroke(2px), AnimatedGradientMaterial { blue, pink, glow }),
}

button.material = glowingBorder
panel.material  = glowingBorder   // same material, different shape
```

---

## The Rendering Pipeline

### Key insight: no intermediate textures
There is no CPU-side compositing. The GPU writes directly to the framebuffer.
The fragment shader runs at every pixel of a full quad covering the widget area.
At each pixel it answers two questions:
1. **Does this pixel survive?** → SDF test (fill or stroke)
2. **What color is it?** → brush formula

Layers compose through sequential draw calls. Alpha blending handles overlap.

### Stencil-then-cover (two draw calls per layer)

```
Pass 1 — Geometry:
  Renders the quad.
  Runs the SDF for the shape.
  Applies the variant test (fill or stroke).
  Writes to the stencil buffer where the test passes.
  Outputs no color.

Pass 2 — Brush:
  Renders the same quad.
  Reads the stencil — only pixels marked in pass 1 survive.
  Computes and outputs the brush color.
  Clears the stencil as it goes (SG_STENCILOP_ZERO).
```

This is what makes the architecture composable: geometry and brush are genuinely separate shaders that never need to know about each other.

---

## Shader Structure

Two shared shaders. Not one uber shader, not N×M shaders.

### geometry.glsl
Knows about shapes. Knows nothing about color.

```glsl
@fs fs_geometry
layout(binding=0) uniform shape_params {
    vec2  size;
    float radius;
    float stroke_width;
    int   shape;    // 0=rect, 1=roundrect, 2=circle
    int   variant;  // 0=fill, 1=stroke
};

in vec2 frag_uv;

float sdf_rect(vec2 p, vec2 h) {
    vec2 q = abs(p) - h;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}

float sdf_roundrect(vec2 p, vec2 h, float r) {
    vec2 q = abs(p) - h + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

float sdf_circle(vec2 p, float r) {
    return length(p) - r;
}

void main() {
    vec2 p = (frag_uv - 0.5) * size;

    float d;
    if      (shape == 0) d = sdf_rect(p, size * 0.5);
    else if (shape == 1) d = sdf_roundrect(p, size * 0.5, radius);
    else                 d = sdf_circle(p, min(size.x, size.y) * 0.5);

    if (variant == 0) {
        if (d > 0.0) discard;                          // fill
    } else {
        if (abs(d) > stroke_width * 0.5) discard;     // stroke
    }

    // write stencil — no color output
}
@end
```

### brush.glsl
Knows about color. Knows nothing about shapes.

```glsl
@fs fs_brush
layout(binding=0) uniform brush_params {
    vec4  color_a;
    vec4  color_b;
    float time;
    int   brush;    // 0=solid, 1=linear_gradient, 2=animated_gradient
};

in vec2 frag_uv;
out vec4 frag_color;

void main() {
    // stencil already filtered surviving pixels — just output color

    if (brush == 0) {
        frag_color = color_a;
    }
    else if (brush == 1) {
        frag_color = mix(color_a, color_b, frag_uv.x);
    }
    else {
        float t = fract(frag_uv.x - time);
        frag_color = mix(color_a, color_b, t);
    }
}
@end
```

### Composability table

| Shape      | Brush             | Geometry pass   | Brush pass        |
|------------|-------------------|-----------------|-------------------|
| RoundedRect fill  | Solid      | geometry(roundrect, fill)   | brush(solid)    |
| Circle     fill   | Gradient   | geometry(circle, fill)      | brush(gradient) |
| RoundedRect stroke| AnimGradient | geometry(roundrect, stroke) | brush(animated) |

Adding a new shape → one new SDF function in `geometry.glsl`.  
Adding a new brush → one new branch in `brush.glsl`.  
They never need to know about each other.

---

## SDF (Signed Distance Field)

A function that returns:
- **negative** → pixel is inside the shape
- **zero** → pixel is exactly on the edge
- **positive** → pixel is outside the shape

```
d = sdf_roundrect(pixel_position, half_size, radius)

Fill test:   d < 0          → inside
Stroke test: abs(d) < w/2   → near the edge (both sides)
```

The stroke uses the same SDF function as the fill. Only the test changes.

Anti-aliasing: instead of a hard discard, use `alpha = smoothstep(0.0, -aa_width, d)` to get a soft edge.

---

## Coordinate System

Vertex shader converts pixel coordinates to NDC (Normalized Device Coordinates, -1..1) using an orthographic projection matrix:

```glsl
layout(binding=0) uniform transform {
    mat4 projection;  // orthographic: pixel (0,0) top-left → NDC (-1,1)
};

void main() {
    gl_Position = projection * vec4(pos, 0.0, 1.0);
}
```

CPU side works in pixels. GPU side handles the conversion.

---

## Draw Call Flow (per frame, per widget)

```
for each widget:
    for each layer in widget.layers:
        // Pass 1: geometry → stencil
        sg_apply_pipeline(geometry_pipeline)
        sg_apply_uniforms(shape_slot, shape_params)
        sg_draw(quad)

        // Pass 2: brush → color
        sg_apply_pipeline(brush_pipeline)
        sg_apply_uniforms(brush_slot, brush_params)
        sg_draw(quad)
```

Each layer = 2 draw calls. A button with fill + animated border = 4 draw calls total.

---

## File Layout (planned)

```
Shaders/
    geometry.glsl       — all SDFs, fill/stroke variant
    brush.glsl          — solid, gradient, animated gradient

src/
    sokol_impl.cpp      — #define SOKOL_IMPL, sokol includes only (isolation)
    main.cpp            — sapp_desc, init/frame/cleanup
    ember/
        Renderer.h/cpp  — draw call API, pipeline management
        Painter.h/cpp   — high-level: draw_widget(widget)
        Material.h      — Material, CompositeMaterial types
        Shape.h         — Shape types (RoundedRect, Circle, Rect)
        Widget.h        — Widget = Shape + list of Layers
        Theme.h         — design tokens (colors, radii, fonts)
```
