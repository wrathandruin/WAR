# WAR — Milestone 37 (Persistence Schema / Save-Load / Versioned Migration)

> Current development milestone: M37 — Persistence Schema / Save-Load / Versioned Migration

## Focus
Begin Phase 3 by giving the authoritative localhost lane a versioned persistence surface that can survive restart, migrate older save payloads forward safely, and keep persistence ownership in the correct shared/server runtime boundaries.

M36 signed off the localhost authority and replication hardening pass.
M37 builds on that by introducing authoritative save/load flow, versioned persistence schema handling, and migration-safe read behavior without regressing the split workspace baseline.

## What this milestone does
- introduces a versioned authoritative persistence save under `Runtime/Saves/authoritative_world_primary.txt`
- loads authoritative world state from the save file during host boot when present
- writes authoritative save state atomically from the host during autosave and shutdown
- records persistence diagnostics in shared simulation state so desktop and server can both surface the same truth
- supports migration-safe loading from schema version `1` into canonical schema version `2`
- preserves the M36 localhost authority lane and keeps persistence out of desktop-only ownership

## Persistence behavior after M37
The current localhost authority stack should now read as:

- `WARShared`: owns persistence-facing simulation state and diagnostics
- `WARServer.exe`: owns authoritative load/save decisions and file publication
- `WAR.exe`: presents persistence status, snapshot status, and authority diagnostics
- `WARLegacy`: remains available only as fallback during transition

## Validation procedure
1. Build the split targets in the normal Windows/MSBuild lane.
2. Launch `WARServer.exe`.
3. Confirm `Runtime/Saves/authoritative_world_primary.txt` is created after autosave or shutdown.
4. Relaunch `WARServer.exe` and confirm persisted state is loaded.
5. Launch `WAR.exe` against the host and review overlay/bgfx persistence diagnostics.
6. Run the staged M37 persistence drill with `scripts/acceptance_m37_persistence_win64.bat`.
7. Re-run the staged local demo package, staged headless host smoke test, and staged M36 acceptance lane to confirm no regression.

## Known limits
- localhost/file-backed authority only
- no final network transport
- single authoritative save slot for this increment
- no broad gameplay-state persistence beyond the current world, entity, path, and event surface
- migrations currently support `schema_version=1` forward into `schema_version=2`

These limits are acceptable for M37 because the goal is a safe persistence foundation, not the final shipping save system.

## Why this matters
Persistence can no longer wait.

Phase 3 depends on save/load correctness before inventory, hazards, and combat harden around the wrong ownership model.
M37 puts persistence into the shared/server side of the split architecture now, while the repo is still flexible enough to do it cleanly.

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

Local textures under `assets/textures/` remain machine-local only and must not be committed.

## Next Milestone
### M38 — Actor Runtime / Inventory / Equipment / Loot
- extend the authoritative persistent surface to actor inventory and equipment state
- keep shared/server ownership clean while the split workspace continues to settle
- preserve the M36 authority acceptance lane and M37 persistence correctness while gameplay breadth expands
