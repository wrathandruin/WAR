# WAR — Milestone 27 (bgfx Region Data Expansion / Scene Semantics)

> Current development milestone: M27 — bgfx Region Data Expansion / Scene Semantics

## Focus
Move beyond the simple left/middle/right region split and make region data describe more meaningful authored spaces.

M26 made region overlays visible and palette tuning practical.
M27 builds on that by expanding world-region tagging into a richer semantic layout so the renderer is no longer just showing broad bands of theme, but distinct scene-oriented zones.

## What this milestone does
- expands `WorldRegionTag` from broad zone buckets into more semantic region types
- replaces the simple three-band test layout with a richer authored region layout
- keeps theme resolution driven from world region tags
- preserves visible region boundary overlays and palette controls from M26
- keeps the current camera / projection / world-space submission flow intact

## Region semantics after M27
The test world now uses region tags that describe purpose, not just palette:

- Cargo Bay
- Transit Spine
- Med Lab
- Command Deck
- Hazard Containment

These semantic regions still resolve into the existing visual themes, but the map layout is now shaped more like authored spaces than like a basic color split.

## Why this matters
This is the bridge from “regions exist” to “regions mean something”.

It makes the next milestones safer:
- authored scene logic tied to visual identity
- better region-driven layout experimentation
- easier future integration of props, encounters, and mission semantics
- continued reduction of renderer-local assumptions

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

And the shared sprite atlas at:

```text
assets/textures/world_atlas.bmp
```

## Next Milestone
### M28 — bgfx Semantic Prop Hooks / Region-Aware Dressing
- begin tying scene props and visual dressing to semantic region data
- prepare authored spaces to influence more than just tile theming
- continue pushing content meaning into the world layer instead of the renderer
