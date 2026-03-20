# Milestone 3 — Interaction Layer

## Goal
Convert input into intent-driven gameplay actions.

## Delivered
- `Action` type with `Move` and `Inspect`
- `ActionQueue`
- left click now queues a move action
- right click now queues an inspect action
- action processing happens in the game layer
- event log now reflects intent and resolution more clearly

## Technical Notes
- movement is no longer triggered directly from input
- inspect is the first non-movement interaction verb
- this milestone establishes the pattern future gameplay systems will use

## Limitations
- inspect currently reports tile state only
- no object/entity interactions yet
- no contextual use/interact actions yet

## Next Milestone
Entity presence, richer inspection, and deeper interaction verbs.
