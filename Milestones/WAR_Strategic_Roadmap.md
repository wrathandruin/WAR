# WAR Strategic Roadmap

## Strategic Intent

Wrath and Ruin must become a deployable, demoable, server-authoritative vertical-slice product before it becomes a broad sandbox.

The project now has two useful histories:

- the new `WAR` repo proves a cleaner top-down client and roguelike presentation direction
- the legacy `Wrath-and-Ruin` repo proves production operations, runtime discipline, release habits, and deployability lessons

The roadmap must merge those strengths without letting rendering remain the strategic center for too long.

## Active Production Checkpoint

M40 remains signed off.
The split desktop/server/shared baseline is healthy, the packaged localhost authority lane is stable, M41 established the directed planetary mission slice, M42 established the persistent docked-ship runtime, M43 established the authoritative local-orbit lane, and M44 completed the first directed planet-orbit-planet continuity lane on the packaged engineering base.

The immediate goal is no longer to prove isolated systems.
The immediate goal is to hand the stable M44 slice into M45 and turn it into the first internally hostable alpha candidate.

## Current Strategic Judgment

The project has enough rendering groundwork to support the next step.

The correct move now is:

- close the current visual-content loop cleanly
- lock down content and runtime boundaries
- establish a local demo and packaging lane
- realign the workspace toward explicit desktop/server/shared/contracts ownership
- stand up the authoritative server foundation
- build one complete planet-to-space vertical slice
- harden it into an internal alpha
- prepare the runway for beta and market readiness

## North Star

The first real slice must let a player:

- connect to a hosted session
- explore a planetary location
- move, inspect, loot, and interact
- survive hazards
- complete a six-second combat encounter
- complete a readable mission chain with explicit gate logic
- board a ship
- enter a local space or orbital layer
- travel to a second destination
- return to persistent state

If work does not move the project toward that loop or make it more deployable and demoable, it is lower priority than it first appears.

## Delivery Ladder

### Foundation

Must prove:

- runtime boundaries
- build and packaging discipline
- diagnostics
- local demoability
- client/shared/server ownership rules

### Alpha

Must prove:

- hosted authoritative slice
- persistence
- planetary gameplay loop
- mission-bearing progression
- ship and space transition
- internal deployment and repeatable testing

### Beta

Must prove:

- external onboarding
- build distribution direction
- supportable operations
- stronger UX and accessibility
- broader content reliability

### Market

Must prove:

- release operations discipline
- packaging and update strategy
- analytics and support readiness
- credible onboarding and first-session flow

## Legacy Guide Rails

These lessons from the old project should guide all planning:

1. Server authority is not optional.
2. Runtime drift must be separated from source control.
3. Staging and live expectations should be defined early.
4. Account and session flow should be intentional.
5. Smoke, acceptance, and healthcheck culture should exist before release pressure.
6. Packaging and updater direction should not be left until the end.

## Workspace Direction

The repo should stop evolving as a single-executable prototype tree and move toward a product workspace built around:

- `desktop/`
- `server/`
- `shared/`
- `contracts/`
- `web/` when real product work exists to justify it

This direction is defined in:

- `Docs/Planning/Wrath and Ruin - Workspace Structure Realignment Plan.md`
- `Docs/Planning/Wrath and Ruin - Workspace Realignment Implementation Brief.md`

Execution rule:
- perform the non-destructive split during the M37-M38 window while persistence and actor runtime boundaries are still forming
- avoid mixing the workspace split with a broad gameplay rewrite
- treat the split as production hardening, not cosmetic cleanup

## Active Milestone Runway

The implemented milestone train is treated as current through M28.

The active tactical roadmap for the next lead developer is now M29 through M70 and is detailed in:

- `Docs/Planning/Wrath and Ruin - M29-M48 Lead Developer Milestone Plan.md`
- `Docs/Planning/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md`
- `Docs/Standards/Wrath and Ruin - Milestone Delivery Standard.md`

### Phase 1 - Visual Closure And Production Contract

