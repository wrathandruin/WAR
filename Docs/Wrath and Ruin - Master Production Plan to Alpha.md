# Wrath and Ruin - Master Production Plan to Alpha

## 1. Executive Summary

Wrath and Ruin is a top-down hard-science-fiction RPG roguelike sandbox built around persistent world simulation, text-supported roleplay, consequential combat, editable terrain, and multiplayer continuity.

This document defines the delivery path from the current milestone state to a disciplined first alpha.

The project has already completed Milestones M1-M16, which prove:

- application shell and runtime loop
- camera, movement, interaction, and world-state prototype flow
- stateful world objects
- renderer abstraction
- bgfx world rendering and shader pipeline bootstrap

The project has not yet proven the systems that actually define alpha:

- server authority
- persistence
- six-second combat
- inventory and progression
- ship runtime
- planetary and space layer transition
- data-driven content
- multiplayer playtest operations

The central production decision is therefore simple:

**Do not scale breadth. Finish one complete playable slice that includes both planets and space, and build the architecture required to support it cleanly.**

Program planning baseline:

- 32 weekly sprints from the current M16 state
- Windows-first alpha target
- one complete playable slice rather than broad feature spread
- multiplayer-capable, server-hosted internal alpha

AI-assisted development may compress implementation time, but planning, integration, review, testing, and milestone sign-off should still follow a weekly operating cadence.

---

## 2. Alpha Product Definition

### 2.1 Alpha Promise

At alpha, a player should be able to:

- join a server-hosted session
- explore a planetary surface location
- interact with world objects, containers, terminals, and hazards
- survive oxygen and environmental pressure
- fight through at least one full six-second combat encounter
- modify terrain through flattening and horizontal digging
- board a ship, transition into a space or orbital layer, and travel to another destination
- dock or land at a second destination
- complete a short mission chain with persistent consequences
- log out and return to a world that has remembered meaningful state

### 2.2 Alpha Included Scope

- one planetary surface biome with authored points of interest
- one orbital or local space traversal layer
- one second destination such as a station, wreck, moon outpost, or asteroid site
- one player ship class
- one mission chain spanning surface and space
- one six-second combat loop
- basic inventory, equipment, loot, and containers
- terrain flattening and horizontal digging
- limited survival model appropriate to the slice
- persistent world state
- 4-8 concurrent internal players, with a stretch target of 12
- functional UI sufficient to play, test, and debug the slice

### 2.3 Alpha Explicitly Excluded

- full solar-system breadth
- broad faction campaign coverage
- large-scale economy simulation
- advanced crafting breadth
- deep social systems beyond basic speech and party-style coordination
- large content volume
- web or cross-platform release
- full live-service backend stack

### 2.4 Alpha Success Criteria

Alpha is successful when the team can repeatedly run:

- one dedicated server
- multiple clients in the same persistent session
- one complete planet-to-space-to-destination gameplay loop
- one stable combat encounter with consequences
- one reproducible onboarding flow for new testers

---

## 3. Current State Review

### 3.1 Confirmed Completed Through M1-M16

- Win32 client runtime and application loop
- camera pan and zoom
- tile-based world representation
- A* pathfinding and click-to-move
- action queue and intent flow
- inspect and interact verbs
- lightweight entity model and stateful interactables
- render and simulation separation
- backend rendering abstraction
- bgfx initialization path
- bgfx frame path
- bgfx tile, entity, player, path, and hover rendering
- repo-owned shader source and runtime shader loading

### 3.2 Major Gaps Between Current State and Alpha

- no shared simulation module
- no authoritative server executable
- no real replication model
- no save or load pipeline
- no content pipeline for maps, prefabs, items, or missions
- no inventory or equipment runtime
- no formal actor or stat model
- no six-second combat controller
- no survival loop worthy of alpha
- no ship model, travel layer, or transition flow
- no telemetry, soak testing, or server operations readiness

### 3.3 Planning Implication

The next program phase must stop optimizing the local prototype and instead transition the project into a server-authoritative vertical slice effort. Renderer completion matters, but only to remove client-facing blockers. It cannot remain the main workstream.

---

## 4. Delivery Principles

### 4.1 Product Principles

- Build one complete slice before building a broad sandbox.
- Preserve the hard-science-fiction identity in all gameplay decisions.
- Protect the top-down readability and MUD-flavored feedback loop.
- Keep planets and space in alpha, but keep the number of playable locations intentionally small.

