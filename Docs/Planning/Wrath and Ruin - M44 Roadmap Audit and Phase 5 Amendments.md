# Wrath and Ruin - M44 Roadmap Audit and Phase 5 Amendments

## 1. Executive Call

The 48-step plan is still directionally correct.
It successfully pulled the project out of a renderer-first trap and into a production-shaped vertical slice.

However, the plan is now too optimistic in four areas that matter before alpha hardening can be considered honest:

- the current repo still proves a localhost authority lane, not a real hosted session flow
- the current slice feels like a directed top-down RPG more than a graphical MUD session
- the current repo still lacks explicit identity, security, and live-operations foundations
- the current content path is system-first and code-authored, not yet obviously scalable for mission and text production

The right move is not to restart the roadmap.
The right move is to amend Phase 5 so it closes those gaps explicitly instead of assuming they will emerge automatically from "hardening."

That amendment has now been expanded into the detailed continuation runway at:

- `Docs/Planning/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md`

## 2. What The Current Repo Actually Proves

As of the pre-M44 audit checkpoint, the active `WAR` repo now proves:

- split client/server/shared workspace direction
- packaged Windows demo lane
- headless authoritative host bootstrap
- localhost authority and reconciliation discipline
- versioned persistence and restore
- actor, inventory, hazard, and six-second combat runtime
- one directed planetary mission lane
- one docked-ship runtime lane
- one narrow local-orbit runtime lane

That is strong progress.
It is enough to justify moving into the final phase of the current roadmap.

## 3. What Is Still Missing

These are the important gaps that should now be treated as explicit scope, not background assumptions.

### 3.1 Architecture Gaps

- real remote transport is still missing
- hosted session topology is still missing
- account/session/ticket ownership is still missing in this repo
- content and mission definitions are still more code-authored than pipeline-authored
- multi-user presence is not yet a proven product behavior
- the contracts boundary is still thin for future web/account/admin integration

### 3.2 Networking Gaps

- the current authority lane is file-backed localhost, not a real network path
- there is no proven remote connect/disconnect/reconnect loop
- there is no proven packet/protocol versioning strategy for a public-facing client/server lane
- there is no relay or gateway direction defined inside this repo
- there is no authenticated hosted session handoff path yet

### 3.3 Security Gaps

- no explicit trust-boundary document for client, host, launcher, website, and future relay
- no concrete account or session token strategy in this repo
- no transport security plan in this repo
- no rate-limiting or abuse-control plan for a public-facing session lane
- no secrets/configuration/environment model yet comparable to the legacy production posture
- no explicit backup/restore/rollback contract for this repo's future hosted alpha environment

### 3.4 Operations And Support Gaps

- no deployable hosted environment model in this repo yet
- no crash-upload path or structured crash triage loop
- no admin/support surface for live debugging, player support, or intervention
- no moderation or incident-response baseline
- no external playtest onboarding/support workflow

### 3.5 Product Identity Gaps

- the current playable slice is mission-bearing and persistent, but not yet convincingly MUD-shaped
- there is no typed command surface in the current client
- there is no local say/emote/social loop in the current client/runtime
- there is no prompt/vitals/text-command identity layer
- there are no authored room or interior entry descriptions yet, so moving into a space does not deliver the immediate text identity a MUD player expects
- there is no proven shared-presence playtest lane that makes the world feel inhabited rather than merely authored

## 4. What The Audit Suggests Should Have Started Earlier

These items were reasonable to defer during renderer and early authority work, but they should now be considered overdue rather than "future nice-to-haves":

- remote hosted-session design
- account/session/ticket integration direction
- threat-model and trust-boundary design
- content-schema and mission-authoring pipeline design
- text/social interaction MVP planning
- admin/support/ops surface planning

The risk is not only technical.
It is evaluative.
If these remain vague, Phase 5 can produce a polished slice that still fails to test the actual product fantasy.

## 5. The Real MVP For Next-Stage Playtesting

If the next stage is about testing whether the game is "fun," the MVP cannot be "a better top-down RPG demo."

The playtest MVP must be a **graphical MUD session**.

That minimum playtest should prove:

- a hosted authoritative session
- one visible world location with mission structure
- movement, inspect, interact, hazards, and combat
- interior or room-entry description text when crossing into meaningful authored spaces
- a visible event/combat log that carries narrative weight
- readable objective and journal-style mission context
- a prompt or status surface that feels like a living session, not just debug text
- a typed command surface for at least a small core set
- local speech and emote support
- at least one shared-presence proof with another player, even if the content scope stays narrow
- persistence across logout/reconnect

## 6. Recommended MUD-Feel MVP Checklist

The minimum "yes, this feels like Wrath and Ruin" playtest should include:

- one hosted shard or controlled hosted session
- two real players or one player plus one parallel operator account if staffing is tight
- graphical traversal and interaction
- room, interior, or region descriptions that fire when entering key authored spaces
- command bar with a small first-class vocabulary such as `say`, `emote`, `look`, `help`, `inv`, and mission/help shortcuts
- event log that surfaces combat, hazard, inspect, mission, and social lines together
- prompt or vitals strip that feels session-like rather than purely diagnostic
- inspect text and authored world flavor on important interactables and regions
- one shared social beat, even if it is small
- one mission beat that requires coordination, observation, or communicated understanding

Without that, the test is mainly measuring top-down readability and mission pacing, not the MUD identity.

## 7. Phase 5 Amendments

The milestone titles can remain the same, but the expectations need to tighten.

### M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline

This milestone should now explicitly prove:

- first real hosted environment for this repo
- remote connect path instead of localhost-only proof
- initial environment/config/secrets model
- telemetry baseline for session, crash, and reconnect visibility
- first MUD-feel MVP surface including room-entry descriptions, prompt/vitals direction, and typed command-bar direction
- first MUD-identity MVP surface planning locked into implementation, not just discussed

### M46 - Alpha Hardening / Crash Capture / Admin Tooling

This milestone should now explicitly prove:

- crash capture and triage
- operator/admin surface for host status and intervention
- backup/restore and rollback expectations for hosted alpha
- first security pass across trust boundaries, tokens, config, and abuse controls
- basic moderation/support readiness for controlled testing

### M47 - External Beta Onboarding / Launcher-Update Direction / Support Flow

This milestone should now explicitly prove:

- account/session/onboarding flow direction
- launcher/updater direction tied to a real hosted product surface
- support workflow for testers
- clearer first-session UX
- accessibility and readability improvements that support text-first identity

### M48 - Market Foundation / Release Ops / Analytics / Readiness Gate

This milestone should now explicitly prove:

- release-ops discipline
- analytics worth acting on
- support and moderation readiness
- environment/runbook credibility
- a go/no-go review that judges the game as a graphical MUD product, not only as a mission slice

## 8. Strategic Rule Going Forward

Do not let Phase 5 collapse into generic polish.

The next phase must answer three questions clearly:

- can this repo run as a hosted product instead of a localhost demo?
- can the slice be evaluated as Wrath and Ruin rather than as a generic top-down RPG?
- can internal and later external testers be supported without heroics?

If those questions are not being answered, the project is not actually finishing the roadmap.
