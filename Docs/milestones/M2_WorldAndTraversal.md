# Milestone 2 — World & Traversal

## Goal
Introduce a real world representation and make movement world-aware.

## Delivered
- `WorldGrid` tile model
- blocked vs walkable tiles
- tile/world conversion helpers
- A* pathfinding
- real-time path following
- debug overlay and event log
- blocked tile visualization

## Milestone 2.1 Stabilization
- safer `WorldGrid` construction
- bounds-safe indexing
- clearer blocked tile colors
- hover blocked state in overlay
- safer test map generation

## Technical Notes
- world data is currently static test data
- traversal is now constrained by the map
- movement still executes immediately from input rather than through a gameplay action layer

## Limitations
- no entity interaction
- no inspect/use verbs
- no combat actions
- no terrain editing

## Next Milestone
Intent-driven actions and first interaction layer