### 4.2 Technical Principles

- Server authority begins before combat, persistence, and progression scale up.
- Gameplay systems should become data-driven before content volume expands.
- The trunk must remain playable and debuggable every week.
- Windows-first is acceptable for alpha. Cross-platform readiness is a design constraint, not an immediate delivery requirement.
- New architecture must reduce risk for replication, save versioning, and mission scripting.

### 4.3 Scope Principles

- Cut breadth before cutting stability.
- Do not build “placeholder forever” systems that will be thrown away during the server pivot.
- Do not add a second major biome, faction arc, or ship class until the first one is proven.
- Do not let the space layer become a separate game. It must support the same alpha loop, not replace it.

---

## 5. Program Workstreams and What We Must Consider

### 5.1 Game Direction and Design

- alpha pillars and player fantasy
- onboarding clarity
- mission arc and progression pacing
- risk of feature spread versus slice integrity
- planet-to-space loop cohesion
- systemic consistency with hard-sci-fi constraints

### 5.2 Client Runtime and Rendering

- bgfx as the default runtime path
- camera and projection correctness
- sprite and texture-capable rendering
- UI rendering and interaction prompts
- asset packaging and shader automation
- debug overlays, diagnostics, and asset validation

### 5.3 Shared Simulation and Networking

- fixed-step simulation ownership
- client, shared, and server code boundaries
- command protocol and serialization
- server authority and validation
- replication strategy and divergence detection
- local prediction only where it meaningfully improves feel

### 5.4 Core Gameplay Systems

- actor state and stats
- six-second combat declaration and resolution
- inventory, equipment, and containers
- terrain modification and pathfinding consequence
- survival pressures such as oxygen and radiation
- ship ownership, travel, and destination state

### 5.5 World, Content, and Tooling

- map and location data
- chunk or region persistence
- prefabs and archetypes
- item, loot, and NPC definitions
- mission graphs and terminal scripts
- content validation tools and reproducible build packaging

### 5.6 Persistence and Operations

- world save format
- versioning and migration strategy
- dedicated server packaging
- admin commands and diagnostics
- backups and session recovery
- crash capture and telemetry

### 5.7 UX, UI, and Accessibility

- navigation readability
- combat readability
- inventory and equipment usability
- ship travel view clarity
- mission tracking
- text console integration without overwhelming the player

### 5.8 QA, Performance, and Production Discipline

- weekly smoke tests
- milestone acceptance criteria
- playtest issue triage
- memory and frame-time budgets
- replication soak tests
- document maintenance and decision logging

### 5.9 Art and Audio Readiness

- surface and space visual differentiation
- silhouette clarity for actors and interactables
- readable terrain states
- combat and hazard feedback
- ambient sound support for mood and legibility
- content budgets appropriate to alpha

---

## 6. Phase Plan

## Phase 1 - Renderer Completion and Client Runtime Closure

Duration: Weeks 1-4

### Objectives

- remove renderer ambiguity
- make bgfx the normal runtime backend
- finish basic sprite-capable client rendering
- make assets, shaders, and debug instrumentation production-safe

### Deliverables

- batched render submission path
- camera and projection cleanup
- texture and sprite-capable pipeline
- shader compilation and packaging workflow
- UI shell and runtime diagnostics

### Exit Criteria

- bgfx is the expected runtime backend
- sprites or textured quads are supported
- build artifacts package shaders and runtime assets reliably
- debug overlays identify asset, render, and pipeline failures cleanly

### Primary Risks

- spending too long polishing visuals instead of finishing runtime readiness
- carrying GDI as a hidden second production path

---

## Phase 2 - Server-Authoritative Foundation

Duration: Weeks 5-10

### Objectives

- split local prototype code into client, shared, and server responsibilities
- establish authority boundaries before combat and persistence scale up
- prove a localhost authoritative gameplay loop

### Deliverables

- shared simulation module
- headless dedicated server
- protocol and serialization layer
- authoritative movement and interaction flow
- replication harness with latency and loss simulation

### Exit Criteria

- client connects to headless server
- movement and interaction resolve through server authority
- replicated state drives presentation
- packet loss and latency test tools exist

### Primary Risks

- attempting to preserve local-only assumptions
- overbuilding networking technology before the slice needs it

---

## Phase 3 - Planetary Sandbox Core

Duration: Weeks 11-18

### Objectives

