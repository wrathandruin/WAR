# Milestone 10 — bgfx Integration Path

## Goal
Wire the project for a real bgfx backend while keeping the codebase buildable and runnable before the dependency is fully installed.

## Delivered
- real bgfx initialization path behind compile guards
- GameLayer backend selection prefers bgfx first
- safe fallback to GDI when bgfx is unavailable
- project file updated with expected bgfx include/lib locations

## Technical Notes
- `BgfxRenderDevice` now contains Win32/bgfx initialization logic
- compile guards prevent build failures when bgfx is absent
- `GameLayer` attempts bgfx, then falls back to GDI
- this milestone intentionally preserves runtime safety over forcing a broken dependency state

## Why this milestone matters
This creates the practical handoff point between the prototype renderer and the future production backend:
- gameplay stays untouched
- rendering orchestration stays stable
- backend activation becomes dependency-driven instead of refactor-driven

## Limitations
- bgfx dependency is not bundled by this package
- renderer classes still draw through GDI/HDC, so full visual migration is not complete yet
- a real bgfx world renderer is still the next rendering step after dependency activation

## Next Milestone
Platform abstraction layer.
