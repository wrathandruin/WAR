# Wrath and Ruin - M36 Closeout Implementation Report

Date: 2026-03-20

## Outcome

M36 is signed off.

The Windows build, staged package lane, staged headless-host smoke lane, and staged M36 acceptance lane now pass.

M37 is unblocked from the M36 authority/replication gate.

## What was completed

### 1. Packaged runtime-path isolation

The staged local demo package now resolves its own package-local `runtime/` directory instead of falling back into repo-root `Runtime/` when launched from `out/local_demo/...`.

This was the critical fix that made the packaged headless-host smoke lane pass.

### 2. Client-side localhost authority loop

The client now properly completes the localhost authority path while the headless host is online:

- gameplay input is predicted locally for responsiveness
- intent requests are also written through the localhost protocol
- host acknowledgements are polled and applied
- authoritative snapshots are read and reconciled
- snapshot age and failure state are tracked in client diagnostics

### 3. Client replication status emission

The client now writes:

- `runtime/Logs/client_replication_status.txt`

This file is machine-readable and supports the M36 acceptance lane.

It includes:

- host heartbeat visibility
- protocol-lane visibility
- snapshot age
- correction/divergence counters
- snapshot read-failure visibility
- host queue visibility

### 4. Harness controls

The documented runtime controls are now wired:

- `J` toggle harness enabled/disabled
- `K` cycle latency preset
- `L` cycle jitter preset

These changes persist into `Runtime/Config/replication_harness.cfg`.

### 5. Diagnostics and presentation updates

The GDI/bgfx diagnostics surface was updated so the client can show:

- client-prediction state
- harness state
- snapshot age
- correction/divergence counters
- snapshot read failures
- host queue visibility

### 6. Acceptance/package script hardening

The M36 acceptance script was hardened so it can:

- stage and read coherent client status snapshots
- validate snapshot-age growth under harness delay
- generate a final pass/fail report
- clean up spawned `WAR.exe` processes after execution

### 7. Diagnostics branding cleanup

The packaged startup report now correctly labels itself as an M36 report instead of the older M32 milestone text.

## Files changed

- `src/engine/core/RuntimePaths.cpp`
- `src/engine/core/LocalDemoDiagnostics.cpp`
- `src/game/GameLayer.cpp`
- `src/engine/render/DebugOverlayRenderer.cpp`
- `src/engine/render/BgfxWorldRenderer.cpp`
- `scripts/acceptance_m36_localhost_authority_win64.bat`
- `README.md`
- `Milestones/M36_Replication_Latency_Harness_Divergence_Diagnostics.md`
- `Milestones/WAR_Strategic_Roadmap.md`
- `Docs/Reports/Wrath and Ruin - M36 Sign-Off Audit.md`

## Validation completed

Validated on the real Windows/MSBuild toolchain:

- `WAR.vcxproj` `Debug|x64` rebuild: PASS
- warning count: `0`
- error count: `0`
- staged local demo package: PASS
- staged local demo smoke test: PASS
- staged headless-host smoke test: PASS
- staged M36 acceptance script: PASS

Primary runtime artifacts verified:

- `runtime/Logs/local_demo_startup_report.txt`
- `runtime/Logs/client_replication_status.txt`
- `runtime/Logs/m36_acceptance_report.txt`
- `runtime/Host/headless_host_status.txt`
- `runtime/Host/authoritative_snapshot.txt`

## Residual note

The final acceptance report is correct and returns `PASS`.

There is still some minor console-output formatting noise in the batch-script echo lines during execution, but it is non-blocking because:

- the script exits `0`
- the generated `m36_acceptance_report.txt` is correct
- the actual pass/fail gate is now reliable

If desired, that console-echo polish can be cleaned as a small follow-up, but it does not block M37.

## Recommendation to Lead Developer

Proceed to M37.

Do not reopen M36 unless:

- the localhost authority contract changes materially
- the acceptance script needs additional polish
- the package/runtime layout changes again
