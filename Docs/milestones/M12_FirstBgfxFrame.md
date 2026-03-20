# Milestone 12 — First bgfx Frame

## Goal
Make bgfx produce a real frame when active, rather than only validating initialization.

## Delivered
- `BgfxRenderDevice` now supports an active frame path
- `BgfxDebugFrameRenderer` draws bgfx debug text
- active backend can now be visibly confirmed on-screen

## Technical Notes
- full world rendering is still GDI-only in this milestone
- bgfx frame path uses debug text and clear operations
- this is the final bridge before rewriting the world renderer for bgfx

## Why this milestone matters
It proves:
- backend selection works
- bgfx can actually own the frame
- the project can render through a non-GDI backend

## Limitations
- tile/entity/player world rendering is not yet moved to bgfx
- debug overlay is minimal in bgfx mode

## Next Milestone
First real bgfx world renderer.
