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

### 4.4 Directional Risk

If the project continues as a renderer-first prototype, it risks becoming visually improved but operationally immature.

That would produce:

- demos that are brittle
- no reliable test lane
- delayed persistence and networking decisions
- a costly future architecture pivot

The renderer is now far enough along that the production base must take priority.

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

Exit Criteria:

- the server owns gameplay state
- the client becomes a rendering and intent surface
- localhost multiplayer is real

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

## 9. Milestone Pass

The renderer era should be treated as groundwork complete through the current M21-level work.
The production roadmap should continue from **M22**.

### Block 0 - Production Foundation

- M22: Legacy migration assessment and production contract
- M23: Runtime data boundaries, asset policy, and canonical content rules
- M24: Local demo flow, diagnostics baseline, and reproducible packaging checklist

### Block 1 - Client Runtime Closure

- M25: bgfx runtime closure and render-path consolidation
- M26: UI shell, interaction HUD, and diagnostics pass
- M27: asset validation, packaging automation, and demo-readiness review

### Block 2 - Authority And Shared Simulation

- M28: shared simulation module and fixed-step ownership
- M29: headless dedicated server bootstrap
- M30: command protocol, serialization, and authoritative movement
- M31: replication harness, latency simulation, and divergence diagnostics

### Block 3 - Planetary Core Loop

- M32: persistence foundation and save/load versioning
- M33: zone, prefab, item, and interactable content pipeline
- M34: actor model, stats, inventory, and equipment
- M35: six-second combat controller and action resolution
- M36: survival hazards, terrain consequences, and environmental state
- M37: NPC encounters, mission hooks, and planetary slice gate

### Block 4 - Space Vertical Slice

- M38: ship runtime, boarding, and ownership state
- M39: local space or orbital traversal
- M40: landing, docking, and cross-layer transitions
- M41: mission, dialogue, terminal, and session flow

### Block 5 - Alpha Readiness

- M42: deployable server package, telemetry, crash capture, and admin tools
- M43: alpha content lock, usability pass, and internal alpha sign-off

---

## 10. Nominal Weekly Sprint Bands

### Weeks 1-4

- finalize the migration contract
- document runtime boundaries and production principles
- define demo and deploy expectations

### Weeks 5-8

- close renderer/runtime ambiguity
- add diagnostics and demo UX

### Weeks 9-16

- stand up shared simulation and dedicated server
- prove authoritative localhost sessions

### Weeks 17-24

- build the planetary gameplay loop
- add persistence, combat, hazards, inventory, and authored encounters

### Weeks 25-30

- connect ship and space traversal
- complete the first real planet-to-space mission loop

### Weeks 31-36

- harden alpha
- package, instrument, deploy, and rehearse

---

## 11. Beta Outlook

Nominal planning baseline: **18 weekly sprints after alpha**

Beta is where the project becomes ready for sustained outside testing.

### Beta Priorities

- broader onboarding and account/session flow
- better UX and accessibility
- stronger content production velocity
- more encounter and mission breadth
- stability under repeated external testing
- moderation, support, and issue triage flows

### Recommended Beta Milestones

- M44: external tester onboarding, account/session contract, and launcher/update direction
- M45: content authoring and validation pipeline expansion
- M46: progression, economy-lite, and retention pass
- M47: external beta telemetry, moderation, and support operations
- M48: accessibility, UX polish, and tutorialization
- M49: beta stability, content broadening, and release-gate review

### Beta Exit

Beta is successful when:

- outside testers can onboard without developer handholding
- content can be produced and validated at a sustainable rate
- deployment, monitoring, and rollback are boring
- the game retains players beyond the first session

---

## 12. Market Readiness Outlook

Nominal planning baseline: **12 weekly sprints after beta**

Market readiness is about commercial credibility, not just feature count.

### Market Priorities

- installer, updater, and patch strategy
- first-session polish and onboarding
- release ops and rollback
- analytics and support tooling
- community and moderation readiness
- legal, privacy, and compliance review for the intended release platform

### Recommended Market Milestones

- M50: commercial packaging, installer, and update lane
- M51: launch onboarding, new-user flow, and retention pass
- M52: live-ops tooling, analytics, and release management
- M53: compliance, policy, store, and support readiness
- M54: market candidate build, release rehearsal, and go/no-go review

### Market Exit

Market readiness is achieved when:

- the team can package and ship repeatably
- support and incident response are defined
- onboarding converts new users effectively
- the product slice is commercially coherent

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
