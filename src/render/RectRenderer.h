#pragma once
#include "raylib.h"
#include "UIRect.h"

// Draws UIRects: one draw call per rect, uniforms set from UIRect before each draw.
// Caller is responsible for BeginShaderMode(shader)/EndShaderMode() around a batch
// of draw() calls (UI::BeginFrame/EndFrame does this).
struct RectRenderer
{
    Shader shader;
    Texture2D white_tex; // dedicated 1x1 texture so DrawTexturePro gives us 0..1 texcoords
    int loc_size, loc_radii, loc_color, loc_border_color, loc_border_width, loc_clip, loc_shadow;

    void init();
    void draw(const UIRect& r) const;
    void shutdown() const;
};