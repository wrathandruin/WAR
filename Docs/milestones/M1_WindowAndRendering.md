# Milestone 1 — Window & Rendering

## Goal
Establish a visible, interactive application shell for WAR.

## Delivered
- Win32 window bootstrap
- real-time application loop
- GDI rendering path with backbuffered presentation
- 2D camera with pan and zoom
- direct click-to-move prototype
- debug overlay

## Technical Notes
- Rendering is intentionally temporary and uses GDI to validate the loop quickly.
- Movement in this milestone is direct and not world-aware.
- This milestone proves the application shell, rendering flow, and basic input path.

## Limitations
- no world model
- no pathfinding
- no action system
- no interaction verbs

## Next Milestone
World model, blocked tiles, traversal, and pathfinding.
