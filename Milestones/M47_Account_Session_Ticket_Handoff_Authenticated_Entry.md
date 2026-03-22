# M47 - Account Session Ticket Handoff / Authenticated Entry

## Title

M47 - Account Session Ticket Handoff / Authenticated Entry

## Description

M47 introduces the first real player session-entry model.

The milestone backbone is:

- define the first real player session-entry model
- stop coupling client and host casually
- introduce trusted session/ticket handoff
- validate and deny bad or expired entry coherently
- add reconnect/resume identity direction
- keep localhost fallback, hosted bootstrap, and M46 config/secrets discipline intact

## Included

- session entry request contract
- session ticket contract
- issued / denied / active-session ownership model
- host-side validation / deny / fail states
- client entry request submission
- client ticket polling
- granted-session identity capture
- reconnect / resume identity direction
- client-facing session-entry telemetry
- M47 validation scripts
- M46 hardening preservation for build staging and validation scripts

## Validation lane

M47 validation scripts are:

- `scripts/validate_m47_ticket_issue_and_client_entry_win64.bat`
- `scripts/validate_m47_ticket_denial_and_fail_states_win64.bat`
- `scripts/validate_m47_reconnect_identity_win64.bat`
- `scripts/validate_m47_internal_alpha_package_win64.bat`

These validate that:

- a client receives a trusted issued ticket before authoritative entry is treated as active
- denied and malformed entry requests fail coherently
- reconnect / resume identity is persisted and handed back through the runtime lane
- M45 localhost fallback still passes
- M45 hosted bootstrap still passes
- M46 environment / secrets / runtime-ownership validation still passes

## Validation status

M47 validation scripts are now passing on the real packaged Windows lane.

The aggregate sign-off run is:

- `scripts/validate_m47_internal_alpha_package_win64.bat Release`

That includes:

- M45 localhost fallback smoke
- M45 hosted bootstrap validation
- M46 environment / secrets / runtime-ownership validation
- M47 ticket issue / client entry validation
- M47 denial / fail-state validation
- M47 reconnect / resume identity validation

## Why this is important

M47 is where the slice starts treating entry as a trusted runtime contract rather than an informal assumption that the client and host already belong together.

## What should be coming up next

M48 - Crash Capture / Failure Bundles / Operator Triage Baseline
