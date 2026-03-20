# Wrath and Ruin - Master Production Plan to Alpha

## 1. Executive Assessment

Wrath and Ruin is no longer a greenfield idea. It now exists across two relevant project histories:

- the new `WAR` repo, which proves a clean Windows-first top-down client prototype with a modern rendering path
- the legacy `Wrath-and-Ruin` monorepo, which proves deployability, operational discipline, account and launcher flow, regression culture, and the realities of authoritative persistent multiplayer

The correct production direction is not to choose one and ignore the other.

The new repo should become the **clean gameplay and client foundation** for the roguelike migration.
The legacy repo should remain the **reference source for production lessons, server authority discipline, data migration strategy, deployment expectations, and commercial reality**.

### Assessment Summary

What the current `WAR` repo proves:

- Win32 runtime shell
- top-down world camera and traversal prototype
- interaction and action queue prototype
- render abstraction and bgfx path
- shader pipeline and first sprite/atlas groundwork through the current renderer work

What it does **not** yet prove:

- authoritative server runtime
- persistence strategy
- deployable multiplayer session flow
- production packaging and update path
- telemetry, crash capture, or operations
- content pipeline suitable for shipping
- a complete planet-to-space gameplay loop

What the legacy repo proves:

- server-authoritative production instincts
- staging/live operational runbooks
- runtime/source separation discipline
- website account identity and ticket handoff
- desktop packaging and updater thinking
- regression and smoke-test culture
- content/data migration awareness

### Core Judgment

The current project direction is valid, but incomplete.

Renderer work was necessary to make the new client real, but renderer work cannot remain the lead workstream.

The next correct move is to turn the current prototype into a **production-shaped vertical-slice program**:

- build a strong shared simulation and dedicated server base
- preserve the MUD soul and persistent multiplayer identity
- inherit operational rigor from the old project
- deliver a narrow, polished, deployable alpha slice
- use alpha as the foundation for beta and market readiness

---

## 2. Product Ladder

The project must now be planned against four product horizons.

### 2.1 Foundation

Foundation is not player-facing breadth. It is the minimum base required to make every later phase cheaper and safer.

Foundation means:

- reproducible builds
- clean runtime data boundaries
- authoritative simulation ownership
- content and asset source-of-truth rules
- dedicated server packaging direction
- diagnostics, crash visibility, and smoke-test discipline

### 2.2 Alpha

Alpha is the first professional proof that the game works as a product, not only as a prototype.

Alpha must be:

- internally deployable
- demoable to external stakeholders
- replayable by a small controlled test cohort
- stable enough to run repeatedly on a real server package

### 2.3 Beta

Beta is where the project becomes externally operable.

Beta must prove:

- wider tester onboarding
- broader content reliability
- stronger UX and accessibility
- supportable build and deployment processes
- better retention and session continuity

### 2.4 Market Candidate

Market readiness does not mean “finished forever.”
It means the product is credible for commercial release or early-access style launch.

That requires:

- reliable packaging and patching
- clear onboarding and first-session UX
- observability and support workflows
- content stability
- business-facing readiness such as release ops, community support, and compliance planning

---

## 3. Product Definition By Horizon

### 3.1 Alpha Definition

At alpha, a player should be able to:

- connect to a hosted authoritative session
- enter a playable planetary location
- move, inspect, loot, and interact
- survive at least one meaningful environmental hazard set
- fight through a complete six-second combat encounter
- modify terrain in limited but meaningful ways
- board a ship
- enter a space or orbital layer
- travel to a second destination
- complete a short mission chain
- leave and return to a persistent world with retained state

### 3.2 Beta Definition

At beta, the game should additionally prove:

- repeatable onboarding for outside testers
- a more complete mission and progression cadence
- stronger UI and accessibility support
- multiple authored encounters and destinations
- more reliable content production and validation
- better operational visibility and support flow

### 3.3 Market Candidate Definition

At market candidate, the project should additionally prove:

