#version 330

// EmberGUI — rect SDF fragment shader (raylib custom shader)
//
// Used with raylib's default vertex shader: draw a plain quad via
// DrawRectangle(x, y, w, h, ...) with this shader bound, and set the
// uniforms below per-rect before each draw. fragTexCoord (0..1 across
// the quad, provided by raylib) gives us local rect-space position.

in vec2 fragTexCoord;
out vec4 finalColor;

uniform vec2  u_size;           // rect size in pixels
uniform vec4  u_radii;          // corner radii: TL TR BR BL
uniform vec4  u_color;          // fill RGBA, 0..1
uniform vec4  u_borderColor;    // border RGBA, 0..1
uniform float u_borderWidth;
uniform vec4  u_clip;           // local-space x,y,w,h; z<=0 disables clipping
uniform float u_shadowSoftness; // 0 = normal rect, >0 = shadow falloff

// ── SDF ──────────────────────────────────────────────────────────────────

float sdf_rounded_rect(vec2 p, vec2 half_size, float r) {
    vec2 q = abs(p) - half_size + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, vec2(0.0))) - r;
}

// Select the corner radius based on which quadrant p falls in.
// u_radii: x=TL  y=TR  z=BR  w=BL  (matches CSS order)
float sdf_rect(vec2 p, vec2 half_size, vec4 radii) {
    float r = p.x > 0.0
        ? (p.y < 0.0 ? radii.y : radii.z)   // right: TR or BR
        : (p.y < 0.0 ? radii.x : radii.w);  // left:  TL or BL
    return sdf_rounded_rect(p, half_size, r);
}

void main() {
    // fragTexCoord.y == 0 at the rect's top edge (raylib's default quad
    // convention), so this lines up with the TL/TR/BR/BL quadrant logic
    // above. If corners ever look swapped, flip this to (1.0 - texcoord.y).
    vec2 local_pos = fragTexCoord * u_size;

    // ── Clip rect (local space) ─────────────────────────────────────────
    if (u_clip.z > 0.0) {
        vec2 clip_min = u_clip.xy;
        vec2 clip_max = u_clip.xy + u_clip.zw;
        if (local_pos.x < clip_min.x || local_pos.y < clip_min.y ||
            local_pos.x > clip_max.x || local_pos.y > clip_max.y) {
            discard;
        }
    }

    // ── SDF ────────────────────────────────────────────────────────────
    vec2  half_size = u_size * 0.5;
    vec2  p         = local_pos - half_size;
    float dist      = sdf_rect(p, half_size, u_radii);
    float aa        = fwidth(dist);

    // ── Shadow mode ────────────────────────────────────────────────────
    // Shadow rects fade symmetrically across the SDF boundary (a cheap
    // stand-in for a gaussian blur) instead of a hard edge: opaque well
    // inside, transparent well outside, soft transition of width
    // ~2*softness straddling the rect's edge. The rect is expanded beyond
    // the widget bounds on the CPU side (spread) so the fade has room.
    if (u_shadowSoftness > 0.0) {
        float falloff = 1.0 - smoothstep(-u_shadowSoftness, u_shadowSoftness, dist);
        if (falloff < 0.001) discard;
        finalColor = vec4(u_color.rgb, u_color.a * falloff);
        return;
    }

    // ── Normal rect mode ───────────────────────────────────────────────
    float shape_mask = 1.0 - smoothstep(-aa, aa, dist);
    if (shape_mask <= 0.001) discard;

    // Fill layer — alpha can be 0 (outline-only rects)
    vec4 out_color = vec4(u_color.rgb, u_color.a * shape_mask);

    // Border layer — composited on top independently
    if (u_borderWidth > 0.0 && u_borderColor.a > 0.0) {
        float inner       = sdf_rect(p, half_size - u_borderWidth, u_radii);
        float border_mask = shape_mask * (1.0 - smoothstep(-aa, aa, -inner));
        float ba           = u_borderColor.a * border_mask;
        out_color.rgb      = mix(out_color.rgb, u_borderColor.rgb, ba);
        out_color.a        = out_color.a + ba * (1.0 - out_color.a);
    }

    finalColor = out_color;
}