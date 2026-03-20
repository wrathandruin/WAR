# Milestone 8 — Stateful World Objects

## Goal
Make world objects remember interaction outcomes and expose those outcomes through visuals and inspection.

## Delivered
- stateful entities
- crate open state
- terminal power state
- locker lock/open behavior
- inspect reflects state
- renderer reflects state visually

## Technical Notes
- entity state is stored directly on `Entity`
- `ActionSystem` mutates entity state during interaction
- `WorldRenderer` uses entity state to choose visuals
- `DebugOverlayRenderer` can show hovered entity state

## Why this milestone matters
This is the first time the world reacts persistently to interaction within a session. It unlocks:
- puzzles
- loot containers
- doors/locks
- scripted world state
- future persistence

## Limitations
- state is runtime-only
- no save/load yet
- no inventory or loot transfer yet
- no UI panels for object details

## Next Milestone
bgfx rendering backend migration.
