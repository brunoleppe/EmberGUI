#pragma once
#include "raylib.h"

// Style data for one rounded rect (fill, border, shadow, clip).
struct UIRect
{
    float x, y, w, h;
    float radii[4] = {0, 0, 0, 0}; // TL TR BR BL
    Color color = {0, 0, 0, 0};
    Color border_color = {0, 0, 0, 0};
    float border_width = 0.0f;
    float shadow_softness = 0.0f;
    float clip[4] = {0, 0, 0, 0}; // global pixel space x,y,w,h; clip[2]<=0 disables
};