- M29: bgfx Regional Set Dressing / Encounter Hooks
- M30: Playable Slice Readability / Interaction Affordances
- M31: Canonical Content Contract / Runtime Boundary Cleanup
- M32: Local Demo Lane / Packaging / Diagnostics Baseline

### Phase 2 - Shared Simulation And Authority

- M33: Shared Simulation Contract / Fixed-Step Ownership
- M34: Headless World Host / Dedicated Server Bootstrap
- M35: Client Intent Protocol / Authoritative Movement And Interaction
- M36: Replication / Latency Harness / Divergence Diagnostics

Phase gate note:
M36 was signed off on 2026-03-20 after the Windows build, staged local-demo smoke tests, staged headless-host smoke test, and staged M36 acceptance script all passed.
M37 is unblocked from the M36 authority/replication gate.
The desktop/server/shared split baseline is also validated going into M37: `WAR`, `WARShared`, `WARServer`, and `WARLegacy` all build, the staged local demo package carries separate client and host executables, and the packaged authority acceptance still passes after the split work.

### Phase 3 - Planetary Gameplay Core

- M37: Persistence Schema / Save-Load / Versioned Migration
- M38: Actor Runtime / Inventory / Equipment / Loot
- M39: Survival Hazards / Terrain Consequence / World State
- M40: Six-Second Combat / Encounter Resolution

Phase gate note:
M40 is the signed-off handoff into the production transition.
Authority, persistence, actor runtime, hazards, six-second combat, and the directed M41 mission slice are treated as the protected base that M42 must preserve while adding ship runtime.

### Phase 4 - Space Vertical Slice

- M41: Planetary Mission Slice / Objectives / Gate Logic
- M42: Ship Runtime / Boarding / Ownership State
- M43: Orbital Space Layer / Travel State / Navigation Rules
- M44: Docking / Landing / Cross-Layer Transition Persistence / Return Loop

Phase gate note:
M42 established a persistent docked-ship runtime with boarding and player-command ownership.
M43 carried that state into a narrow authoritative local-orbit lane with readable route state, transfer rules, and persistence continuity.
M44 now uses that audited base to prove docking, landing, cross-layer persistence, and the first return loop without widening into M45 hosted-alpha work prematurely.

Current implementation focus:
M44 must preserve the M41-M43 chain while adding one Dust Frontier docking lane, one second-destination landing lane, one surface relay handoff, and one clean return to Khepri Dock.

Execution note:
M44 must remain narrow.
The correct production move is to complete one tidy return loop and hand a stable slice into M45 packaging, telemetry, and hosted hardening.

### Phase 5A - Hosted Product Foundation

- M45: Internal Alpha Package / Hosted Deploy / Telemetry Baseline
- M46: Trust Boundary / Environment Config / Secrets Baseline
- M47: Account Session Ticket Handoff / Authenticated Entry
- M48: Crash Capture / Failure Bundles / Operator Triage Baseline
- M49: Beta Content Scale / Retention Expansion / Release-Candidate Discipline
- M50: Launcher / Installer / Update Productionization

### Phase 5B - Graphical MUD Session MVP

- M51: Room And Interior Description Runtime / Location Titles
- M52: Prompt Vitals Session HUD / Event Log Partitioning
- M53: Typed Command Bar / Core Command Routing / Help
- M54: Speech Emote And Local Social Text Visibility
- M55: Shared Presence / Same-Space Awareness / Interest Rules
- M56: Reconnect Resume / Session Continuity / Drop Recovery
- M57: Description Mission Journal Data Schema / Authoring Pipeline
- M58: Art Integration Pass 1 / Environment Identity / Text Presentation

### Phase 5C - Alpha Cohort Readiness

- M59: Multi-User Scenario Beat / Cooperation And Communication Validation
- M60: Hosted Staging Acceptance / Soak / Alerting
- M61: Performance Budgets / Hitch Memory Network Diagnostics
- M62: Accessibility / Keybinding / Input Resilience / Readability
- M63: First Session Onboarding / Tutorialization / Expectation Setting
- M64: Support Workflow / Player Reports / Incident Response
- M65: Launcher Installer Update Channel / Build Distribution
- M66: Analytics Funnel / Survey / Feedback Instrumentation
- M67: Content Reliability / Description Coverage / Art Pass 2
- M68: Alpha Cohort Rehearsal / Go-No-Go Gate

