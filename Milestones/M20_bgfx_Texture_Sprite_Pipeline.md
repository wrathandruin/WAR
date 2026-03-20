# M20 — bgfx Texture / Sprite Pipeline

> Current development milestone: M20 — bgfx Texture / Sprite Pipeline

## Focus
Move the bgfx renderer beyond placeholder flat-color geometry and establish the first proper textured rendering path.

M19 cleaned up shader ownership, asset-path handling, and build clarity.
M20 builds on that foundation by introducing textured quads, sprite-ready shader flow, and explicit texture asset loading.

## What this milestone does
- introduces a textured bgfx shader/program path alongside the existing solid-color path
- adds textured quad submission for bgfx
- introduces explicit texture asset loading for renderer-owned assets
- separates solid-color rendering from textured sprite rendering
- establishes the first sprite-capable material path for world visuals
- keeps the current camera / projection / world-space pipeline intact

## Renderer architecture after M20
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
   - load solid-color shader program
   - load textured shader program
   - load texture assets required by the textured path

4. **Submit render layers**
   - submit solid-color layers through the color program
   - submit textured layers through the textured program
   - preserve current world ordering and visibility behavior

## Why this matters
Until now, the bgfx renderer has proven the world-space camera path with flat-color placeholder visuals.
That was the right bridge milestone, but it is not a viable long-term visual pipeline.

M20 matters because it creates the first real content path for:
- sprites
- textured props
- texture-backed tiles
- future atlas work
- cleaner separation between debug geometry and art-driven rendering

## Runtime shader layout
```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

## Texture asset layout
```text
assets/textures/player.bmp
assets/textures/crate.bmp
assets/textures/terminal.bmp
assets/textures/locker.bmp
```

## Files touched in this milestone
- `assets/shaders/src/vs_texture.sc`
- `assets/shaders/src/fs_texture.sc`
- `assets/shaders/src/varying_texture.def.sc`
- `assets/textures/player.bmp`
- `assets/textures/crate.bmp`
- `assets/textures/terminal.bmp`
- `assets/textures/locker.bmp`
- `scripts/build_shaders_win64.bat`
- `src/engine/render/RenderAssetPaths.h`
- `src/engine/render/RenderAssetPaths.cpp`
- `src/engine/render/BgfxShaderProgram.h`
- `src/engine/render/BgfxShaderProgram.cpp`
- `src/engine/render/BgfxTextureAsset.h`
- `src/engine/render/BgfxTextureAsset.cpp`
- `src/engine/render/BgfxRenderData.h`
- `src/engine/render/BgfxRenderData.cpp`
- `src/engine/render/BgfxWorldRenderer.h`
- `src/engine/render/BgfxWorldRenderer.cpp`
- `src/game/GameLayer.cpp`
- `CMakeLists.txt`
- `WAR.vcxproj`
- `WAR.vcxproj.filters`

## Next Milestone
### M21 — bgfx Atlas / Material Organization
- introduce atlas support
- reduce per-draw texture switching
- define sprite/material descriptors more cleanly
- prepare the renderer for larger-scale art integration
