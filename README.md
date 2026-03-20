# WAR — Milestone 20 (bgfx Texture / Sprite Pipeline)

> Current development milestone: M20 — bgfx Texture / Sprite Pipeline

## Focus
Move the bgfx renderer beyond placeholder flat-color geometry and establish the first proper textured rendering path.

## What this milestone does
- introduces a textured bgfx shader/program path alongside the existing solid-color path
- adds explicit texture asset loading for renderer-owned assets
- uses textured world-space quads for entities and the player
- keeps tiles, path markers, and hovered-tile feedback on the existing solid-color path
- updates build files so both shader binaries and texture assets are staged into the runtime output

## Renderer architecture
The bgfx world path is now split into:

1. **Build world-space render data**
   - tiles
   - path nodes
   - hovered tile
   - entity sprites
   - player sprite

2. **Build view/projection**
   - use the active camera
   - compute visible world extents
   - submit world-space geometry through bgfx

3. **Load renderer assets**
   - resolve shader asset paths
   - load the color shader program
   - load the texture shader program
   - load runtime texture assets

## Why this matters
This is the bridge from “flat-color placeholder geometry” to “real textured sprite-capable rendering”.

It makes the next milestones safer:
- atlas / material organization
- cleaner sprite expansion
- batching improvements
- richer visual authoring

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

And texture assets at:

```text
assets/textures/player.bmp
assets/textures/crate.bmp
assets/textures/terminal.bmp
assets/textures/locker.bmp
```

## Next Milestone
### M21 — bgfx Atlas / Material Organization
- introduce atlas support
- reduce per-draw texture switching
- define sprite/material descriptors more cleanly
