#pragma once
#include "raylib.h"

// An offscreen render target. Subsequent draw calls between BeginLayer/EndLayer
// land in the layer's texture instead of whatever target was active before;
// CompositeLayer pastes that texture back, optionally through a post-process shader.
struct Layer
{
    RenderTexture2D rt{};
    int width = 0;
    int height = 0;
};

Layer CreateLayer(int width, int height);
void DestroyLayer(Layer& layer);

void BeginLayer(const Layer& layer); // clears to transparent, redirects draws into it
void EndLayer();

// Pastes the layer's texture onto dst (in whatever target is currently active).
// Caller is responsible for BeginShaderMode/EndShaderMode around this call if a
// post-process effect shader should run during the paste; this just issues the
// textured draw (handling the render-texture vertical flip) and flushes it.
void CompositeLayer(const Layer& layer, Rectangle dst);