- make the on-foot slice real
- add persistence, inventory, actors, combat, hazards, and AI
- move from prototype interactions to an actual game loop

### Deliverables

- save and load pipeline
- map, prefab, and item data loading
- actor stats and equipment runtime
- six-second combat controller
- survival hazards
- NPC AI and encounter scripting support

### Exit Criteria

- player can explore, loot, fight, survive, and persist progress in one planetary zone
- the zone is data-driven, not hardcoded
- one meaningful encounter can be authored and replayed

### Primary Risks

- designing too much combat depth before the controller is stable
- delaying persistence until too many systems depend on it

---

## Phase 4 - Space Layer and Vertical Slice Completion

Duration: Weeks 19-26

### Objectives

- connect the planetary game to a playable space layer
- make ship travel part of the same persistent experience
- complete the first end-to-end mission slice

### Deliverables

- player ship runtime
- orbital or local space traversal layer
- docking, landing, and transition flow
- mission, dialogue, and terminal framework
- progression and economy-lite support
- multiplayer session and onboarding flow

### Exit Criteria

- one player can move from planet to ship to space to second destination and back
- one mission chain spans both planetary and space gameplay
- multiple players can perform the slice in the same session

### Primary Risks

- turning travel into a disconnected minigame
- underestimating transition-state complexity between layers

---

## Phase 5 - Alpha Hardening and External Playtest Readiness

Duration: Weeks 27-32

### Objectives

- make the slice usable, testable, and supportable
- finish the minimum UI and diagnostics required for external alpha-style testing
- lock scope and stabilize

### Deliverables

- polished functional UI pass
- performance and memory budget pass
- telemetry, crash capture, and admin tooling
- dedicated server packaging and deployment workflow
- alpha balance, bug burn-down, and content lock

### Exit Criteria

- the slice is stable for repeated multi-hour internal sessions
- build and server deployment are reproducible
- alpha test documentation and acceptance checklist exist

### Primary Risks

- opening scope during stabilization
- discovering late that ops, telemetry, or onboarding are missing

---

## 7. Milestone Pass

The completed milestone history ends at M16. The alpha roadmap continues from M17 forward.

## Block A - Client Runtime Completion

- M17: Render data cleanup and batching
- M18: Camera, projection, and sprite-material pass
- M19: Asset packaging, shader automation, and build reproducibility
- M20: Client UX shell, diagnostics, and debug tooling

## Block B - Authority and Architecture

- M21: Shared-client-server code split
- M22: Fixed-step simulation and authority boundaries
- M23: Command protocol and serialization
- M24: Headless dedicated server runtime
- M25: Localhost authoritative movement and interactions
- M26: Replication, latency simulation, and divergence diagnostics

## Block C - Planetary Sandbox Core

- M27: Save, load, and persistent world state
- M28: Data-driven maps, prefabs, items, and interactables
- M29: Actor model, stats, inventory, and equipment foundation
- M30: Six-second combat controller and action resolution
- M31: Survival hazards, terrain consequences, and environmental state
- M32: NPC AI, faction reactions, and authored encounters

## Block D - Space Layer and Mission Slice

- M33: Ship runtime and player vessel state
- M34: Orbital or local space traversal layer
- M35: Landing, docking, and cross-layer transition flow
- M36: Mission scripting, dialogue, and terminal framework
- M37: Economy-lite, loot, and progression slice
- M38: Multiplayer session flow and onboarding

## Block E - Alpha Readiness

- M39: Interaction, inventory, combat, and travel UI pass
- M40: Performance, memory, and soak-test pass
- M41: Telemetry, crash capture, admin tools, and server packaging
- M42: Alpha content lock, stabilization, and sign-off

---

## 8. Weekly Sprint Plan

Every sprint week must end with:

- a green build
- updated documentation
- a runnable demo or captured evidence
- a risk review
- a clear next-week objective

### Weeks 1-4: Phase 1

- Week 1: Stabilize bgfx as the default runtime backend and close remaining renderer parity issues.
- Week 2: Implement render batching and formalize the client render-data path.
- Week 3: Complete camera and projection cleanup and add sprite or textured-quad support.
- Week 4: Automate shader and asset packaging and finalize client diagnostics gate review.

### Weeks 5-10: Phase 2

