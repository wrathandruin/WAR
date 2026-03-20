# WAR — Milestone 28 (bgfx Semantic Prop Hooks / Region-Aware Dressing)

> Current development milestone: M28 — bgfx Semantic Prop Hooks / Region-Aware Dressing

## Focus
Begin tying scene props and visual dressing to semantic region data so authored spaces influence more than just tile theming.

M27 expanded region data into scene semantics.
M28 builds on that by introducing semantic prop population hooks and region-aware prop rendering so Cargo Bay, Transit Spine, Med Lab, Command Deck, and Hazard Containment begin to read as different spaces.

## What this milestone does
- introduces `WorldSemanticDressing` for region-aware test-world prop population
- seeds entities from semantic region intent instead of ad hoc placement
- resolves prop placement into valid open tiles inside the intended region
- adds region-aware entity coloring in both bgfx and GDI renderers
- preserves region boundary overlays and palette controls from M26
- preserves world-driven theme resolution from semantic regions

## Region-aware prop dressing after M28
The current test world now begins to read as:

- Cargo Bay: clustered cargo crates
- Transit Spine: service props along shared routes
- Med Lab: powered medical terminals and clean storage
- Command Deck: command consoles and secure lockers
- Hazard Containment: sealed storage and danger-tinted equipment

## Why this matters
This is the bridge from “regions have names” to “regions affect what is in the scene”.

It makes the next milestones safer:
- region-driven prop expansion
- richer authored spaces
- visual storytelling through layout and equipment
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
### M29 — bgfx Regional Set Dressing / Encounter Hooks
- expand semantic prop dressing density and variety
- prepare authored regions to influence gameplay-facing encounter hooks
- continue moving scene meaning into the world layer
