# M19 — bgfx Shader / Asset Pipeline Cleanup

> Current development milestone: M19 — bgfx Shader / Asset Pipeline Cleanup

## Focus
Make the bgfx shader and asset pipeline explicit, portable, and maintainable.

M18 cleaned up the camera/projection path and moved the renderer toward proper world-space submission.
M19 finishes the next critical piece: shader ownership, asset-path handling, and build-system clarity.

## What this milestone does
- introduces a dedicated bgfx shader program loader
- centralizes renderer asset-path construction
- removes direct shader file IO from `BgfxWorldRenderer`
- keeps shader/program lifetime management out of the world renderer
- updates build files so Visual Studio and CMake describe the same modern source layout
- adds an explicit shader build/staging script for the Direct3D 11 path

## Renderer architecture after M19
The bgfx world path is now split into:

1. **Build world-space render data**
   - tiles
   - path nodes
   - hovered tile
   - entities
   - player

2. **Build view/projection**
   - use the active camera
   - compute visible world extents
   - submit world-space geometry through bgfx

3. **Load renderer assets**
   - resolve renderer backend shader folder
   - construct shader binary paths
   - load shader binaries through a dedicated shader program layer
   - create and own bgfx shader/program handles outside the world renderer

## Why this matters
This milestone removes brittle renderer-local assumptions from the bgfx pipeline.

Before M19:
- shader binary paths were built directly inside `BgfxWorldRenderer`
- shader file loading and program creation lived inside the renderer
- Visual Studio contained the real shader build flow, while CMake lagged behind the actual codebase
- shader/asset failures were functional, but not yet cleanly isolated as pipeline concerns

After M19:
- shader loading is a first-class renderer service
- runtime asset expectations are explicit
- renderer code is cleaner and easier to extend
- build-system behavior is easier to reason about
- the project is better prepared for texture and sprite work

## Shader asset layout
The runtime shader layout remains:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
```

Shader source files remain the canonical authored sources:

```text
assets/shaders/src/vs_color.sc
assets/shaders/src/fs_color.sc
assets/shaders/src/varying.def.sc
```

## Build / tooling changes
M19 makes shader compilation and staging more explicit.

### Added / clarified
- dedicated shader asset path helpers
- dedicated bgfx shader program loader
- explicit shader build script for the D3D11 path
- synchronized Visual Studio and CMake source layout

### Result
The renderer no longer hides shader pipeline responsibility inside world-rendering code.

## Current behavior
- bgfx still renders the current world-space geometry path
- shader failures now surface as pipeline failures, not renderer confusion
- asset-path behavior is clearer and easier to debug
- shutdown behavior is cleaner because shader program lifetime is explicit

## Files touched in this milestone
- `src/engine/render/RenderAssetPaths.h`
- `src/engine/render/RenderAssetPaths.cpp`
- `src/engine/render/BgfxShaderProgram.h`
- `src/engine/render/BgfxShaderProgram.cpp`
- `src/engine/render/BgfxWorldRenderer.h`
- `src/engine/render/BgfxWorldRenderer.cpp`
- `src/game/GameLayer.cpp`
- `CMakeLists.txt`
- `WAR.vcxproj`
- `WAR.vcxproj.filters`
- `scripts/build_shaders_win64.bat`

## Controls
- Left click: Move
- Right click: Interact
- Shift + Right click: Inspect
- Middle mouse drag: Pan camera
- Mouse wheel: Zoom

## Next Milestone
### M20 — bgfx Texture / Sprite Pipeline
- introduce textured quad support
- separate solid-color and textured material paths
- begin moving world visuals away from placeholder flat-color geometry
- prepare sprite asset loading and atlas work

## Notes
M19 is intentionally a cleanup milestone.
It does not try to add sprite rendering, materials, animation, or batching expansion beyond what is necessary to stabilize the shader and asset pipeline.
