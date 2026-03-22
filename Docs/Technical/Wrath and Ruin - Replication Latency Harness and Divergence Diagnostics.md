# Wrath and Ruin - Replication Latency Harness and Divergence Diagnostics

## Purpose

M36 hardens the localhost authoritative lane established in M35.

The goal is not to broaden networking scope.
The goal is to make the existing lane observable under stress so the team can see timing, correction, queue pressure, and divergence behavior clearly before persistence and broader gameplay layers depend on it.

## What this contract adds

- a shared replication harness config file at `Runtime/Config/replication_harness.cfg`
- controllable latency for client intent arrival at the host
- controllable latency for acknowledgement delivery back to the client
- controllable latency for authoritative snapshot delivery back to the client
- deterministic jitter so the lane can be stressed repeatably
- host queue diagnostics for inbound intents, outbound acknowledgements, and pending snapshots
- client diagnostics for snapshot age, read failures, drift distance, divergence count, and correction count
- a machine-readable client replication status file for acceptance automation
- an M36 acceptance script that validates the localhost lane end to end

## What this is not

This milestone does not introduce:

- real socket transport
- reliability layers
- packet serialization libraries
- encryption
- rollback netcode
- matchmaking
- production remote hosting

Those would be the wrong scope here.
The correct purpose is to make the localhost authoritative lane debuggable and sign-off ready.

## Shared config rules

File:

`Runtime/Config/replication_harness.cfg`

Fields:

- `enabled`
- `intent_latency_ms`
- `ack_latency_ms`
- `snapshot_latency_ms`
- `jitter_ms`

The client is allowed to update this file during local testing.
The headless host is expected to re-read it while running.

## Diagnostics expectations

By the end of M36 the team should be able to answer all of these from overlay, status text, event log, or runtime files:

- Is host authority actually online?
- Is the heartbeat valid or malformed?
- Are intents arriving immediately or under simulated delay?
- How old is the last authoritative snapshot the client applied?
- Are snapshot reads failing, and why?
- Are corrections happening?
- Is the correction only positional, or also path/entity state related?
- Are host outbound queues building up?
- Is jitter active?
- Is the client still coherent under the current harness preset?

If those questions cannot be answered quickly, M36 is incomplete.

## Validation procedure

1. Build/stage with `scripts/build_local_demo_package_win64.bat Release`
2. Start the host with `scripts/launch_headless_host_win64.bat`
3. Start the client with `scripts/launch_local_client_against_host_win64.bat`
4. Confirm overlay/state shows authoritative host ownership and queue visibility
5. Enable the harness and cycle latency/jitter presets
6. Verify snapshot age increases under delayed delivery
7. Run `scripts/acceptance_m36_localhost_authority_win64.bat`
8. Review the report under `Runtime/Logs/m36_acceptance_report.txt`


Automated note:
- the batch acceptance lane does not synthesize gameplay input
- correction and divergence counters are therefore validated for visibility and reporting, and may remain zero unless the client is actively driven during the run

## Acceptance criteria before M37

M36 is ready to sign off only when:

- malformed or missing heartbeat files cannot report host online
- acknowledgement and snapshot timestamps reflect original publication time, not delivery time
- authoritative snapshot writes are atomic
- snapshot read failures are visible in diagnostics and event log
- queue visibility exists for inbound intents, outbound acknowledgements, and pending snapshots
- the acceptance script produces a pass/fail report with snapshot age growth and visible correction/divergence metrics
- the Windows build/package lane resolves `MSBuild.exe` through `vswhere.exe`

## Why this matters before M37

Persistence becomes harder and riskier if authority and replication behavior still feel vague.

M36 exists so that M37 and later milestones inherit:

- clearer runtime evidence
- better supportability
- better bug reports
- lower risk when persistent state starts depending on authoritative progression
