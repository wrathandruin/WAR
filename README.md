# WAR — Milestone 18 (bgfx Camera / Projection Cleanup)

> Current development milestone: M18 — bgfx camera / projection cleanup

## Focus
Move the bgfx renderer away from CPU-built screen-space/NDC assumptions and toward a cleaner world-space pipeline.

## What this milestone does
- introduces `BgfxViewTransform`
- builds an orthographic bgfx projection from the camera
- changes bgfx render data from screen rectangles to world-space quads
- keeps the current visible world rendering behavior

## Renderer architecture
The bgfx world path is now split into:

1. **Build world-space render data**
   - tiles
   - path nodes
   - hovered tile
   - entities
   - player

2. **Build view/projection**
   - use the active camera
   - compute visible world extents
   - submit world-space geometry through bgfx

## Why this matters
This is the bridge from “CPU pushes screen quads” to “renderer owns the real camera path”.

It makes the next milestones safer:
- texture and sprite pipeline
- cleaner zoom/pan behavior
- batching improvements
- material/shader expansion

## Requirements
The bgfx geometry path still expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
```

## Next Milestone
### M19 — bgfx Shader / Asset Pipeline Cleanup
- improve shader/tooling workflow
- make shader asset handling more explicit and less brittle
