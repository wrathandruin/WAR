# Wrath and Ruin - M36 Sign-Off Audit

Audit date: 2026-03-20

## Summary

M36 is signed off.

The milestone now clears the full Windows build, packaging, smoke, and acceptance gate required before M37.

The correct status is:
- build-ready
- package-staging-ready
- acceptance-ready
- sign-off-ready

## What was verified

The following checks were run on the real Windows/MSBuild lane:

- `WAR.vcxproj` `Debug|x64` build: PASS
- current build warning count: `0`
- local demo package staging: PASS
- staged local demo smoke test: PASS
- staged headless-host smoke test: PASS
- staged M36 acceptance script: PASS

## Findings

### 1. Package-local runtime isolation was fixed

When the staged package lives under `WAR/out/local_demo`, runtime path discovery now detects the staged local demo layout and keeps runtime traffic inside the package-local `runtime/` directory.

This prevents packaged host/runtime files from falling back into:

- `WAR/Runtime`

and correctly keeps them inside:

- `WAR/out/local_demo/WAR_M36_Debug/runtime`

This change is what allowed the staged headless-host smoke test to pass.

## 2. The client-side localhost authority lane was completed

The client now:

- routes requests through the localhost intent protocol while the headless host is online
- keeps client-side prediction active for responsiveness
- polls host acknowledgements
- reads and applies authoritative snapshots
- writes machine-readable replication status for diagnostics and automation

## 3. The advertised acceptance lane now completes

The client now emits the machine-readable replication status file expected by the acceptance script:

- `Runtime/Logs/client_replication_status.txt`

The staged acceptance script now completes and produces a final `PASS` report.

## 4. Published harness controls are wired

The public M36 controls describe:

- `J`: toggle latency harness
- `K`: cycle latency preset
- `L`: cycle jitter preset

Those controls are now wired in the current client input path and persist their values to `Runtime/Config/replication_harness.cfg`.

## Release recommendation

Advance to M37.

The M36 gate has been closed successfully.

## Exit criteria for M36 sign-off

M36 sign-off was granted once all of the following became true:

- staged packages write runtime files into their own package-local `runtime/` directory
- staged headless-host smoke test passes
- the client submits requests through the localhost authority protocol instead of only local simulation enqueue
- the client emits `client_replication_status.txt`
- the M36 acceptance script completes and produces a final `PASS` report
- the published `J / K / L` controls operate as documented

## Decision

Current recommendation: GO for M37.
