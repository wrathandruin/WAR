# WAR — Milestone 17 (bgfx Batching & Render Data)

> Current development milestone: M17 — bgfx batching and render data cleanup

## Focus
Make the bgfx renderer less ad hoc by separating render data generation from GPU submission.

## What this milestone does
- introduces `BgfxRenderDataBuilder`
- groups world render data into explicit layers
- keeps batching by layer instead of mixing generation and submission
- preserves the current gameplay path and bgfx world rendering behavior

## Renderer architecture
The bgfx world path is now split into two steps:

1. **Build render data**
   - tiles
   - path nodes
   - hovered tile
   - entities
   - player

2. **Submit render data**
   - one submission path per layer
   - cleaner CPU-side renderer flow

## Why this matters
This makes the next renderer milestones safer:
- camera/projection cleanup
- texture pipeline
- material/shader expansion
- better batching and draw ordering

## Requirements
The bgfx geometry path still expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
```

## Next Milestone
### M18 — bgfx Camera / Projection Cleanup
- move away from CPU-heavy screen-space assumptions
- prepare the renderer for a cleaner world-space pipeline
