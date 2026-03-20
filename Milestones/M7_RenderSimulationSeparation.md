# Milestone 7 — Render / Simulation Separation

## Goal
Remove rendering responsibility from `GameLayer` and formalize a rendering layer.

## Delivered
- `WorldRenderer`
- `DebugOverlayRenderer`
- slimmer `GameLayer`
- preserved M6 behavior:
  - LMB move
  - RMB interact
  - Shift + RMB inspect

## Technical Notes
- `WorldRenderer` owns world-facing drawing:
  - tiles
  - hovered tile
  - path
  - entities
  - player
- `DebugOverlayRenderer` owns:
  - debug text
  - hovered details
  - event log
- `GameLayer` now coordinates:
  - input
  - systems
  - render calls

## Why this milestone matters
This separates simulation flow from presentation flow and makes future renderer replacement safer.

## Limitations
- still GDI-backed
- renderers are not yet optimized/cached
- debug overlay is still presentation-only text

## Next Milestone
Stateful world objects and persistent interaction consequences.
