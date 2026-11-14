#include "RectRenderer.h"
#include "rlgl.h"

void RectRenderer::init()
{
    Image img = GenImageColor(1, 1, WHITE);
    white_tex = LoadTextureFromImage(img);
    UnloadImage(img);

    shader = LoadShader(nullptr, "Shaders/rect.fs");
    loc_size = GetShaderLocation(shader, "u_size");
    loc_radii = GetShaderLocation(shader, "u_radii");
    loc_color = GetShaderLocation(shader, "u_color");
    loc_border_color = GetShaderLocation(shader, "u_borderColor");
    loc_border_width = GetShaderLocation(shader, "u_borderWidth");
    loc_clip = GetShaderLocation(shader, "u_clip");
    loc_shadow = GetShaderLocation(shader, "u_shadowSoftness");
}

void RectRenderer::draw(const UIRect& r) const
{
    float size[2] = {r.w, r.h};
    Vector4 color = ColorNormalize(r.color);
    Vector4 border_color = ColorNormalize(r.border_color);

    // clip[] is in the same space as x,y; the shader works in rect-local space.
    float clip_local[4] = {
        r.clip[0] - r.x, r.clip[1] - r.y, r.clip[2], r.clip[3]
    };

    SetShaderValue(shader, loc_size, size, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, loc_radii, r.radii, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, loc_color, &color, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, loc_border_color, &border_color, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, loc_border_width, &r.border_width, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_clip, clip_local, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, loc_shadow, &r.shadow_softness, SHADER_UNIFORM_FLOAT);

    Rectangle src = {0, 0, (float)white_tex.width, (float)white_tex.height};
    Rectangle dst = {r.x, r.y, r.w, r.h};
    DrawTexturePro(white_tex, src, dst, Vector2{0, 0}, 0.0f, WHITE);

    // Force this rect's draw call to execute now, with its own uniforms,
    // instead of being batched together with the next rect's (which
    // would overwrite these uniforms before the GPU actually draws).
    rlDrawRenderBatchActive();
}

void RectRenderer::shutdown() const
{
    UnloadShader(shader);
    UnloadTexture(white_tex);
}