- stable installer and update path
- commercial-quality onboarding
- production crash capture and analytics
- release candidate packaging and rollback capability
- community-facing support and moderation readiness
- a product slice strong enough to retain and convert real players

---

## 4. Current State Review

### 4.1 Confirmed Strengths In The New Repo

- top-down click-to-move prototype
- tile and entity world model
- interaction flow
- renderer abstraction
- bgfx integration
- world-space rendering path
- shader and atlas-facing groundwork

### 4.2 Confirmed Strengths In The Legacy Repo

- production-minded server operations
- account and portal flow
- desktop client release runbook
- live/staging deployment model
- mutable runtime separation lessons
- content migration awareness
- regression and hotboot discipline

### 4.3 Critical Gaps In The New Repo

- no shared simulation module
- no headless server executable
- no authoritative protocol
- no persistence surface
- no account, launcher, or update direction yet in this repo
- no deployment scripts or environment model
- no telemetry, crash capture, or admin tools
- no market-facing packaging plan
- workspace structure is still prototype-shaped around one native executable tree

### 4.4 Directional Risk

If the project continues as a renderer-first prototype, it risks becoming visually improved but operationally immature.

That would produce:

- demos that are brittle
- no reliable test lane
- delayed persistence and networking decisions
- a costly future architecture pivot

The renderer is now far enough along that the production base must take priority.

### 4.5 Workspace Shape Risk

The current repository layout still reflects a single-executable prototype:

- `src/engine/`
- `src/game/`
- `src/platform/`
- `src/tools/`

That shape was acceptable while the project was proving rendering, local simulation, and the first localhost authority lane.
It is the wrong long-term shape for:

- separate desktop and server deliverables
- a real shared gameplay/runtime library
- persistence and protocol contracts that outlive one binary
- future account, relay, admin, or web surfaces

The repo should deliberately move toward:

- `desktop/`
- `server/`
- `shared/`
- `contracts/`
- `web/` when justified by real product work

The detailed move map and execution order are defined in:

- `Docs/Wrath and Ruin - Workspace Structure Realignment Plan.md`
- `Docs/Wrath and Ruin - Workspace Realignment Implementation Brief.md`

This is part of the production transition, not cosmetic cleanup.

---

## 5. Legacy Project Lessons We Must Carry Forward

The old project should not be copied wholesale, but it should absolutely inform this roadmap.

### 5.1 Preserve Conceptually

- server authority
- text and roleplay as first-class systems
- account and identity separation from raw game transport
- desktop and web as product surfaces, not hacks
- explicit staging and live environments
- runtime/source separation
- regression and smoke-test gates
- deployment runbooks as real production assets

### 5.2 Redesign For The New Game

- room-based world logic into spatial simulation
- legacy monolith into clearer client/shared/server boundaries
- legacy top-level product/workspace separation into the new repo structure
- flat-file assumptions into intentional content pipelines
- MUD-first navigation into graphical tactical navigation with text support

### 5.3 Do Not Carry Forward Blindly

- monolithic subsystem coupling
- mixed mutable runtime state inside reviewable source areas
- “ops knowledge lives in people’s heads” behavior
- overgrown feature breadth without slice discipline

---

## 6. Non-Negotiable Production Principles

1. The authoritative server must arrive before broad combat, content, or economy expansion.
2. Demoability and deployability are production work, not polish.
3. Runtime data boundaries must be explicit early, not after the repo gets messy.
4. Every major milestone must leave the trunk buildable, runnable, and diagnosable.
5. Alpha is one complete slice, not a diluted feature spread.
6. Beta expands reliability and audience, not just scope.
7. Market planning begins before launch prep, especially for packaging, onboarding, telemetry, and operations.
8. The new repo should inherit production lessons from the old repo even when code is not directly ported.

---

## 7. Major Workstreams

### 7.1 Product And Design

- alpha, beta, and market scope control
- mission chain design
- progression and retention loop design
- demo slice clarity
- MUD identity preservation

