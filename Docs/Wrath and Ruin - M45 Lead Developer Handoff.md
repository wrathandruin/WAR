# Wrath and Ruin - M45 Lead Developer Handoff

## Purpose

This document is the authoritative onboarding brief for the next lead developer taking over after the Phase 4 closeout.

It exists so the handoff into M45 is:

- specific
- current
- production-minded
- unambiguous

If older planning notes conflict with this document on the immediate next milestone, this document wins for M45 execution.

## Current Verified Checkpoint

The repo is handing off from a completed M40-M44 engineering base.

Current verified state:

- M40 remains signed off and stable
- M41 through M44 are complete enough to hand the project into the next roadmap section
- the split baseline is active:
  - `WAR`
  - `WARShared`
  - `WARServer`
  - `WARLegacy`
- `WARServer.exe` is the preferred host executable
- the Windows release package lane is green
- packaged host smoke passes
- packaged client/host integration smoke passes
- M43 and M44 acceptance wrappers are now truthful post-walkthrough validation rather than false-positive fresh-package checks
- the repo is public-safe and art/source-output rules are now explicitly documented

Important boundary:

- the packaged smoke lane proves staged boot and authority wiring
- the M43 and M44 acceptance wrappers prove authored milestone completion after the directed walkthrough

That distinction is intentional and must not be weakened again.

## Canonical M45 Title

`M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline`

## Why M45 Exists

M44 proved the first complete directed planet-orbit-planet continuity lane on the packaged authoritative localhost base.

M45 must turn that into the first internally hostable alpha candidate.

This is the milestone where the project stops behaving like an impressive packaged prototype and starts behaving like a product that can be handed to an internal test group with discipline.

## M45 Scope Lock

M45 must deliver all of the following:

1. a first honest hosted deploy path for this repo
2. a remote or non-localhost connect lane that stops treating file-backed localhost as the product truth
3. a packaged internal-alpha run path that is clear enough for repeatable operator use
4. a telemetry baseline that makes connect, disconnect, session, build, host, and restore behavior visible
5. room or interior description runtime for key authored spaces
6. the first product-facing MUD-feel surfaces:
   - room-entry description presentation
   - prompt or vitals direction
   - first typed command-bar shell or MVP entry surface
7. preservation of the current M44 local package lane as fallback validation

## M45 Required Focus

### 1. Hosted session bootstrap

The new lead developer must define and implement:

- a hosted boot path
- a connect target model
- protocol ownership and version stamping
- connection failure visibility
- disconnect visibility
- a clear distinction between localhost fallback and hosted product truth

### 2. Internal alpha package discipline

M45 must produce:

- a package shape that an internal operator can stage and run
- clear launch and smoke scripts for the hosted/internal-alpha lane
- documentation that tells operators exactly which scripts to use

### 3. Telemetry baseline

M45 must add a true baseline, not a hand-wavy promise.

Minimum telemetry surface:

- host identity
- session identity
- client identity
- build identity
- connect event
- disconnect event
- connect failure event
- mission/session lifecycle visibility
- persistence restore visibility

M46 can deepen crash capture and admin tooling.
M45 must at least make the hosted/internal-alpha lane diagnosable.

### 4. MUD-feel product surface

M45 must start proving that this is a graphical MUD and not only a top-down RPG mission slice.

At minimum:

- entering important rooms or interiors must produce authored descriptions
- those descriptions must feel like product UX, not debug text
- a prompt or vitals direction must exist
- a typed command-bar shell or MVP input surface must exist

This does not require full command richness yet.
It does require a real first-class text-entry direction.

### 5. Preservation of the protected base

M45 must not regress:

- M40 combat
- M41 mission flow
- M42 ship runtime
- M43 orbital routing
- M44 docking and return-loop continuity
- packaged localhost smoke validation
- persistence and migration behavior

## M45 Explicit Exclusions

M45 must not widen into:

- full account system
- full authenticated launcher or portal flow
- full moderation stack
- broad multi-user scenario design
- broad accessibility pass
- full social chat system
- full admin tool surface
- full crash-ops discipline
- broad art-complete overhaul

Those belong to later milestones unless a narrow enabling seam is absolutely required.

## M45 Deliverables

Minimum expected deliverables:

- code changes for hosted bootstrap and remote lane ownership
- packaging or deploy scripts for the hosted/internal-alpha lane
- telemetry/logging changes
- room-description runtime support for key spaces
- first typed command-bar or prompt-entry surface
- updated docs
- updated milestone/roadmap truth if scope or sequencing shifts

## M45 Acceptance Expectations

M45 should not be called complete unless all of the following are true:

- the repo still builds cleanly
- the packaged localhost host smoke still passes
- the packaged localhost client/host integration smoke still passes
- the hosted/internal-alpha lane has a repeatable documented boot path
- the client can reach a non-localhost or explicitly hosted target lane
- protocol/version/connect failures are diagnosable
- entering key interiors produces authored description output
- the slice has a visible prompt/vitals direction and a typed command-entry surface
- telemetry artifacts make internal-alpha failures diagnosable without guesswork

## Documents The New Lead Developer Must Review

1. `Docs/Wrath and Ruin - M40-M44 Sign-Off Report.md`
2. `Docs/Wrath and Ruin - M45 Lead Developer Handoff.md`
3. `Docs/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md`
4. `Milestones/WAR_Strategic_Roadmap.md`
5. `Docs/Wrath and Ruin - Master Production Plan to Alpha.md`
6. `Docs/Wrath and Ruin - Milestone Delivery Standard.md`
7. `Docs/Wrath and Ruin - Runtime Boundary Contract.md`
8. `Docs/Wrath and Ruin - GDD.md`
9. `Docs/Feature List.md`
10. `Docs/Wrath and Ruin - Art Production and Asset Planning Track.md`
11. `Docs/Wrath and Ruin - Art Asset Delivery Contract.md`
12. `Docs/Wrath and Ruin - Lead Artist Onboarding Package.md`
13. current milestone docs in `Milestones/`

## Immediate Rule For The New Lead Developer

Do not begin with a long understanding report.

The first useful deliverable is M45 itself.

That means:

- inspect the repo and docs
- lock the exact M45 implementation slice
- then begin implementation in narrow, shippable increments immediately

If there is a blocking conflict in the docs, resolve it in the docs and proceed.
Do not spend the first turn paraphrasing the project back to production.
