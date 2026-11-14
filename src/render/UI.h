#pragma once
#include "raylib.h"
#include "UIRect.h"

class Effect;
struct Layer;

// The render layer's public facade. GUI logic talks to this and nothing else —
// no Texture2D or uniform location should ever need to leak past here. Effect
// instances (Effect.h) and the offscreen Layer (Layer.h) used while one is
// active are owned by whatever UI element applies them, not by this module.
namespace UI
{
    void Init();
    void Shutdown();

    void BeginFrame(Color clear_color);
    void EndFrame();

    void DrawRect(const UIRect& rect);

    // Renders subsequent DrawRect calls into `layer` (created/resized to
    // `bounds` on demand), then composites that layer back through `effect`
    // on EndEffect(). `layer` and `effect` must stay alive until the matching
    // EndEffect() call, and should persist across frames (own one per UI
    // element that needs an effect, rather than a fresh Layer each frame) so
    // a stable bounds size doesn't reallocate the render texture every call.
    // Rect/clip coordinates passed to DrawRect while an effect is active stay
    // in the same absolute space you'd normally use — the offset into the
    // layer is handled internally. Not reentrant: one active effect region at
    // a time for now.
    void BeginEffect(Rectangle bounds, Effect& effect, Layer& layer);
    void EndEffect();
}