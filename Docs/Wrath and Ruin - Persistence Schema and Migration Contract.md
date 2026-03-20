# Wrath and Ruin - Persistence Schema and Migration Contract

## Purpose

This document defines the first authoritative persistence contract for the split WAR workspace.

It exists to ensure that save/load work lands in the correct ownership boundary:

- shared runtime owns persistent game-state shape
- server owns authoritative load/save decisions
- desktop presents persistence state and diagnostics
- legacy fallback remains available without regressing the new lane

## Scope Of M37

M37 is intentionally a foundation milestone, not a breadth milestone.

This increment covers:

- versioned persistence schema
- authoritative save publication
- authoritative save load on host boot
- migration-safe handling of older schema versions
- diagnostics that expose persistence truth in desktop and server lanes

This increment does not yet cover:

- multiple named save slots
- cloud sync
- account-bound save ownership
- final content breadth persistence
- production rollback beyond atomic file publication
- full deploy-time backup policy

## Current Canonical Save Slot

The current authoritative slot is:

```text
Runtime/Saves/authoritative_world_primary.txt
```

This slot is runtime-only mutable data.

## Schema Version Rules

### Canonical current version

- `schema_version=2`

### Supported older version

- `schema_version=1`

### Rule

- the writer always publishes the current canonical version
- the reader may migrate supported older versions forward in memory
- after a successful migration from an older supported version, the host should republish the canonical current version

## Persisted Surface In This Increment

The authoritative save currently persists:

- simulation ticks
- last processed intent sequence
- authoritative player position
- movement target state
- current path and path index
- entity state
- event log
- save slot name
- save epoch

This is enough for a safe first persistence foundation without prematurely widening scope.

## Ownership Boundaries

### `WARShared`

Owns:

- shared persistence-facing simulation shape
- diagnostics for load/save status
- snapshot application and export helpers

### `WARServer`

Owns:

- save-file read on authoritative boot
- periodic autosave
- shutdown save
- migration-driven canonical republish

### `WAR`

Owns:

- visibility only
- overlay status
- bgfx status text
- replication-status file output

### `WARLegacy`

Owns:

- nothing new for persistence design
- fallback only during transition

## Save Publication Rules

1. Save publication must be atomic.
2. Partial writes must never become accepted persistent state.
3. The host remains the only writer of authoritative save content.
4. The client must never become the canonical save writer.

## Load Rules

1. The host attempts load on boot.
2. Missing save is valid and should fall back to a fresh initialized world.
3. Malformed or unsupported save payloads must fail visibly.
4. A failed load must not silently mutate the running world.

## Migration Rules

1. Migration must be explicit by schema version.
2. Unsupported versions must fail closed.
3. Supported older versions may be defaulted and upgraded in memory.
4. A migrated save should be republished in canonical current format as soon as practical.

## Validation Procedure

1. Delete or move the current authoritative save slot and boot `WARServer.exe`.
2. Confirm fresh-world startup succeeds.
3. Let autosave publish the first canonical save.
4. Restart `WARServer.exe` and confirm the save is loaded.
5. Launch `WAR.exe` and confirm persistence diagnostics are visible.
6. Run the staged M37 persistence drill with `scripts/acceptance_m37_persistence_win64.bat`.
7. Re-run the staged local demo package.
8. Re-run the staged headless host smoke test.
9. Re-run the staged M36 acceptance lane to confirm no regression.

## Acceptance Criteria Before M38

- authoritative save file is written atomically
- authoritative host loads persisted state on boot when present
- missing save falls back cleanly to a fresh world
- malformed save fails visibly
- supported older schema versions migrate safely
- desktop diagnostics surface persistence status
- persisted load restores simulation ticks and last processed intent bookkeeping
- localhost authority validation remains green after persistence lands

## Strategic Reason

Persistence must land before actor runtime, inventory, hazards, and combat harden around ephemeral state.

M37 is the correct moment because the split workspace baseline now exists and the authoritative localhost lane is already validated.
