# WAR Milestone 2.1

Milestone 2.1 for **Wrath and Ruin (WAR)**.

## Focus of this patch
This package is a stabilization pass on Milestone 2:

- safer `WorldGrid` construction and indexing
- safer test-map generation
- clearer blocked-tile visuals
- hover blocked state in overlay
- same real-time pathfinding shell as Milestone 2

## Scope
- Win32 window
- Real-time main loop
- Backbuffered GDI rendering
- `WorldGrid` tile model
- Blocked / walkable tiles
- A* pathfinding
- Real-time path following
- Debug/event log

## Controls
- **Left click**: issue move order to clicked tile
- **Middle mouse drag**: pan camera
- **Mouse wheel**: zoom

## What changed from M2
- blocked tiles are much more visible
- `WorldGrid::index` now uses `size_t`
- grid generation uses bounds-safe helpers
- overlay shows whether hovered tile is blocked
