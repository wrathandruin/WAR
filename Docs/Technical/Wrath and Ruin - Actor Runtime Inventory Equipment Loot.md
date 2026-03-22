# Wrath and Ruin - Actor Runtime Inventory Equipment Loot

## Purpose

This document defines the M38 gameplay-runtime increment that follows the signed-off M37 persistence base.

It exists to keep actor runtime, inventory, equipment, and loot inside the correct shared/server ownership model while preserving the M36 authority lane and the split workspace baseline.

## Scope

M38 covers:

- player actor runtime
- starter equipment
- inventory stacks
- equipped weapon / suit / tool state
- container loot profiles
- one-time container depletion
- replication of actor runtime and loot state
- persistence of actor runtime and loot state

M38 does **not** cover:

- full inventory UI
- item economy
- crafting
- consumable activation UX
- combat item effects
- armor mitigation rules beyond simple actor-state representation

## Ownership Rules

### Shared
`WARShared` owns:

- player actor runtime state shape
- inventory stack shape
- equipment slot state
- loot profile application
- authoritative snapshot serialization for actor runtime
- save/load persistence state shape

### Server
`WARServer` owns:

- authoritative save publication
- authoritative load on boot
- migration handling during persisted load
- autosave cadence
- validation that actor runtime comes back from saved state

### Desktop
`WAR` owns:

- input surface
- presentation diagnostics
- visual proof that actor runtime and inventory state are changing
- no gameplay-truth ownership

## First-Pass Equipment Rule

M38 intentionally uses a safe first-pass rule:

- if the player acquires the first compatible weapon, suit, or tool and the slot is empty, it auto-equips
- suits may replace the current suit if the incoming armor bonus is strictly better

This keeps the increment small and avoids inventing a desktop-only equipment UI before the shared/runtime model is in place.

## Loot Rule

Container loot is profile-driven and one-time:

- crates and lockers may advertise a `lootProfileId`
- once looted, `lootClaimed` is persisted and replicated
- reopening a depleted container must not grant duplicate items

This is a real persistence requirement, not polish.

## Persistence Rule

The authoritative save must now preserve:

- `simulation_ticks`
- `last_processed_intent_sequence`
- `next_intent_sequence`
- actor runtime fields
- inventory stacks
- equipped state
- container loot-claimed state

Migration-safe load handling must tolerate older schema payloads that do not yet contain actor runtime.

## Validation Before M39

M38 is not healthy unless all of the following remain true:

- M36 localhost authority regression still passes
- the staged package still launches `WAR.exe` and `WARServer.exe`
- a saved authoritative world restores simulation tick continuity
- a migrated legacy save lands as the current schema
- actor runtime fields are present in the saved payload
- looted containers stay depleted after restart

## Known Limits

These limits are acceptable at M38:

- diagnostics-first actor visibility
- auto-equip instead of full equip UI
- no item activation verbs beyond existing interact flow
- no combat use of the equipped weapon yet
- no durability, rarity, or balance pass yet