### Immediate Post-Playtest Follow-On

- M69: Alpha Cohort 1 Execution / Live Observation / Rapid Triage
- M70: Alpha Stabilization / Roadmap Recut / Beta Gate Reset

Phase 5+ amendment note:
The current objective is no longer "finish four hardening milestones."
The current objective is to move from a strong localhost vertical slice to the first real hosted graphical-MUD alpha cohort without skipping the hidden product work.

Reference audits and planning:

- `Docs/Planning/Wrath and Ruin - M44 Roadmap Audit and Phase 5 Amendments.md`
- `Docs/Art/Wrath and Ruin - Art Production and Asset Planning Track.md`
- `Docs/Planning/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md`

## Nominal Weekly Sprint View

These weekly bands are planning shorthand.
AI-assisted execution may compress or expand individual tasks, but the cadence remains useful for production tracking.

### Weeks 1-4

- M29 through M32
- close the renderer-era content pass
- establish content, runtime, packaging, and demo rules

### Weeks 5-8

- M33 through M36
- stand up shared simulation, authority, and replication visibility

### Weeks 9-12

- M37 through M40
- add persistence, inventory, hazards, and combat
- complete the non-destructive workspace split before the repo hardens around the wrong ownership model

### Weeks 13-16

- M41 through M44
- complete the mission-bearing planet-to-space loop

### Weeks 17-22

- M45 through M50
- replace localhost-only assumptions with a hosted product foundation

### Weeks 23-30

- M51 through M58
- add the first true graphical-MUD session identity to the slice

### Weeks 31-40

- M59 through M68
- prepare, rehearse, and gate the first real alpha cohort

### Weeks 41-44

- M69 through M70
- run the first cohort and recut the roadmap from evidence

## What Must Not Happen

- renderer work continuing as the main strategic branch indefinitely
- broad sandbox expansion before one vertical slice works
- persistence arriving after too many systems depend on it
- deployment and packaging deferred until late-stage panic
- runtime and source-control boundaries remaining ambiguous
- market planning treated as a final-week problem
- M41 broadening into ship runtime before the planetary mission slice is actually complete
- Phase 5 fun testing being run as a solo top-down RPG evaluation instead of a graphical-MUD session evaluation
- real hosted transport, security, identity, and ops questions being deferred until after alpha claims are already being made
- room-entry text and authored space description being left so late that the first playtests still feel text-thin
- the first alpha cohort launching before reconnect, recovery, support, and build-distribution discipline exist
- the art lane being treated as a passive dependency instead of an active milestone workstream

## Success Conditions

### Foundation Success

- the repo has clear ownership boundaries
- local demos are repeatable
- the packaging and diagnostics base is real

### Alpha Success

- one complete hosted slice exists
- it is deployable, replayable, and supportable
- internal testers can use it without constant engineering intervention
- the slice preserves enough text, command, social, and descriptive identity to be judged as Wrath and Ruin rather than as a generic tactics prototype
- entering important interiors, stations, and destination spaces produces authored descriptive text instead of silent traversal

### Beta Readiness Success

- outside testers can be onboarded through a controlled process
- distribution, support, and telemetry direction are defined
- hosted-session trust, security, and moderation direction are explicit enough that broader testing is responsible

### Alpha Cohort Success

- the team can host, operate, observe, and support a real cohort
- the slice reads as a graphical MUD, not only as a top-down mission prototype
- room descriptions, commands, prompt/vitals, and social presence are part of the tested experience

### Market Readiness Success

- the game can be packaged, updated, monitored, and presented commercially

## Final Strategic Rule

Run the project as:

- a new gameplay and client foundation
- guided by old operational lessons
- aimed at one excellent slice first
- extended into beta and market only after the base is strong

That is the most attainable path to a real product.
