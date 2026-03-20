# WAR — Milestone 39 (Survival Hazards / Terrain Consequence / World State)

> Current development milestone: M39 — Survival Hazards / Terrain Consequence / World State

## Focus

Introduce environmental pressure to the slice without breaking the split desktop/shared/server baseline.

M37 established versioned persistence.
M38 introduced actor runtime, inventory, equipment, and loot.
M39 makes the planetary slice dangerous by adding terrain hazards, survival pressure, and stateful world consequence that the authoritative runtime can persist.

## What this milestone does

- adds a terrain-hazard model to `WorldState`
- populates authored radiation, toxic, and vacuum-breach lanes in the test world
- applies survival pressure inside `SimulationRuntime` through hazard ticks, oxygen drain, suit integrity loss, radiation dose, toxic exposure, and health damage
- persists survival state through authoritative snapshots
- surfaces hazard and survival state in overlay, bgfx status text, and machine-readable client replication status
- upgrades staged local demo packaging to M39 naming and adds an M39 hazard acceptance script while preserving the signed-off M36 regression lane

## Local validation procedure

1. Build or stage with `scripts/build_local_demo_package_win64.bat Release`
2. Launch the headless host with `scripts/launch_headless_host_win64.bat`
3. Launch the client against the host with `scripts/launch_local_client_against_host_win64.bat`
4. Move through Hazard Containment and Transit Spine review spaces
5. Confirm overlay/bgfx diagnostics show survival state, current hazard, oxygen pressure, and consequence counters
6. Restart the host and verify authoritative snapshot state still carries survival fields
7. Run `scripts/acceptance_m39_survival_hazards_win64.bat`
8. Re-run `scripts/acceptance_m36_localhost_authority_win64.bat` as the regression lane

## Known limits

- the current hazard model is authored and tile-based, not yet a generalized simulation field system
- no med treatment loop or hazard-remediation gameplay exists yet
- no terrain modification toolchain exists yet
- survival pressure is intentionally narrow and tuned for readability before combat integration
- the file-backed localhost authority lane remains the same signed-off M36 baseline

## Demo controls

- `LMB`: move / set movement target
- `RMB`: interact
- `Shift + RMB`: inspect
- `MMB drag`: pan camera
- `Mouse wheel`: zoom
- `O`: toggle region boundary overlay
- `H`: toggle authored hotspot overlay
- `7 / 8 / 9`: Default / Muted / Vivid palette modes
- `J`: toggle replication latency harness
- `K`: cycle latency preset
- `L`: cycle jitter preset

## Next Milestone

### M40 — Six-Second Combat / Encounter Resolution
