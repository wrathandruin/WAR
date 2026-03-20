# WAR — Milestone 24 (bgfx Theme Sets / Authoring Hooks)

> Current development milestone: M24 — bgfx Theme Sets / Authoring Hooks

## Focus
Group tile visuals into explicit scene themes and add lightweight runtime authoring hooks for switching them without rewriting renderer code.

M23 introduced tile variants and centralized tile visual selection.
M24 builds on that by introducing theme-oriented material sets and a world-state theme selector so scene styling can change through authored intent instead of renderer churn.

## What this milestone does
- introduces `BgfxWorldTheme` as the theme/material grouping layer
- stores the active world visual theme inside `WorldState`
- updates `BgfxTileVisuals` to resolve tile materials and tints through the active theme
- adds lightweight runtime theme switching hooks in `GameLayer`
- keeps actor materials on the shared atlas path
- preserves the current camera / projection / world-space submission flow

## Renderer architecture
The bgfx world path is now split into:

1. **Build world-space render data**
   - themed tile sprite layer
   - path overlay
   - hovered tile overlay
   - actor sprite layer

2. **Build view/projection**
   - use the active camera
   - compute visible world extents
   - submit world-space geometry through bgfx

3. **Resolve materials**
   - map sprite material ids to atlas UV regions
   - resolve tile materials through `BgfxTileVisuals`
   - group tile materials through `BgfxWorldTheme`

4. **Load renderer assets**
   - resolve shader asset paths
   - load the color shader program
   - load the texture shader program
   - load the shared sprite atlas texture

## Why this matters
This is the bridge from “tile visuals can vary” to “scene visuals can be steered by explicit themes”.

It makes the next milestones safer:
- theme-based content styling
- richer authoring control
- broader atlas expansion
- cleaner visual iteration

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
### M25 — bgfx Content Tagging / Region Hooks
- introduce map-region visual tagging hooks
- prepare theme selection to be driven by authored world zones
- continue reducing hardcoded visual assumptions
