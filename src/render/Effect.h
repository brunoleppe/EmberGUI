#pragma once
#include "raylib.h"

// Flyweight: the shared GL resource for one *kind* of effect (compiled shader +
// uniform locations). Loaded once via init() and shared by every Effect
// instance that uses this kind. init()/shutdown() stay separate from a real
// ctor/dtor because kinds are constructed as global statics before the GL
// context exists (InitWindow runs after static initialization) — but the
// constructor itself just registers the kind (see InitEffectKinds below), so
// adding a new EffectKind subclass needs no other code changes.
class EffectKind
{
public:
    EffectKind(); // registers this instance with InitEffectKinds/ShutdownEffectKinds
    virtual ~EffectKind() = default;

    virtual void init() = 0;
    virtual void shutdown() = 0;

    Shader shader{};
};

class TintEffectKind final : public EffectKind
{
public:
    void init() override;
    void shutdown() override;

    int loc_tint = -1;
};

class RainbowEffectKind final : public EffectKind
{
public:
    void init() override;
    void shutdown() override;

    int loc_time = -1;
};

class LaserEffectKind final : public EffectKind
{
public:
    void init() override;
    void shutdown() override;

    int loc_time = -1;
    int loc_color1 = -1;
    int loc_color2 = -1;
    int loc_size = -1;
};

// Loads/frees every registered EffectKind (every kind that has been
// constructed as a global somewhere — see EffectKind's constructor). Called
// once by UI::Init/Shutdown; not meant to be called by UI elements. Adding a
// new kind of effect never requires touching these two functions.
void InitEffectKinds();
void ShutdownEffectKinds();

// Effect instance: small, owned by whatever UI element applies the effect
// (e.g. a panel that wants a rainbow outline declares one of these alongside
// its other draw state). Carries only per-use data; the GL resources live in
// the shared EffectKind it points back to via kind().
class Effect
{
public:
    virtual ~Effect() = default;

    virtual const EffectKind& kind() const = 0;
    virtual void apply() const = 0; // sets uniforms; caller wraps in BeginShaderMode/EndShaderMode
};

// Multiplies a layer's rendered output by tint. This exists to prove the
// offscreen-layer -> shader-pass -> composite round trip. Real effects (glow,
// blur) get added the same way: an EffectKind + an Effect with whatever
// per-use data it needs.
class TintEffect final : public Effect
{
public:
    Color tint = WHITE;

    const EffectKind& kind() const override;
    void apply() const override;
};

// Replaces a layer's color with an animated rainbow hue, keeping its alpha
// mask. Time-driven instead of static, but otherwise the same shape as
// TintEffect.
class RainbowEffect final : public Effect
{
public:
    const EffectKind& kind() const override;
    void apply() const override;
};

// Rotating two-color gradient that sweeps around a rect's outline (the
// "Gemini text box" chasing-border look). Keeps the original alpha mask, same
// trick as RainbowEffect — pair with a border-only rect (fill alpha = 0) so
// only the outline reads the sweep instead of the whole fill.
class LaserEffect final : public Effect
{
public:
    Color color1 = WHITE;
    Color color2 = WHITE;
    Vector2 size{}; // rect's pixel size; set once by the owning UI element so the sweep is circular, not skewed

    const EffectKind& kind() const override;
    void apply() const override;
};