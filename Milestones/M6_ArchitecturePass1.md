# Milestone 6 — Architecture Pass 1

## Goal
Refactor the prototype backend so future gameplay can scale without turning `GameLayer` into a god object.

## Delivered
- `WorldState`
- `EntityManager`
- `ActionSystem`
- slimmer `GameLayer`
- preserved M5 behavior:
  - LMB move
  - RMB interact
  - Shift + RMB inspect
  - pathfinding and world rendering still work

## Technical Notes
- `WorldState` now owns `WorldGrid` and `EntityManager`
- `EntityManager` owns entity storage and tile lookups
- `ActionSystem` resolves move / inspect / interact actions
- `GameLayer` now focuses on:
  - input collection
  - per-frame coordination
  - rendering

## Why this milestone matters
This is the first structural pass that separates backend responsibilities from the prototype layer. It makes future milestones safer:
- stateful objects
- persistence
- combat systems
- multiplayer

## Limitations
- still not full ECS
- world and rendering are still coupled in places
- action execution still writes directly to the event log

## Next Milestone
Stateful world objects and persistent interaction consequences.
