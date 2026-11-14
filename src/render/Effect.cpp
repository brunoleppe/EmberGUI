#include "Effect.h"
#include <vector>

namespace
{
    std::vector<EffectKind*>& Registry()
    {
        static std::vector<EffectKind*> registry;
        return registry;
    }

    TintEffectKind g_tint_kind;
    RainbowEffectKind g_rainbow_kind;
    LaserEffectKind g_laser_kind;
}

EffectKind::EffectKind()
{
    Registry().push_back(this);
}

void TintEffectKind::init()
{
    shader = LoadShader(nullptr, "Shaders/tint.fs");
    loc_tint = GetShaderLocation(shader, "u_tint");
}

void TintEffectKind::shutdown()
{
    UnloadShader(shader);
}

void RainbowEffectKind::init()
{
    shader = LoadShader(nullptr, "Shaders/rainbow.fs");
    loc_time = GetShaderLocation(shader, "u_time");
}

void RainbowEffectKind::shutdown()
{
    UnloadShader(shader);
}

void LaserEffectKind::init()
{
    shader = LoadShader(nullptr, "Shaders/laser.fs");
    loc_time = GetShaderLocation(shader, "u_time");
    loc_color1 = GetShaderLocation(shader, "u_color1");
    loc_color2 = GetShaderLocation(shader, "u_color2");
    loc_size = GetShaderLocation(shader, "u_size");
}

void LaserEffectKind::shutdown()
{
    UnloadShader(shader);
}

void InitEffectKinds()
{
    for (EffectKind* kind : Registry()) kind->init();
}

void ShutdownEffectKinds()
{
    auto& registry = Registry();
    for (auto it = registry.rbegin(); it != registry.rend(); ++it) (*it)->shutdown();
}

const EffectKind& TintEffect::kind() const
{
    return g_tint_kind;
}

void TintEffect::apply() const
{
    Vector4 c = ColorNormalize(tint);
    SetShaderValue(g_tint_kind.shader, g_tint_kind.loc_tint, &c, SHADER_UNIFORM_VEC4);
}

const EffectKind& RainbowEffect::kind() const
{
    return g_rainbow_kind;
}

void RainbowEffect::apply() const
{
    float t = (float)GetTime();
    SetShaderValue(g_rainbow_kind.shader, g_rainbow_kind.loc_time, &t, SHADER_UNIFORM_FLOAT);
}

const EffectKind& LaserEffect::kind() const
{
    return g_laser_kind;
}

void LaserEffect::apply() const
{
    float t = (float)GetTime();
    Vector4 c1 = ColorNormalize(color1);
    Vector4 c2 = ColorNormalize(color2);
    float size_arr[2] = {size.x, size.y};
    SetShaderValue(g_laser_kind.shader, g_laser_kind.loc_time, &t, SHADER_UNIFORM_FLOAT);
    SetShaderValue(g_laser_kind.shader, g_laser_kind.loc_color1, &c1, SHADER_UNIFORM_VEC4);
    SetShaderValue(g_laser_kind.shader, g_laser_kind.loc_color2, &c2, SHADER_UNIFORM_VEC4);
    SetShaderValue(g_laser_kind.shader, g_laser_kind.loc_size, size_arr, SHADER_UNIFORM_VEC2);
}