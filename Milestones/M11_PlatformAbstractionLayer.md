# Milestone 11 — Platform Abstraction Layer

## Goal
Reduce direct Win32 dependency in gameplay orchestration by introducing a window/platform interface.

## Delivered
- `IWindow`
- `Win32Window : IWindow`
- `GameLayer` now depends on `IWindow`

## Technical Notes
- input and size queries are now requested through the interface
- platform implementation remains Win32
- this is a structural milestone, not a feature milestone

## Why this milestone matters
It isolates platform concerns before future cross-platform work begins.

## Limitations
- Win32 is still the only actual platform backend
- lower-level rendering and startup flow still reference Win32 where required

## Next Milestone
First real bgfx frame path.
