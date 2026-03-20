# Milestone 18 — bgfx Camera / Projection Cleanup

## Goal
Stop treating the bgfx world renderer as a CPU-side screen-space system and move it to a cleaner world-space projection path.

## Delivered
- `BgfxViewTransform`
- orthographic bgfx projection derived from camera state
- world-space render data for tiles, path, hovered tile, entities, and player
- updated `BgfxWorldRenderer` submission path

## Technical Notes
- visible world extents are derived from `Camera2D::screenToWorld`
- the renderer now builds geometry in world units
- bgfx view/projection setup now owns the camera transform
- this preserves current rendering output while removing a large chunk of screen-space conversion logic

## Why this milestone matters
This is the first real camera/projection milestone for the bgfx renderer:
- cleaner renderer responsibility split
- less CPU-side projection logic
- safer base for textured rendering and richer shaders

## Limitations
- still a simple colored-quad renderer
- no textures/materials yet
- no view-layer separation yet
- batching is still layer-based, not material-based

## Next Milestone
bgfx shader / asset pipeline cleanup.
