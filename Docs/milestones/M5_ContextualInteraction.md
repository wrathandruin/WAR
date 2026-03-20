# Milestone 5 — Contextual Interaction

## Goal
Turn passive entities into interactive world objects.

## Delivered
- `ActionType::Interact`
- `EntityType` for simple object behavior
- right click now interacts
- shift + right click inspects
- context-sensitive interaction responses
- richer overlay and event log

## Technical Notes
- interaction is intentionally simple and action-driven
- this remains a lightweight gameplay layer, not full ECS
- inspect and interact are now separate concepts

## Limitations
- interactions do not yet mutate object state
- no inventory or loot transfer yet
- no persistence for object state yet

## Next Milestone
Stateful objects and first gameplay verbs with consequences.
