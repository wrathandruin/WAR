# Milestone 14 — First bgfx Tile Renderer

## Goal
Render the world tile grid through bgfx instead of the legacy GDI path.

## Delivered
- `BgfxWorldRenderer`
- colored quad submission for tiles
- camera-based tile-to-screen transform in bgfx path

## Technical Notes
- tiles are submitted as simple colored quads
- coordinates are converted to normalized device coordinates on the CPU
- this avoids introducing camera matrices too early in the first geometry pass

## Why this milestone matters
It is the first true replacement of GDI world drawing with GPU-submitted geometry.
