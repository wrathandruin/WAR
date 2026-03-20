# Milestone 9 — Render Backend Abstraction

## Goal
Abstract the render backend so the project can migrate from GDI to bgfx without disturbing gameplay systems.

## Delivered
- `IRenderDevice`
- `GdiRenderDevice`
- `BgfxRenderDevice` stub
- `GameLayer` now renders through a backend device

## Technical Notes
- `GdiRenderDevice` owns per-frame GDI buffer setup and presentation
- `BgfxRenderDevice` is intentionally a compile-safe stub
- `WorldRenderer` and `DebugOverlayRenderer` remain unchanged in this milestone
- this milestone isolates backend lifecycle before a real bgfx dependency lands

## Why this milestone matters
It creates the seam needed for backend migration:
- Win32 stays for platform
- render backend becomes swappable
- gameplay code remains untouched

## Limitations
- bgfx is not wired yet
- GDI remains the active runtime backend
- renderer code still consumes `HDC`

## Next Milestone
bgfx backend integration.
