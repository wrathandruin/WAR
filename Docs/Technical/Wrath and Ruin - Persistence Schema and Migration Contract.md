# Wrath and Ruin - Persistence Schema and Migration Contract

## Purpose

This document defines the authoritative persistence contract established in M37 and extended in M38.

It exists to prevent persistence drift while the workspace is still transitioning through split desktop/server/shared ownership.

## Canonical Persistence Surface

The authoritative save lives under runtime-only mutable state:

```text
Runtime/Saves/authoritative_world_primary.txt
```

In packaged execution the same file lives under the executable-local runtime root:

```text
runtime/Saves/authoritative_world_primary.txt
```

This file is never canonical source content.

## Current Schema

Current persistence schema version:

- `3`

### Schema 3 fields include

- simulation tick state
- last processed intent sequence
- next intent sequence
- player position
- movement/path state
- replicated world entities
- player actor runtime
- inventory stacks
- equipped state
- loot collection count
- event log tail

## Migration Rules

### Supported legacy input
The runtime must tolerate legacy payloads that contain only the earlier snapshot surface:

- no schema field
- no actor runtime fields
- no inventory fields
- no equipment fields
- no loot-depletion fields beyond earlier entity state

These loads are treated as schema `1` inputs.

### Migration behavior
When a schema `1` payload is loaded:

- the runtime must restore simulation ticks and processed-intent state
- missing actor runtime fields are default-seeded safely
- the next save must publish the current schema
- the current save must record `migrated_from_schema_version=1`

## Authority Rule

Only the authoritative host publishes saves.

The desktop client may inspect state and surface diagnostics, but it must not own save publication or canonical load decisions.

## Validation Rule

Before moving beyond M38, the repo must prove:

- a legacy seed payload migrates to schema 3
- `simulation_ticks` survive restart
- intent sequence bookkeeping survives restart
- actor runtime fields appear in the saved payload
- staged package save/load drill passes
