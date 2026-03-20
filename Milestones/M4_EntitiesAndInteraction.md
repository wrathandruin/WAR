# Milestone 4 — Entities & Interaction

## Goal
Add lightweight world entities and make inspection report more than tile state.

## Delivered
- lightweight `Entity` type
- entity storage in `GameLayer`
- tile-based entity lookup
- inspect now reports tile state and entity presence
- simple world entity rendering

## Technical Notes
- this is intentionally not a full ECS
- entities are lightweight and local to the game layer for now
- this milestone extends the M3 action system instead of replacing it

## Limitations
- no contextual interact/use action yet
- entities have no behavior yet
- no persistence yet

## Next Milestone
Context-sensitive interaction verbs and first gameplay objects.
