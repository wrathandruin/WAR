# WAR — Milestone 26 Correction (Region Authoring Overlay Visibility Fix)

> Current development milestone: M26 — bgfx Region Authoring Overlay / Palette Hooks

## Focus
Make the M26 region authoring overlay unmistakably visible.

The original M26 pass introduced overlay and palette hooks, but the overlay itself was too subtle because it was implemented as a light translucent wash over already themed tiles.
This correction replaces that weak wash with explicit region boundary strips so the authoring overlay is obvious when toggled.

## What this correction does
- replaces the weak full-tile overlay wash with strong region boundary lines
- keeps palette modes active for region-aware coloring
- keeps world-driven Industrial / Sterile / Emergency regions intact
- keeps the current bgfx world-space rendering architecture unchanged
- improves the event messaging so the overlay behavior is clearer

## Intended result
Pressing **O** should now visibly toggle boundary markers where neighboring region tags differ.
On the current test map, that means the two vertical seams between the three world regions should become clearly visible.

## Package contents
- `src/engine/render/BgfxWorldTheme.h`
- `src/engine/render/BgfxWorldTheme.cpp`
- `src/engine/render/BgfxRenderData.cpp`
- `src/game/GameLayer.cpp`
- `Milestones/M26_bgfx_Region_Authoring_Overlay_Palette_Hooks.md`
