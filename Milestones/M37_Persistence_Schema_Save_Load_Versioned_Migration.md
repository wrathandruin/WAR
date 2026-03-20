# Title

M37 - Persistence Schema / Save-Load / Versioned Migration

# Description

This milestone introduces the first authoritative persistence lane for the split workspace baseline.

The save/load surface now lives in the correct ownership model:

- shared runtime owns persistent state shape and diagnostics
- `WARServer.exe` owns authoritative load/save decisions
- `WAR.exe` surfaces persistence status and validation signals
- `WARLegacy` remains fallback-only during transition

This is intentionally a safe incremental milestone.
It does not widen gameplay breadth yet.
Its job is to make later actor, inventory, hazard, and combat work land on persistent authoritative state instead of temporary runtime-only assumptions.

# Download

WAR_M37_code_package.zip

The milestone document is included inside the package at:

Milestones/M37_Persistence_Schema_Save_Load_Versioned_Migration.md

# Included

- versioned authoritative save slot under `Runtime/Saves/authoritative_world_primary.txt`
- host boot-time load from authoritative save when present
- atomic autosave and shutdown save publication
- migration-safe support for `schema_version=1` forward into canonical `schema_version=2`
- shared persistence diagnostics surfaced in desktop and server lanes
- README and technical documentation updates

# What should be seen visually

- startup should report M37 and persistence activation
- desktop diagnostics should show persistence schema, save/load counts, save/load success state, and migration state
- host status should expose persistence save presence and last save/load metadata
- restarting `WARServer.exe` after autosave should restore persisted authoritative state
- the localhost authority lane should continue to behave correctly while persistence is active
- the staged package should include an M37 persistence drill script and report path for save/load/migration validation

# Why this is important

M37 is the first point where the project stops treating runtime state as disposable.

That matters because M38-M40 need persistence to exist before actor runtime, inventory, hazards, and combat become more expensive to retrofit.

Landing persistence now also protects the split workspace direction by keeping save/load responsibilities out of desktop-only code.

Sign-off for M37 also requires a packaged save/load/migration drill and a clean rerun of the existing M36 authority validation lane.

# What should be coming up in the next milestone

M38 - Actor Runtime / Inventory / Equipment / Loot
