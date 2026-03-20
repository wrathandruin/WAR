# M36 - Replication / Latency Harness / Divergence Diagnostics

## Description

M36 hardens the localhost authoritative lane by adding latency/jitter harness support, queue visibility, snapshot age visibility, snapshot read failure surfacing, and acceptance-lane automation.

This is not a breadth milestone.
It is a hardening and sign-off milestone for the authority subsection before M37 persistence work begins.

## Included

- replication harness config support under `Runtime/Config/replication_harness.cfg`
- host-side simulated latency for intent arrival, acknowledgement delivery, and snapshot delivery
- preserved original publication timestamps under delayed delivery
- atomic authoritative snapshot publication
- client-side visibility for snapshot age, snapshot read failures, drift distance, correction count, and divergence count
- host queue visibility for pending inbound intents, pending outbound acknowledgements, and pending snapshots
- an automated M36 acceptance script
- Windows package/build script resolution through `vswhere.exe`

## What should be seen visually

- the client should remain coherent under controlled latency
- snapshot age should be visible and should grow under delayed delivery
- divergence corrections should be visible in diagnostics and event log
- snapshot read failures should be visible instead of silent
- host queue state should be visible
- latency and jitter presets should be toggleable without rebuilding

## Validation procedure

1. Build/stage with `scripts/build_local_demo_package_win64.bat Release`
2. Launch the host with `scripts/launch_headless_host_win64.bat`
3. Launch the client with `scripts/launch_local_client_against_host_win64.bat`
4. Toggle harness controls and review overlay/bgfx diagnostics
5. Run `scripts/acceptance_m36_localhost_authority_win64.bat`
6. Review `Runtime/Logs/m36_acceptance_report.txt`

## Known limits

- localhost only
- file-backed transport only
- no real socket transport
- no final reliability/session model
- no rollback model beyond current prediction plus correction

These limits are acceptable for M36 and are intentionally not solved here.


Automated note:
- the batch acceptance lane does not synthesize gameplay input
- correction and divergence counters are therefore validated for visibility and reporting, and may remain zero unless the client is actively driven during the run

## Acceptance criteria before M37

- heartbeat freshness must fail closed on malformed or missing data
- acknowledgements and snapshots must preserve original publication timestamps
- snapshot publication must be atomic
- snapshot read failures must surface in diagnostics and event log
- queue visibility must be present
- the M36 acceptance script must pass

## Current sign-off status

Audit date: 2026-03-20

M36 is signed off.

Verified during audit:
- `WAR.vcxproj` builds successfully on the real Windows/MSBuild lane with 0 warnings and 0 errors
- the local demo package stages successfully
- the staged local demo smoke test passes
- the staged headless-host smoke test passes
- the staged M36 acceptance script passes and produces a final `PASS` report

Completed during closeout:
- packaged runtime-path resolution now prefers the staged package runtime when a real local demo bundle is detected
- the client now submits localhost intent requests while the headless host is online and reconciles to authoritative snapshots
- `runtime/Logs/client_replication_status.txt` is emitted for diagnostics and acceptance automation
- the published harness hotkeys (`J / K / L`) are now wired

M37 is unblocked from the M36 sign-off perspective.

## Why this is important

M36 makes future gameplay and persistence work safer because the authoritative lane is now observable, debuggable, and sign-off ready under local stress instead of only looking correct under ideal timing.

## What should be coming up in the next milestone

M37 - Persistence Schema / Save-Load / Versioned Migration
