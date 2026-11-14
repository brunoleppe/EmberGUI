#include "Layer.h"
#include "rlgl.h"

Layer CreateLayer(int width, int height)
{
    Layer layer;
    layer.rt = LoadRenderTexture(width, height);
    layer.width = width;
    layer.height = height;
    return layer;
}

void DestroyLayer(Layer& layer)
{
    UnloadRenderTexture(layer.rt);
    layer.width = layer.height = 0;
}

void BeginLayer(const Layer& layer)
{
    BeginTextureMode(layer.rt);
    ClearBackground(Color{0, 0, 0, 0});
}

void EndLayer()
{
    EndTextureMode();
}

void CompositeLayer(const Layer& layer, Rectangle dst)
{
    // Render textures are stored vertically flipped relative to a normally
    // loaded texture; a negative source height flips it back on draw.
    Rectangle src = {0, 0, (float)layer.rt.texture.width, -(float)layer.rt.texture.height};
    DrawTexturePro(layer.rt.texture, src, dst, Vector2{0, 0}, 0.0f, WHITE);
    rlDrawRenderBatchActive();
}