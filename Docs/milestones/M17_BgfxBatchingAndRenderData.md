# Milestone 17 — bgfx Batching & Render Data

## Goal
Separate bgfx world render data generation from GPU submission so the renderer can scale without turning into a monolithic function.

## Delivered
- `BgfxRenderDataBuilder`
- layered world render data
- cleaner `BgfxWorldRenderer`
- retained bgfx tile/entity/player/path rendering

## Technical Notes
- CPU-side render data is built into explicit layers:
  - tiles
  - path
  - hovered tile
  - entities
  - player
- `BgfxWorldRenderer` now focuses on:
  - shader/program readiness
  - transient buffer submission
  - layer ordering
- this milestone preserves the current visual output while improving renderer structure

## Why this milestone matters
This is the renderer equivalent of the earlier gameplay architecture passes:
- data generation is now isolated
- submission code is easier to evolve
- the next projection and texture steps are much safer

## Limitations
- still uses CPU-side screen-space conversion
- no textures yet
- no material system yet
- batching is layer-based, not material-based

## Next Milestone
bgfx camera / projection cleanup.
