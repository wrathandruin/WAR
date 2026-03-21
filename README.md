# WAR — Milestone 40 (Six-Second Combat / Encounter Resolution)

> Current development milestone: M40 — Six-Second Combat / Encounter Resolution

## Focus
Prove one authoritative six-second combat encounter on top of the signed-off authority, persistence, actor-runtime, and survival groundwork.

This milestone is intentionally narrow:
- one readable encounter lane
- one authoritative combat loop
- one persistence-aware encounter outcome
- no broad combat-system sprawl

## What this milestone does
- adds a first authoritative combat encounter runtime
- introduces round-based six-second combat resolution
- persists player actor state, inventory/equipment state, and encounter state through authoritative snapshots
- restores persisted encounter/runtime state on host boot
- surfaces combat state in overlay, client replication status, and bgfx status text
- stages an M40 local demo package and M40 acceptance wrapper

## Manual validation procedure
1. Build/stage with `scripts/build_local_demo_package_win64.bat Release`
2. Launch the headless host with `scripts/launch_headless_host_win64.bat`
3. Launch the client with `scripts/launch_local_client_against_host_win64.bat`
4. Move into `Bridge Access Chokepoint` or `Quarantine Access Gate`
5. Let at least one six-second round resolve
6. Confirm:
   - normal movement feels stable before the encounter starts
   - `client_replication_status.txt` is not showing corrections climbing rapidly with the latency harness disabled
   - combat becomes active
   - hostile health changes
   - player health/armor/suit state changes coherently
   - the event log explains what resolved
7. Close and restart the host, then confirm the save under `runtime/Saves/authoritative_world_primary.txt` restores actor/combat state correctly

## Known limits
- localhost authority lane only
- file-backed transport only
- one narrow encounter profile, not a full combat sandbox
- no ranged cover system yet
- no broad NPC runtime yet
- this is the first encounter proof, not final combat breadth

## Why this matters
M40 is one of the first true alpha proof points.
It turns the slice from movement/inventory/survival scaffolding into a product lane that can now resolve an actual hostile encounter with persistence and authority.
