#include "raylib.h"
#include "render/UI.h"
#include "render/Effect.h"
#include "render/Layer.h"

int main()
{
    InitWindow(800, 600, "EmberGUI");
    SetTargetFPS(60);

    UI::Init();

    UIRect panel = {};
    panel.x = 100;
    panel.y = 100;
    panel.w = 200;
    panel.h = 150;
    panel.radii[0] = panel.radii[1] = panel.radii[2] = panel.radii[3] = 8.0f;
    panel.color = Color{0x22, 0x22, 0x33, 0xFF};
    panel.border_color = Color{0x88, 0x88, 0xAA, 0xFF};
    panel.border_width = 1.5f;

    // Child rect — larger than the panel, should be clipped to panel bounds
    UIRect child = {};
    child.x = 80;
    child.y = 120; // starts OUTSIDE the panel on the left
    child.w = 280;
    child.h = 60; // wider than the panel too
    child.radii[0] = child.radii[1] = child.radii[2] = child.radii[3] = 8.0f;
    child.color = Color{0xFF, 0x66, 0x33, 0xFF};
    child.border_color = Color{0xFF, 0x99, 0x66, 0xFF};
    child.border_width = 1.5f;
    // clip = panel's interior (inset by border)
    child.clip[0] = 101;
    child.clip[1] = 101;
    child.clip[2] = 198;
    child.clip[3] = 148;

    // Outline rect — rendered through a post-process effect layer. The
    // rainbow effect replaces the border's white with an animated hue
    // sweep, keeping the border's shape/alpha untouched.
    UIRect outline = {};
    outline.x = 400;
    outline.y = 100;
    outline.w = 200;
    outline.h = 150;
    outline.radii[0] = outline.radii[1] = outline.radii[2] = outline.radii[3] = 8.0f;
    outline.color = Color{0xff, 0xff, 0xff, 0x56};
    outline.border_color = Color{0xff, 0xff, 0xff, 0xff};
    outline.border_width = 5.0f;

    RainbowEffect outline_effect;
    Layer outline_layer{};

    // Laser box — border-only rect (fill alpha = 0) so the chasing gradient
    // reads as an outline instead of tinting a filled rect.
    UIRect laser_box = {};
    laser_box.x = 400;
    laser_box.y = 320;
    laser_box.w = 200;
    laser_box.h = 80;
    laser_box.radii[0] = laser_box.radii[1] = laser_box.radii[2] = laser_box.radii[3] = 12.0f;
    laser_box.border_color = WHITE;
    laser_box.border_width = 3.0f;

    LaserEffect laser_effect;
    laser_effect.color1 = Color{0x00, 0xE0, 0xFF, 0xFF};
    laser_effect.color2 = Color{0xFF, 0x00, 0xC8, 0xFF};
    laser_effect.size = {laser_box.w, laser_box.h};
    Layer laser_layer{};

    while (!WindowShouldClose())
    {
        UI::BeginFrame(Color{26, 26, 51, 255}); // matches old 0.1,0.1,0.2 clear color

        UI::DrawRect(panel);
        UI::DrawRect(child);

        UI::BeginEffect(Rectangle{outline.x, outline.y, outline.w, outline.h}, outline_effect, outline_layer);
        UI::DrawRect(outline);
        UI::EndEffect();

        UI::BeginEffect(Rectangle{laser_box.x, laser_box.y, laser_box.w, laser_box.h}, laser_effect, laser_layer);
        UI::DrawRect(laser_box);
        UI::EndEffect();

        UI::EndFrame();
    }

    UI::Shutdown();
    if (outline_layer.width > 0) DestroyLayer(outline_layer);
    if (laser_layer.width > 0) DestroyLayer(laser_layer);
    CloseWindow();
    return 0;
}