# Wrath and Ruin - Headless World Host Bootstrap

## Purpose

M34 establishes the first separate host-owned runtime lane for WAR.

This milestone is intentionally narrower than a full authoritative networking implementation.
Its job is to prove three things cleanly:

1. the shared simulation runtime can boot outside the client window loop
2. a separate host process can publish diagnosable runtime state
3. the client can detect that host bootstrap state without pretending that M35 protocol work already exists

## Scope Boundary

M34 **does** include:

- headless host executable mode via `WARServer.exe`
- transitional legacy host mode via `WAR.exe --headless-host`
- fixed-step world bootstrap in a separate process
- host startup and heartbeat status publication under `Runtime/Host`
- local launch and smoke-test scripts for host bootstrap review
- client-side visibility into host presence and heartbeat freshness

M34 **does not** claim yet:

- authoritative movement resolution in the host
- a client/server gameplay command protocol
- state replication
- reconciliation or prediction correction

Those are M35 and M36 responsibilities.

## Runtime Contract

Host bootstrap artifacts now belong under:

- `Runtime/Host/`

Expected files include:

- `Runtime/Host/headless_host_status.txt`
- `Runtime/Logs/headless_host_log.txt`

The status file is intentionally simple key/value text so early diagnostics can be inspected by humans and scripts.

## Client Meaning

Until M35 lands, the client still owns immediate local gameplay truth through the shared simulation runtime in-process.

The host bootstrap lane is therefore a **proof-of-runtime-boundary** milestone, not a finished authority lane.
That distinction must remain explicit in docs, diagnostics, and milestone language.

## Review Expectations

A successful M34 review should prove:

- the host can start without the client window
- the host writes a fresh heartbeat/status file while running
- the client can surface whether a host heartbeat is present or stale
- the codebase clearly separates bootstrap host ownership from future protocol work

Current structural note:
- after the workspace split began, `WARServer.exe` became the preferred host process while the old `WAR.exe --headless-host` path remains only as transitional fallback