### 7.2 Client Runtime

- bgfx default path
- input and camera feel
- UI shell and diagnostics
- asset validation
- gameplay readability

### 7.3 Shared Simulation And Networking

- fixed-step loop
- deterministic state ownership
- command protocol
- serialization
- replication and divergence detection

### 7.4 Core Gameplay

- actors and stats
- inventory and equipment
- six-second combat
- survival and hazards
- terrain editing consequences
- ship and travel logic

### 7.5 Content And Tooling

- location data
- prefab and item definitions
- mission data
- terminal/dialogue scripting
- content validation and import/export

### 7.6 Persistence And Operations

- save/load
- migrations
- runtime boundaries
- dedicated server packaging
- backups and rollback
- staging/live runbooks

### 7.7 Productization

- installer and update path
- launcher/session flow
- onboarding
- telemetry and crash reporting
- support/admin tools
- demo and release candidate packaging

---

## 8. Alpha Delivery Phases

Nominal planning baseline: **36 weekly sprints** from the current renderer-forward state.

This is the recommended planning cadence even if AI-assisted implementation compresses certain tasks.

### Phase 0 - Production Foundation And Migration Contract

Duration: Weeks 1-4

Objectives:

- declare what this repo owns
- define source versus mutable runtime boundaries
- document inherited production lessons from the legacy project
- make the build, packaging, and demo lane reproducible

Deliverables:

- runtime boundary rules
- asset and content source-of-truth rules
- baseline local demo checklist
- build/package checklist
- initial deployability and observability plan

Exit Criteria:

- the repo has a production-safe contract
- runtime drift and asset policy are explicit
- the project can be demoed locally in a repeatable way

### Phase 1 - Client Runtime Closure

Duration: Weeks 5-8

Objectives:

- finish client/runtime ambiguity
- make the renderer a solved dependency, not the main program
- add the minimum UX and diagnostics needed for repeated demos

Deliverables:

- stable bgfx runtime path
- sprite/material pipeline closed enough for slice work
- debug overlays and asset validation
- minimal UI shell for interaction and state visibility

Exit Criteria:

- the client no longer blocks authority work
- rendering failures are diagnosable quickly
- a clean demo lane exists

### Phase 2 - Shared Simulation And Authority

Duration: Weeks 9-16

Objectives:

- establish the real architecture
- move gameplay-critical decisions to the server
- prove authoritative localhost play

Deliverables:

- shared simulation module
- headless dedicated server
- command and replication protocol
- authoritative movement and interaction
- latency and divergence harness
- non-destructive workspace realignment toward explicit desktop/server/shared/contracts boundaries

Exit Criteria:

- the server owns gameplay state
- the client becomes a rendering and intent surface
- localhost multiplayer is real
- the repo no longer reads like a forever single-executable prototype tree

### Phase 3 - Planetary Core Loop

Duration: Weeks 17-24

Objectives:

- make the on-foot game loop real
- prove persistence, combat, hazards, inventory, and authored content

Deliverables:

- save/load pipeline
- zone, item, and prefab data definitions
- actor stats and equipment
- six-second combat controller
- survival model
- terrain consequence and persistence
- NPC encounters

Exit Criteria:

- one planetary zone is replayable and persistent
- the game loop is more than movement and rendering
- one credible stakeholder demo can happen without apology

### Phase 4 - Ship And Space Vertical Slice

Duration: Weeks 25-30

Objectives:

- connect the on-foot game to a ship and space layer
- complete the first true Wrath and Ruin slice

Deliverables:

- player ship runtime
- boarding and embark flow
- local space or orbital traversal
- docking and landing transitions
- mission, terminal, and dialogue framework
- second destination

Exit Criteria:

- the planet-to-space-to-destination loop is complete
- the slice reflects the game’s actual identity
- the product is demoable as a differentiated concept

### Phase 5 - Alpha Hardening

Duration: Weeks 31-36

