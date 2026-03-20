# WAR — Milestone 23 (bgfx Tile Variant / Authoring Prep)

> Current development milestone: M23 — bgfx Tile Variant / Authoring Prep

## Focus
Introduce tile material variants and centralize tile visual selection so the atlas/material path is easier to extend without renderer churn.

M22 expanded atlas-backed rendering into the world tile layer.
M23 builds on that by formalizing tile material selection behind a dedicated resolver and by expanding the atlas to support multiple floor and wall variants.

## What this milestone does
- introduces `BgfxTileVisuals` as the tile material selection layer
- expands the shared world atlas with multiple floor and wall regions
- moves tile visual choice out of render-data construction and into a dedicated authoring-prep helper
- keeps actor materials on the shared atlas path
- preserves the current camera / projection / world-space submission flow
- keeps path nodes and hovered tile feedback as explicit overlays

## Renderer architecture
The bgfx world path is now split into:

1. **Build world-space render data**
   - tile sprite layer
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
   - keep material selection separate from submission code

4. **Load renderer assets**
   - resolve shader asset paths
   - load the color shader program
   - load the texture shader program
   - load the shared sprite atlas texture

## Why this matters
This is the bridge from “the scene uses an atlas” to “the scene can grow visually without rewriting render code”.

It makes the next milestones safer:
- richer tile variation
- broader atlas authoring
- cleaner content expansion
- less renderer churn when visuals change

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
### M24 — bgfx Theme Sets / Authoring Hooks
- introduce theme-oriented material grouping
- prepare scene visuals for broader content styling
- make atlas/material expansion easier to direct from authored data
