# WAR — Milestone 36 (Replication / Latency Harness / Divergence Diagnostics)

> Current development milestone: M36 — Replication / Latency Harness / Divergence Diagnostics

## Focus
Harden the localhost authoritative lane so it is observable, testable, and supportable before persistence work begins.

M35 moved movement and interaction requests into the headless host through the file-backed localhost authority lane.
M36 closes that subsection by adding latency and jitter harness controls, queue visibility, divergence diagnostics, snapshot read failure surfacing, and an acceptance lane the team can run repeatedly.

## What this milestone does
- adds a shared replication harness config under `Runtime/Config/replication_harness.cfg`
- simulates latency for inbound intents, outbound acknowledgements, and outbound snapshots
- preserves original publication timestamps for acknowledgements and snapshots under delayed delivery
- publishes authoritative snapshots atomically through temp-file write plus rename
- surfaces snapshot read failures in diagnostics and event log instead of failing silently
- hardens host freshness so malformed or incomplete heartbeat files cannot report the host as online
- adds a machine-readable client replication status file for acceptance and diagnostics review
- upgrades the local Windows packaging lane to locate `MSBuild.exe` through `vswhere.exe`
- adds an M36 acceptance script for localhost authority review

## Local validation procedure
1. Build or stage with `scripts/build_local_demo_package_win64.bat Release`
2. Launch the headless host with `scripts/launch_headless_host_win64.bat`
3. Launch the client against the host with `scripts/launch_local_client_against_host_win64.bat`
4. Verify in the overlay that authority is `headless-host`, host heartbeat is online, and queue visibility is present
5. Toggle the harness with `J`, cycle latency with `K`, and cycle jitter with `L`
6. Confirm snapshot age increases under delayed delivery and that correction/divergence counters remain visible
7. Run `scripts/acceptance_m36_localhost_authority_win64.bat` and review the generated pass/fail report under `Runtime/Logs`

## Known limits of the current localhost protocol
The current authority lane is still intentionally file-backed and local-only.
It is suitable for proving runtime ownership and diagnostics, but it is not a final transport.

Current limits:
- no real socket transport
- no binary serialization or packet framing
- no reliability layer beyond file presence and sequencing
- no session security or remote host discovery
- no bandwidth or packet-loss simulation beyond the current latency/jitter harness
- no rollback or advanced prediction model beyond current correction flow

That is acceptable for M36.
It would not be acceptable to carry these limits forward unchanged into hosted beta work.


Automated note:
- the batch acceptance lane does not synthesize gameplay input
- correction and divergence counters are therefore validated for visibility and reporting, and may remain zero unless the client is actively driven during the run

## Acceptance criteria before M37
M36 is not signed off until all of the following are true:
- the headless host heartbeat cannot report online when the file is missing or malformed
- host acknowledgements and snapshots preserve original publication timestamps through delayed delivery
- authoritative snapshots are written atomically
- snapshot read failures show up in diagnostics and event log instead of disappearing silently
- queue visibility is available for inbound intents, outbound acknowledgements, and pending snapshots
- the acceptance script produces a pass/fail report covering heartbeat, snapshot age growth, correction count visibility, divergence count visibility, and queue visibility
- the local Windows package/build lane no longer assumes `msbuild` is on `PATH`

## Current audit status
Audit date: 2026-03-20

M36 is signed off.

Verified on the real Windows/MSBuild lane:
- `WAR.vcxproj` `Debug|x64` rebuild passes with `0 warnings` and `0 errors`
- the local demo package stages successfully under `out/local_demo/WAR_M36_Debug`
- the staged local demo smoke test passes
- the staged headless-host smoke test passes
- the staged acceptance script passes and writes a final `PASS` report under `runtime/Logs/m36_acceptance_report.txt`
- the split targets `WAR`, `WARShared`, `WARServer`, and `WARLegacy` all build successfully
- the staged bundle now carries `WAR.exe` and `WARServer.exe` as separate client and host outputs

M37 is unblocked from the M36 sign-off perspective.
The pre-M37 workspace split baseline is also validated.

## Public Repo Hygiene
- only `assets/shaders/` is treated as canonical source-controlled asset content
- local textures and images under `assets/textures/` remain on disk for development but are not part of the public Git payload
- runtime data, packaged bundles, and build outputs remain ignored and disposable

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

## Why this matters
M36 is the finish line for the authority subsection, not the start of a networking sprawl pass.

The project now has:
- shared simulation ownership
- a separate headless host runtime
- host-owned movement and interaction resolution
- observable localhost replication behavior under stress
- a repeatable validation lane for authority review

That is the correct base before versioned persistence enters the runtime in M37.

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

And the shared sprite atlas at:

```text
assets/textures/world_atlas.bmp
```

The texture atlas is a local development asset and is intentionally not versioned in Git.
Only the shader pipeline under `assets/shaders/` is treated as canonical source-controlled asset content in this public repo.

## Next Milestone
### M37 — Persistence Schema / Save-Load / Versioned Migration
- introduce versioned save schema and migration rules
- persist authoritative slice state safely
- prepare the runtime for hosted persistence work instead of transient demo-only state
