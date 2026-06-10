// EmberGUI — rect renderer shader
// Compile with:
//   sokol-shdc --input rect.glsl --output rect.h --slang glsl430:hlsl5:metal_macos

// ─────────────────────────────────────────────────────────────────────────────
// Shared uniform block — resolution, uploaded once per frame
// ─────────────────────────────────────────────────────────────────────────────
@block vs_uniforms
layout(binding=0) uniform vs_params {
    vec2 u_resolution;
};
@end

// ─────────────────────────────────────────────────────────────────────────────
// Vertex shader
// ─────────────────────────────────────────────────────────────────────────────
@vs vs
@include_block vs_uniforms

// Per-instance attributes — one RectInstance per rect
// Locations must match pipeline layout attrs[] indices in C
layout(location=0) in vec2  i_pos;           // x, y  (pixels)
layout(location=1) in vec2  i_size;          // w, h  (pixels)
layout(location=2) in vec4  i_radii;         // TL TR BR BL
layout(location=3) in vec4  i_color;         // RGBA  (ubyte4n → vec4)
layout(location=4) in vec4  i_border_color;  // RGBA  (ubyte4n → vec4)
layout(location=5) in float i_border_width;
layout(location=6) in vec4  i_clip;          // x y w h (pixels, 0-size = disabled)

// Outputs to fragment shader
out vec2  v_frag_pos;
out vec2  v_rect_pos;
out vec2  v_rect_size;
out vec4  v_radii;
out vec4  v_color;
out vec4  v_border_color;
out float v_border_width;
out vec4  v_clip;

void main() {
    // Generate quad corners from gl_VertexID (triangle strip: BL BR TL TR)
    vec2 corners[4];
    corners[0] = vec2(0.0, 1.0);
    corners[1] = vec2(1.0, 1.0);
    corners[2] = vec2(0.0, 0.0);
    corners[3] = vec2(1.0, 0.0);

    vec2 corner    = corners[gl_VertexIndex];
    vec2 pixel_pos = i_pos + corner * i_size;

    // Pixel coords → NDC. UI origin is top-left, Y flipped.
    vec2 ndc = (pixel_pos / u_resolution) * 2.0 - 1.0;
    ndc.y    = -ndc.y;

    gl_Position = vec4(ndc, 0.0, 1.0);

    v_frag_pos     = pixel_pos;
    v_rect_pos     = i_pos;
    v_rect_size    = i_size;
    v_radii        = i_radii;
    v_color        = i_color;
    v_border_color = i_border_color;
    v_border_width = i_border_width;
    v_clip         = i_clip;
}
@end

// ─────────────────────────────────────────────────────────────────────────────
// Fragment shader
// ─────────────────────────────────────────────────────────────────────────────
@fs fs
in vec2  v_frag_pos;
in vec2  v_rect_pos;
in vec2  v_rect_size;
in vec4  v_radii;
in vec4  v_color;
in vec4  v_border_color;
in float v_border_width;
in vec4  v_clip;

out vec4 frag_color;

// ── SDF ──────────────────────────────────────────────────────────────────────

float sdf_rounded_rect(vec2 p, vec2 half_size, float r) {
    vec2 q = abs(p) - half_size + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, vec2(0.0))) - r;
}

// Select the corner radius based on which quadrant p falls in.
// v_radii: x=TL  y=TR  z=BR  w=BL  (matches CSS order)
float sdf_rect(vec2 p, vec2 half_size, vec4 radii) {
    float r = p.x > 0.0
        ? (p.y < 0.0 ? radii.y : radii.z)   // right: TR or BR
        : (p.y < 0.0 ? radii.x : radii.w);  // left:  TL or BL
    return sdf_rounded_rect(p, half_size, r);
}

// ─────────────────────────────────────────────────────────────────────────────

void main() {
    // ── Clip rect ────────────────────────────────────────────────────────────
    // clip.zw == 0 means "no clip", otherwise test against the pixel position.
    if (v_clip.z > 0.0) {
        vec2 clip_min = v_clip.xy;
        vec2 clip_max = v_clip.xy + v_clip.zw;
        if (v_frag_pos.x < clip_min.x || v_frag_pos.y < clip_min.y ||
            v_frag_pos.x > clip_max.x || v_frag_pos.y > clip_max.y) {
            discard;
        }
    }

    // ── Rounded rect SDF ─────────────────────────────────────────────────────
    vec2  center    = v_rect_pos + v_rect_size * 0.5;
    vec2  half_size = v_rect_size * 0.5;
    vec2  p         = v_frag_pos - center;
    float dist      = sdf_rect(p, half_size, v_radii);
    float aa        = fwidth(dist);

    // Outer shape mask — 1 inside the rect, 0 outside, smooth at the edge
    float shape_mask = 1.0 - smoothstep(-aa, aa, dist);
    if (shape_mask <= 0.001) discard;

    // ── Fill layer ───────────────────────────────────────────────────────────
    // fill alpha can be 0 (transparent interior) — that's valid.
    vec4 out_color = vec4(v_color.rgb, v_color.a * shape_mask);

    // ── Border layer ─────────────────────────────────────────────────────────
    if (v_border_width > 0.0 && v_border_color.a > 0.0) {
        // Inner edge SDF — positive outside the inset rect, negative inside
        float inner = sdf_rect(p, half_size - v_border_width, v_radii);

        // Border ring = inside outer shape AND outside inner shape
        float border_mask = shape_mask * (1.0 - smoothstep(-aa, aa, -inner));

        // Composite border on top of fill (standard over operation)
        float ba      = v_border_color.a * border_mask;
        out_color.rgb = mix(out_color.rgb, v_border_color.rgb, ba);
        out_color.a   = out_color.a + ba * (1.0 - out_color.a);
    }

    frag_color = out_color;
}
@end

// ─────────────────────────────────────────────────────────────────────────────
@program rect vs fs