Objectives:

- make alpha deployable, supportable, and presentable
- stop acting like a prototype

Deliverables:

- UI usability pass
- telemetry and crash capture
- admin and debug tooling
- dedicated server packaging
- deployment and rollback flow
- alpha checklist and sign-off package

Exit Criteria:

- the build can be packaged and hosted repeatably
- testers can be onboarded through a controlled process
- the team can run structured internal alpha sessions

---

## 9. Active Milestone Pass

The implemented milestone train should now be treated as current through **M28**.

The active tactical roadmap for the next lead developer is **M29 through M48** and is detailed in:

- `Docs/Wrath and Ruin - M29-M48 Lead Developer Milestone Plan.md`
- `Docs/Wrath and Ruin - Milestone Delivery Standard.md`
- `Docs/milestones/WAR_Strategic_Roadmap.md`

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

### Phase 3 - Planetary Gameplay Core

- M37: Persistence Schema / Save-Load / Versioned Migration
- M38: Actor Runtime / Inventory / Equipment / Loot
- M39: Survival Hazards / Terrain Consequence / World State
- M40: Six-Second Combat / Encounter Resolution

### Phase 4 - Space Vertical Slice

- M41: Planetary Mission Slice / Objectives / Gate Logic
- M42: Ship Runtime / Boarding / Ownership State
- M43: Orbital Space Layer / Travel State / Navigation Rules
- M44: Docking / Landing / Cross-Layer Transition Persistence / Return Loop

### Phase 5 - Alpha Hardening And Product Runway

- M45: Internal Alpha Package / Hosted Deploy / Telemetry Baseline
- M46: Alpha Hardening / Crash Capture / Admin Tooling
- M47: External Beta Onboarding / Launcher-Update Direction / Support Flow
- M48: Market Foundation / Release Ops / Analytics / Readiness Gate

---

## 10. Nominal Weekly Sprint Bands

These weekly bands are planning shorthand.
AI-assisted execution may compress or expand individual tasks, but the weekly cadence remains useful for production tracking.

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

### Weeks 13-16

- M41 through M44
- complete the mission-bearing planet-to-space loop

### Weeks 17-20

- M45 through M48
- host, harden, instrument, support, and evaluate the slice as a product

---

## 11. Post-M48 Continuation

The next detailed tactical runway should stop at M48 so leadership can assess the product honestly before broadening the plan again.

However, the likely continuation after M48 is:

- M49: beta content scale, retention expansion, and release-candidate discipline
- M50: launcher, installer, and update productionization
- M51: market onboarding and first-session polish
- M52: live-ops tooling, analytics hardening, and release management
- M53: compliance, storefront, support, and policy readiness
- M54: launch rehearsal, market candidate review, and go/no-go sign-off

---

## 12. Horizon Outlook

### Beta Outlook

Beta should focus on:

- external onboarding
- broader content reliability
- build distribution and updater direction
- accessibility and UX improvements
- operational supportability
- stronger telemetry and retention understanding

### Market Readiness Outlook

Market readiness should focus on:

- release-ops discipline
- installer and update maturity
- analytics and support tooling
- onboarding quality
- commercial packaging and credibility

---

## 13. Scope Discipline

These must not expand before alpha:

- large planetary count
- broad faction wars
- deep crafting breadth
- wide ship roster
- heavy economy simulation
- broad social platform features
- cross-platform launch requirements

These are acceptable after alpha or beta proves the base:

- additional destinations
- broader progression breadth
- deeper faction systems
- richer economic simulation
- more advanced construction and base ownership

---

## 14. Final Recommendation

The shortest credible route to market is:

1. inherit production discipline from the old project
2. stop treating the new repo as a renderer sandbox
3. build the authoritative and deployable base early
4. prove one professional alpha slice
5. expand into beta only after the slice is operationally stable
6. prepare market readiness as a continuation of production discipline, not a late scramble

This is the path that preserves ambition while remaining attainable.
