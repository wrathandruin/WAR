# WAR — Milestone 21 (bgfx Atlas / Material Organization)

> Current development milestone: M21 — bgfx Atlas / Material Organization

## Focus
Organize the textured bgfx path around a shared sprite atlas and explicit material descriptors.

M20 introduced the first textured sprite-capable rendering path.
M21 refactors that path so sprite content is no longer organized as one texture per sprite.
Instead, the renderer now uses a shared atlas texture plus clean material-to-UV mapping.

## What this milestone does
- introduces `BgfxSpriteMaterial` as the atlas/material descriptor layer
- moves player and entity sprites onto a shared `world_atlas.bmp`
- removes per-sprite texture selection from world render data
- reduces texture switching by submitting textured world sprites through one atlas-backed path
- keeps the current camera / projection / world-space rendering flow intact
- keeps tiles, path nodes, and hovered tile rendering on the existing color path

## Renderer architecture
The bgfx world path is now split into:

1. **Build world-space render data**
   - tiles
   - path nodes
   - hovered tile
   - entity sprite instances
   - player sprite instance

2. **Build view/projection**
   - use the active camera
   - compute visible world extents
   - submit world-space geometry through bgfx

3. **Resolve materials**
   - map sprite material ids to atlas UV regions
   - keep sprite material selection out of the renderer submission logic

4. **Load renderer assets**
   - resolve shader asset paths
   - load the color shader program
   - load the texture shader program
   - load the shared sprite atlas texture

## Why this matters
This is the bridge from “textured sprites work” to “textured sprites are organized and scalable”.

It makes the next milestones safer:
- broader sprite coverage
- cleaner sprite variation
- tighter batching and layer submission
- easier art-driven expansion

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
### M22 — bgfx Sprite Expansion / Batch Cleanup
- expand atlas-backed sprite coverage further into the scene
- clean up draw ordering and batch submission strategy
- prepare the renderer for broader world visual replacement
