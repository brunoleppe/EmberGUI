#include "UI.h"
#include "RectRenderer.h"
#include "Layer.h"
#include "Effect.h"
#include <cassert>

namespace
{
    RectRenderer g_rect_renderer;

    bool g_effect_active = false;
    Rectangle g_effect_bounds{};
    Effect* g_active_effect = nullptr;
    Layer* g_active_layer = nullptr;
    Vector2 g_offset{0, 0}; // subtracted from DrawRect coords while an effect is active
}

void UI::Init()
{
    g_rect_renderer.init();
    InitEffectKinds();
}

void UI::Shutdown()
{
    ShutdownEffectKinds();
    g_rect_renderer.shutdown();
}

void UI::BeginFrame(Color clear_color)
{
    BeginDrawing();
    ClearBackground(clear_color);
    BeginShaderMode(g_rect_renderer.shader);
}

void UI::EndFrame()
{
    EndShaderMode();
    EndDrawing();
}

void UI::DrawRect(const UIRect& rect)
{
    UIRect r = rect;
    r.x -= g_offset.x;
    r.y -= g_offset.y;
    r.clip[0] -= g_offset.x;
    r.clip[1] -= g_offset.y;
    g_rect_renderer.draw(r);
}

void UI::BeginEffect(Rectangle bounds, Effect& effect, Layer& layer)
{
    assert(!g_effect_active && "UI::BeginEffect is not reentrant — call EndEffect first");

    int w = (int)bounds.width;
    int h = (int)bounds.height;
    if (layer.width != w || layer.height != h)
    {
        if (layer.width > 0) DestroyLayer(layer);
        layer = CreateLayer(w, h);
    }

    g_effect_active = true;
    g_effect_bounds = bounds;
    g_active_effect = &effect;
    g_active_layer = &layer;
    g_offset = {bounds.x, bounds.y};

    BeginLayer(layer);
}

void UI::EndEffect()
{
    assert(g_effect_active && "UI::EndEffect called without a matching BeginEffect");

    EndLayer();
    g_effect_active = false;
    g_offset = {0, 0};

    BeginShaderMode(g_active_effect->kind().shader);
    g_active_effect->apply();
    CompositeLayer(*g_active_layer, g_effect_bounds);
    EndShaderMode();

    // EndShaderMode() reset the active program to raylib's default — restore
    // the rect shader so subsequent DrawRect calls this frame are correct.
    BeginShaderMode(g_rect_renderer.shader);
    g_active_effect = nullptr;
    g_active_layer = nullptr;
}