# WAR — Milestone 9 (Render Backend Abstraction)

> Current development milestone: M9 — Render Backend Abstraction

Milestone 9 for **Wrath and Ruin (WAR)**.

## Focus of this milestone
This milestone introduces a render backend abstraction so the project can move from GDI toward bgfx without touching gameplay systems.

- `GameLayer` no longer owns frame buffer setup
- rendering now goes through an `IRenderDevice`
- current backend is `GdiRenderDevice`
- a compile-safe `BgfxRenderDevice` stub is included as the next integration target

## Why this milestone exists
A true bgfx integration requires adding bgfx itself to the repo and build configuration. This milestone prepares the architecture safely so that swap can happen cleanly.

## Current Backends

### GdiRenderDevice
- active backend
- preserves current rendering behavior

### BgfxRenderDevice
- compile-safe stub
- placeholder for real bgfx hookup in the next milestone

## Controls
- **Left click**: Move
- **Right click**: Interact
- **Shift + Right click**: Inspect
- **Middle mouse drag**: Pan camera
- **Mouse wheel**: Zoom

## What changed from M8
- introduced render backend interface
- moved frame setup / presentation out of `GameLayer`
- added active GDI backend implementation
- added bgfx-ready backend stub

## Current Status
WAR now has:
- separated gameplay systems
- separated renderers
- separated render backend lifecycle

## Next Milestone

### M10 — bgfx Integration
- wire real bgfx init into Win32
- replace GDI frame device with bgfx device
- keep gameplay and renderer orchestration unchanged
