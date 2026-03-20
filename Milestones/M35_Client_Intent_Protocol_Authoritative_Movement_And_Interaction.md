# Title

M35 - Client Intent Protocol / Authoritative Movement And Interaction

# Description

This milestone moves movement and interaction truth into the headless host lane through a real localhost intent and acknowledgement contract.

The client now submits requests.
The host validates and resolves them.
The client predicts locally for responsiveness and then reconciles to host-owned snapshots.

This is intentionally still a narrow localhost milestone.
It does not pretend to be the final transport or replication layer.
Its job is to convert server authority from architecture language into gameplay behavior.

# Download

WAR_M35_code_package.zip

The milestone document is included inside the package at:

Milestones/M35_Client_Intent_Protocol_Authoritative_Movement_And_Interaction.md

# Included

- file-backed localhost intent queue
- host acknowledgement queue
- authoritative snapshot publication
- host validation of movement and interaction requests
- client prediction plus reconciliation
- diagnostics for acknowledgements, rejections, snapshot sequence, and corrections
- README and architecture documentation updates

# What should be seen visually

- movement and interactions should still feel responsive
- a running headless host should clearly own authority
- accepted and rejected intents should be visible in diagnostics and event log
- host sequence, snapshot sequence, and correction count should be visible
- the client should no longer read like the direct owner of gameplay-critical movement and interaction truth

# Why this is important

M35 converts authority from a conceptual architecture goal into actual gameplay behavior.

Without this milestone, M34 is only a host bootstrap.
With it, the project begins proving the alpha requirement of hosted authoritative play on a real runtime lane.

It also makes M36 safer because replication, latency, and divergence work now have a real authoritative request/response seam to harden.

# What should be coming up in the next milestone

M36 - Replication / Latency Harness / Divergence Diagnostics
