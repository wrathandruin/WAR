# WAR — Milestone 22 (bgfx Sprite Expansion / Batch Cleanup)

> Current development milestone: M22 — bgfx Sprite Expansion / Batch Cleanup

## Focus
Expand atlas-backed sprite coverage further into the scene and tighten textured world submission around a cleaner batched flow.

M21 organized the textured path around a shared atlas and material descriptors.
M22 builds on that foundation by moving world tiles onto the atlas-backed sprite path and reducing the number of textured world layers that need to be submitted separately.

## What this milestone does
- expands atlas-backed rendering from actors into world tiles
- introduces tile sprite materials for floor and wall visuals
- combines player and entity sprite submission into a single actor layer
- keeps path nodes and hovered-tile feedback as explicit overlays
- preserves the current camera / projection / world-space rendering flow
- keeps the atlas/material approach established in M21

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
   - keep material selection separate from submission code

4. **Load renderer assets**
   - resolve shader asset paths
   - load the color shader program
   - load the texture shader program
   - load the shared sprite atlas texture

## Why this matters
This is the bridge from “a few textured sprites work” to “the scene is broadly atlas-driven and submission is cleaner”.

It makes the next milestones safer:
- tile variation expansion
- richer sprite coverage
- future authoring improvements
- cleaner scene-wide visual iteration

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
### M23 — bgfx Tile Variant / Authoring Prep
- introduce additional tile/material variants
- prepare the atlas/material layer for broader content authoring
- make scene visuals easier to extend without renderer churn
