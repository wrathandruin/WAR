# WAR — Milestones 14 + 15 + 16 (First bgfx Geometry Pass)

> Current development milestone: M16 — bgfx Shader Pipeline & Geometry Pass  
> This package combines **M14**, **M15**, and **M16** so the bgfx path makes a meaningful jump.

## Focus of these milestones

### M14 — First bgfx Tile Renderer
- render the tile grid through bgfx
- use colored quad submission instead of GDI rectangles
- keep the GDI path as a safe fallback

### M15 — bgfx Entities, Player, and Path
- render entities through bgfx
- render a player placeholder through bgfx
- render path nodes and hovered-tile highlight through bgfx

### M16 — Shader Pipeline & Runtime Fallback
- add shader source files to the repo
- load compiled shader binaries at runtime
- show bgfx debug text when shaders are missing instead of leaving you blind

## Important note
This package does **not** bundle precompiled shader binaries.

To see the full bgfx world rendering, you must compile the included shader source into:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
```

Until those binaries exist:
- bgfx can still be the active backend
- you will see bgfx debug text
- the world geometry path will report that shaders are missing

## What this package changes
- introduces `BgfxWorldRenderer`
- upgrades `BgfxDebugFrameRenderer`
- updates `GameLayer` to route bgfx rendering through real world renderers
- updates the Visual Studio project files
- adds shader source files to the repo

## Controls
- **Left click**: Move
- **Right click**: Interact
- **Shift + Right click**: Inspect
- **Middle mouse drag**: Pan camera
- **Mouse wheel**: Zoom

## Next Milestone

### M17 — bgfx Batching & Render Data
- reduce draw-call count
- move render data generation out of renderer code
- prepare the renderer for texture/material work
