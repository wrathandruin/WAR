# Wrath and Ruin - Authoritative Intent Protocol

## Purpose

This document defines the localhost authoritative intent contract introduced in M35 and carried forward through the M36 hardening pass.

It is intentionally not the final networking stack.
It is the first production-safe protocol surface that proves:

- the client submits gameplay requests instead of directly owning truth
- the host validates and resolves those requests
- the client can reconcile to host-owned state
- diagnostics can explain accepted, rejected, corrected, and unreadable-state outcomes

## Scope

This protocol covers:

- movement request submission
- inspect/interact request submission
- host validation
- host acknowledgement
- authoritative snapshot publication
- client prediction plus reconciliation
- queue-based localhost diagnostics

M36 adds hardening around timestamp preservation, atomic snapshot publication, and explicit read-failure reporting.

## Localhost protocol shape

The current localhost protocol is file-backed under `Runtime/Host`:

- `IntentQueue/`
- `AckQueue/`
- `authoritative_snapshot.txt`
- `headless_host_status.txt`

This remains intentionally narrow and local-only.
Its job is to prove the authority seam and its support surface, not to masquerade as final transport.

## Ownership rules

### Client owns
- input capture
- camera
- rendering
- short-lived prediction
- diagnostics
- writing requests to the host queue
- applying acknowledgements and snapshots

### Host owns
- request validation
- authoritative simulation advancement
- acknowledgement generation
- authoritative snapshot publication
- heartbeat and queue-state publication

### Shared runtime owns
- world-state structures
- movement and interaction rules
- fixed-step progression contract
- entity and path state representation

## Request contract

Each request includes:

- sequence number
- request type
- target tile

Current request types:

- move
- inspect
- interact

## Acknowledgement contract

The host writes an acknowledgement per request with:

- sequence number
- request type
- accepted or rejected result
- reason text
- host simulation tick
- original publication timestamp

M36 hardening rule:
acknowledgements must preserve their original publication timestamp even when delivery is delayed by the replication harness.

## Snapshot contract

The host publishes an authoritative snapshot with:

- host simulation tick
- last processed request sequence
- original publication timestamp
- authoritative player position
- active movement target state
- current path state
- replicated entity state
- recent event log entries

M36 hardening rules:
- snapshot publication must be atomic
- malformed or missing snapshot fields must report as read failures, not disappear silently
- delayed delivery must preserve the original publication timestamp

## Known limits

The current protocol still has deliberate limits:

- file-backed only
- localhost only
- no sockets
- no packet framing
- no reliability or retransmission beyond file presence/sequence handling
- no remote auth/session model
- no compression or bandwidth budgeting

These limits are acceptable at M36 because the current goal is runtime proof and diagnostics quality.
They are not acceptable as a final hosted transport.

## Sign-off expectations before M37

Before persistence work begins, the team must be able to prove:

- requests are host-owned, not client-owned
- acknowledgements carry stable publication timing
- snapshots carry stable publication timing
- snapshot writes are atomic
- malformed snapshot reads produce visible failures
- queue visibility exists for request, acknowledgement, and snapshot lanes
- the localhost authority lane remains intelligible under harness-induced delay

## Why this is the right cut

This remains the smallest real authority protocol that:

- proves client intent instead of client truth
- proves host validation in runtime behavior
- proves that timing and failure modes are diagnosable
- creates a safe base for M37 persistence instead of layering persistence onto opaque behavior
