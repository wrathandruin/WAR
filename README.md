# WAR — Milestone 38 (Actor Runtime / Inventory / Equipment / Loot)

> Current development milestone: M38 — Actor Runtime / Inventory / Equipment / Loot

## Focus
Turn the player runtime into a real authored gameplay surface on top of the signed-off authority and persistence base.

M37 established versioned persistence and authoritative save/load direction.
M38 builds on that base by introducing a durable player actor runtime, inventory state, starter equipment, container loot profiles, and replicated actor-state visibility without regressing the localhost authority lane or the split workspace baseline.

## What this milestone does
- adds a real player actor runtime with health, armor, inventory, and equipped weapon / suit / tool state
- seeds the player with a small starter loadout so actor state is visible immediately
- extends world entities with authored loot profiles and one-time loot-claimed state
- turns crates and lockers into real loot carriers instead of only open/closed props
- auto-equips compatible gear in safe first-pass fashion when the player recovers it
- extends authoritative snapshots so actor runtime and loot state replicate cleanly through the localhost host lane
- keeps authoritative save/load on the host side and persists actor runtime into the versioned save payload
- adds an M38 persistence and migration acceptance drill while keeping the M36 regression lane staged

## Manual review lane
1. Build or stage with `scripts/build_local_demo_package_win64.bat Release`
2. Launch the host with `scripts/launch_headless_host_win64.bat`
3. Launch the client with `scripts/launch_local_client_against_host_win64.bat`
4. Open nearby crates and lockers in Cargo Bay, Med Lab, and Hazard Containment
5. Confirm the overlay updates health, armor, inventory counts, equipped items, and loot collections
6. Restart the host and confirm the saved authoritative world restores actor runtime and looted container state

## Automated validation
- run `scripts/acceptance_m36_localhost_authority_win64.bat` as the regression authority lane
- run `scripts/acceptance_m38_persistence_inventory_win64.bat` to validate save migration, tick restoration, sequence restoration, and staged persistence wiring

## Current limits
This is still an intentionally narrow actor-runtime milestone.

Current limits:
- no explicit inventory UI shell yet beyond diagnostics
- no player-driven equip/unequip controls yet
- no weight or encumbrance penalties yet
- no combat use of equipped items yet
- no item rarity, economy, vendor, or crafting model yet

Those are acceptable for M38.
They should not be solved by dragging desktop-only logic back into shared/server ownership.

## Public Repo Hygiene
- only `assets/shaders/` is treated as canonical source-controlled asset content
- local textures and images under `assets/textures/` remain on disk for development but are not part of the public Git payload
- runtime data, packaged bundles, and build outputs remain ignored and disposable

## Demo controls
- `LMB`: move / set movement target
- `RMB`: interact / loot / use
- `Shift + RMB`: inspect
- `MMB drag`: pan camera
- `Mouse wheel`: zoom
- `O`: toggle region boundary overlay
- `H`: toggle authored hotspot overlay
- `7 / 8 / 9`: Default / Muted / Vivid palette modes
- `J`: toggle replication latency harness
- `K`: cycle latency preset
- `L`: cycle jitter preset

## Why this matters
M38 makes the first persistent player-facing state loop real.

The project now has:
- signed-off localhost authority and replication diagnostics
- authoritative save/load direction
- a real player actor runtime
- recoverable loot
- equipped gear that can carry forward into hazards and combat work

That is the correct base before survival, terrain consequence, and six-second combat expand in M39 and M40.

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

The texture atlas remains a local development asset and is intentionally not versioned in Git.
Only the shader pipeline under `assets/shaders/` is treated as canonical source-controlled asset content in this public repo.

## Next Milestone
### M39 — Survival Hazards / Terrain Consequence / World State
- make environmental pressure matter
- bind actor runtime and persistence into hazard consequences
- prepare the gameplay loop for six-second encounter resolution
