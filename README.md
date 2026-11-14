# EmberGUI

A GPU-accelerated GUI rendering engine written in C++17. Every widget is drawn procedurally by GLSL fragment shaders using **signed distance fields (SDFs)**, with no textures, nine-slice images or pre-rendered assets. The aim is crisp, resolution-independent UI with modern effects (animated borders, glows, soft shadows) that are hard to get from traditional widget toolkits.

> **Status:** early development. The rendering core works; widgets, text and layout are planned.

<!-- TODO: add a screenshot or GIF of the demo, e.g. docs/demo.gif -->

## Features

- **SDF rounded rectangles.** A single shader draws a radius per corner (CSS order: TL, TR, BR, BL), anti-aliased fills and borders, and border-only rects (fill alpha = 0).
- **Soft shadows.** A shadow mode fades smoothly across the SDF boundary.
- **Clipping.** Child elements can be clipped to their parent's bounds in pixel space.
- **Offscreen layers.** Any group of draw calls can be redirected into a render texture and composited back.
- **Post-process effects.** The composite pass runs through an effect shader:
  - `TintEffect`: multiplies the layer by a color
  - `RainbowEffect`: animated hue sweep that keeps the layer's alpha mask
  - `LaserEffect`: two-color gradient that sweeps around a box's outline, corrected for aspect ratio
- **Extensible effect system.** Each effect type is a flyweight (one compiled shader plus uniform locations, shared by all instances) and registers itself automatically. Adding an effect means writing one subclass; no central code changes.
- **Narrow API.** Application code talks only to the `UI::` facade. GPU handles and uniform locations never leak out of the render layer.

## Usage

```cpp
#include "render/UI.h"
#include "render/Effect.h"
#include "render/Layer.h"

InitWindow(800, 600, "EmberGUI");
UI::Init();

UIRect box{};
box.x = 400; box.y = 320; box.w = 200; box.h = 80;
box.radii[0] = box.radii[1] = box.radii[2] = box.radii[3] = 12.0f;
box.border_color = WHITE;
box.border_width = 3.0f;          // border only: fill alpha stays 0

LaserEffect laser;
laser.color1 = {0x00, 0xE0, 0xFF, 0xFF};
laser.color2 = {0xFF, 0x00, 0xC8, 0xFF};
laser.size   = {box.w, box.h};
Layer laser_layer{};              // persists across frames

while (!WindowShouldClose())
{
    UI::BeginFrame({26, 26, 51, 255});

    UI::BeginEffect({box.x, box.y, box.w, box.h}, laser, laser_layer);
    UI::DrawRect(box);
    UI::EndEffect();

    UI::EndFrame();
}

UI::Shutdown();
CloseWindow();
```

See [`src/main.cpp`](src/main.cpp) for the full demo (clipping, rainbow outline, laser border).

## How it works

```
UI::DrawRect ──► RectRenderer ──► rect.fs (SDF fill / border / shadow / clip)
                                     │
UI::BeginEffect ─► Layer (render texture) ◄── draws land here while an effect is active
UI::EndEffect ──► CompositeLayer ──► effect shader (tint / rainbow / laser) ──► framebuffer
```

1. Each `UIRect` is one quad. The fragment shader computes the signed distance to the rounded rect and uses `fwidth` for anti-aliasing. Fill, border and shadow come from that same distance value.
2. Between `BeginEffect` and `EndEffect`, draws go into an offscreen `Layer`. Coordinates stay in absolute screen space because the offset is applied internally.
3. On `EndEffect`, the layer texture is drawn back to the screen with the effect's shader bound. Effects keep the layer's alpha, so they recolor the shape without changing its geometry.

The longer-term design (separate shape, material and layer concepts, plus composite materials) is described in [`docs/RENDERING_ARCHITECTURE.md`](docs/RENDERING_ARCHITECTURE.md). Note that it was written for an earlier Sokol-based backend.

## Project structure

```
src/
  main.cpp               demo application
  render/
    UI.h/.cpp            public facade: frames, rects, effect regions
    UIRect.h             rect style data (radii, colors, border, shadow, clip)
    RectRenderer.h/.cpp  binds rect.fs and uploads per-rect uniforms
    Layer.h/.cpp         offscreen render targets and compositing
    Effect.h/.cpp        effect kinds (flyweights) and effect instances
Shaders/
  rect.fs                SDF rounded-rect shader
  tint.fs  rainbow.fs  laser.fs   post-process effects
docs/
  RENDERING_ARCHITECTURE.md
```

## Building

Requirements: a C++17 compiler, CMake 3.14 or newer, and OpenGL 3.3. [Raylib 5.5](https://github.com/raysan5/raylib) is downloaded automatically through CMake `FetchContent`.

```bash
git clone https://github.com/brunoleppe/EmberGUI.git
cd EmberGUI
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/EmberGUI          # run from the repo root so Shaders/ is found
```

On Linux you may need the X11 and OpenGL development packages first (Debian/Ubuntu):

```bash
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

## Roadmap

- [ ] More SDF shapes (circle, pill, line)
- [ ] Gradient fills and a material system (see architecture doc)
- [ ] Text rendering (SDF/MSDF fonts)
- [ ] Widgets (button, panel, text input) and input handling
- [ ] Layout system
- [ ] Batching/instancing to reduce draw calls
- [ ] Glow and blur effects
- [ ] Nested effect regions

## Tech stack

C++17 · OpenGL 3.3 · GLSL · Raylib · CMake