- Week 5: Establish client, shared, and server module boundaries.
- Week 6: Move simulation update ownership into a fixed-step shared runtime.
- Week 7: Define command protocol, message schema, and serialization formats.
- Week 8: Boot a headless dedicated server and complete initial client connection flow.
- Week 9: Route movement and interaction through authority on localhost.
- Week 10: Add replication harnesses, latency simulation, and Phase 2 gate review.

### Weeks 11-18: Phase 3

- Week 11: Build the save format, world chunk persistence, and migration strategy.
- Week 12: Implement load flow and server-owned persistent world startup.
- Week 13: Convert maps, items, and interactables to data-driven definitions.
- Week 14: Add actor data, stats, inventory slots, and equipment skeleton.
- Week 15: Implement six-second combat turn ownership and action declaration.
- Week 16: Add damage resolution, cover rules, and combat feedback.
- Week 17: Implement survival hazards, oxygen pressure, and terrain consequence rules.
- Week 18: Add NPC encounter logic and complete the on-foot slice gate review.

### Weeks 19-26: Phase 4

- Week 19: Add player ship state, ship interaction, and ship ownership rules.
- Week 20: Build the orbital or local space traversal layer and movement rules.
- Week 21: Implement landing, docking, embark, and disembark transitions.
- Week 22: Add mission scripting, terminal interactions, and narrative state flow.
- Week 23: Add loot flow, resource exchange, and progression-lite support.
- Week 24: Build multiplayer session flow, reconnect basics, and tester onboarding.
- Week 25: Run the first full planet-to-space-to-destination mission rehearsal.
- Week 26: Close slice blockers and complete the vertical-slice gate review.

### Weeks 27-32: Phase 5

- Week 27: Complete HUD and interaction UI pass.
- Week 28: Complete inventory, combat, mission, and travel interface pass.
- Week 29: Profile client and server performance and enforce budget targets.
- Week 30: Add telemetry, crash capture, admin commands, and soak-test reporting.
- Week 31: Lock content, run bug burn-down, and execute alpha rehearsal sessions.
- Week 32: Produce alpha candidate build, server package, and sign-off materials.

---

## 9. Phase Gates and Acceptance Standards

### Gate 1 - Client Runtime Ready

- bgfx is the standard runtime backend
- client render path is asset-driven and diagnosable
- no critical renderer dependency remains on GDI

### Gate 2 - Authoritative Runtime Proven

- client-server loop works locally
- server owns movement and interactions
- replication and loss testing are operational

### Gate 3 - On-Foot Slice Proven

- player can explore, survive, fight, loot, and persist in one planetary location
- authored encounter content can be shipped through data

### Gate 4 - Planet and Space Slice Proven

- player can transition between surface, ship, and space
- one mission chain spans multiple layers
- multiplayer session supports the slice end to end

### Gate 5 - Alpha Candidate

- repeatable internal playtests complete successfully
- critical crash rate is acceptable
- server deployment and support workflow are documented
- alpha content is locked and signed off

---

## 10. Risk Register

### High Risks

- Renderer work continuing to consume the schedule after runtime closure.
- Server pivot uncovering major assumptions hidden in the local prototype.
- Combat design expanding faster than the team can test and tune it.
- Persistence schema churn causing repeated save breakage.
- Space layer scope growing into a second major game.
- Content remaining hardcoded too long and slowing all future milestones.

### Mitigation Strategy

- enforce milestone exit criteria strictly
- require data-driven content before content expansion
- keep planet and space slice count intentionally narrow
- treat save versioning, telemetry, and diagnostics as milestone work, not polish
- hold weekly integration reviews against alpha scope, not wish-list scope

---

## 11. Alpha Exit Criteria

Wrath and Ruin is ready for alpha when all of the following are true:

- one dedicated server package can be launched by a non-programmer with documented steps
- multiple clients can complete the intended gameplay loop in the same persistent world
- the surface-to-space loop is stable and understandable
- six-second combat is complete enough to test balance meaningfully
- persistence survives server restart and version-controlled content changes
- top-priority crash and data-loss bugs are closed
- the build has sufficient telemetry and logging to support external playtest feedback

---

## 12. Final Direction

The correct path to alpha is not “more prototype features.”

The correct path is:

1. finish the client runtime cleanly
2. pivot into server-authoritative architecture
3. prove one on-foot planetary sandbox loop
4. connect that loop to one space-travel layer
5. stabilize the result like a product, not a prototype

If the team protects that sequence, alpha is achievable without diluting the game’s identity.
