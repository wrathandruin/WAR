# Wrath and Ruin - M29-M48 Lead Developer Milestone Plan

## Purpose

This document is the active milestone runway for the next lead developer after M28.

It remains the correct detailed historical runway for M29 through M48.
However, the post-M44 continuation has now been expanded and refined in:

- `Docs/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md`

It is intentionally designed to break the project out of renderer-only momentum and move it into a production-shaped vertical-slice program.

The schedule uses weekly sprint language as a management shorthand.
AI-assisted execution may compress or expand individual tasks, but the weekly banding remains the planning cadence for production tracking.

## Strategic Direction

The current visual groundwork is enough to support a pivot.

From M29 onward, the project should move through five clear phases:

1. close the current visual-content loop without getting trapped there
2. establish runtime boundaries, packaging discipline, and authoritative ownership
3. build the planetary gameplay core
4. build the ship-and-space vertical slice
5. harden the product for internal alpha, then lay the runway for beta and market readiness

The legacy Wrath-and-Ruin project should be treated as the production reference for:

- runtime boundaries
- deployment and rollback habits
- release runbooks
- account and session flow
- smoke and acceptance discipline
- packaging and updater thinking

Phase 5 should also inherit one additional rule from the legacy product:

- playtesting "fun" is only meaningful if the slice is being evaluated as a graphical MUD session, not only as a better top-down RPG

## Phase Overview

### Phase 1 - Visual Closure And Production Contract

Milestones:

- M29
- M30
- M31
- M32

Goal:
Finish the current art-direction and scene-readability pass while establishing canonical content rules, packaging expectations, and demo-lane discipline.

### Phase 2 - Shared Simulation And Authority

Milestones:

- M33
- M34
- M35
- M36

Goal:
Create the first real client/shared/server contract so the game stops being a local-only prototype and starts becoming an authoritative product.

### Phase 3 - Planetary Gameplay Core

Milestones:

- M37
- M38
- M39
- M40

Goal:
Make the planetary game loop worth deploying by adding persistence, inventory, hazards, and combat.

### Phase 4 - Space Vertical Slice

Milestones:

- M41
- M42
- M43
- M44

Goal:
Connect the planetary loop to ship runtime, orbital travel, second-destination flow, and mission return continuity.

### Phase 5 - Alpha Hardening And Product Runway

Milestones:

- M45
- M46
- M47
- M48

Goal:
Turn the slice into something the team can host, test, demonstrate, support, and grow toward beta and market readiness.

Phase 5 amendment:
This phase must explicitly close the gap between the current localhost mission slice and the actual product fantasy.

That means:

- hosted-session reality, not only package quality
- account/session/updater direction, not only local launch flow
- telemetry, crash, admin, support, and security foundations
- a MUD-identity MVP with text-first context, commands, and social presence strong enough for meaningful playtests
- room and interior descriptions early enough that entering authored spaces already feels text-rich

Parallel production rule:
From M45 onward, engineering and art must be run as explicit parallel workstreams.
The lead developer should not assume missing art will be solved ad hoc, and the lead artist should not be handed unstructured requests without milestone context.

## Milestone Pass

## Title

M29 - bgfx Regional Set Dressing / Encounter Hooks

## Description

M29 should finish the immediate visual-authoring bridge that M27 and M28 started.

The goal is to make semantic regions feel intentionally dressed rather than merely tinted, while introducing encounter-ready authored hooks that later gameplay systems can consume.

This is the last milestone in which rendering is allowed to be the visible center of gravity.
After this, the renderer should support the product roadmap rather than define it.

## Download

Planned package name:

`WAR_M29_code_package.zip`

The milestone document should live at:

`Milestones/M29_bgfx_Regional_Set_Dressing_Encounter_Hooks.md`

## Included

Planned focus areas:

- expanded semantic prop dressing rules
- encounter-marker and authored hotspot hooks on regions
- stable bgfx and GDI presentation for region-aware dressing
- milestone messaging that clearly closes the renderer-first chapter

## What should be seen visually

- authored spaces should look more deliberately differentiated
- semantically important locations should be easier to identify at a glance
- encounter-capable spaces should feel like real gameplay anchor points

## Why this is important

M29 closes the current rendering-content loop cleanly and creates authored anchor points for gameplay systems without broadening the feature set irresponsibly.

## What should be coming up in the next milestone

M30 - Playable Slice Readability / Interaction Affordances

## Title

M30 - Playable Slice Readability / Interaction Affordances

## Description

M30 should improve scene readability and interaction clarity so the current prototype stops feeling like an internal rendering test and starts feeling like a controlled playable slice.

The focus should be on hover, selection, inspect, use-state, movement readability, interaction prompts, and slice legibility rather than on adding many new mechanics.

## Download

Planned package name:

`WAR_M30_code_package.zip`

The milestone document should live at:

`Milestones/M30_Playable_Slice_Readability_Interaction_Affordances.md`

## Included

Planned focus areas:

- interaction affordance pass
- clearer inspect and use feedback
- path, hover, and state readability improvements
- lightweight diagnostics that support demo presentation

## What should be seen visually

- a new viewer should understand what is interactable without explanation
- movement, hover, and inspect states should read cleanly
- the prototype should be easier to demo in a short session

## Why this is important

M30 gives the current slice the minimum readability required for demos, usability review, and future gameplay feature integration.

## What should be coming up in the next milestone

M31 - Canonical Content Contract / Runtime Boundary Cleanup

## Title

M31 - Canonical Content Contract / Runtime Boundary Cleanup

## Description

M31 should formalize what is source-controlled truth and what is runtime-only mutable state inside the new repo.

This milestone should take direct inspiration from the legacy project's runtime boundary work so the new project does not repeat source-versus-runtime drift problems.

## Download

Planned package name:

`WAR_M31_code_package.zip`

The milestone document should live at:

`Milestones/M31_Canonical_Content_Contract_Runtime_Boundary_Cleanup.md`

## Included

Planned focus areas:

- canonical content directory rules
- runtime-only directory rules
- generated versus versioned artifact rules
- asset and package policy updates
- documentation and ignore-rule cleanup

## What should be seen visually

- the repo should be clearly safer to work in
- asset and runtime files should no longer feel ambiguous
- contributors should be able to tell what belongs in source control and what does not

## Why this is important

M31 protects the project from future persistence, deployment, and asset-pipeline chaos.
It is a production milestone, not clerical cleanup.

## What should be coming up in the next milestone

M32 - Local Demo Lane / Packaging / Diagnostics Baseline

## Title

M32 - Local Demo Lane / Packaging / Diagnostics Baseline

## Description

M32 should create a repeatable local demo path that builds, launches, and surfaces enough diagnostics to support disciplined iteration.

This is where the project starts acting like a product lane instead of a manually assembled prototype.

## Download

Planned package name:

`WAR_M32_code_package.zip`

The milestone document should live at:

`Milestones/M32_Local_Demo_Lane_Packaging_Diagnostics_Baseline.md`

## Included

Planned focus areas:

- local packaging baseline
- build and launch checklist
- startup diagnostics and runtime visibility
- smoke-test or scripted verification starter lane

## What should be seen visually

- a reproducible local build should launch cleanly
- diagnostics should make startup state and failures understandable
- demo preparation should become repeatable instead of ad hoc

## Why this is important

M32 gives the next phase a safer base.
Authoritative networking, persistence, and deployment work are much cheaper when local demo and diagnostics discipline already exist.

## What should be coming up in the next milestone

M33 - Shared Simulation Contract / Fixed-Step Ownership

## Title

M33 - Shared Simulation Contract / Fixed-Step Ownership

## Description

M33 should define the shared gameplay contract between client and future server runtime.

The immediate goal is not feature breadth.
The goal is to establish simulation ownership, update cadence, state boundaries, and intent-processing rules.

## Download

Planned package name:

`WAR_M33_code_package.zip`

The milestone document should live at:

`Milestones/M33_Shared_Simulation_Contract_Fixed_Step_Ownership.md`

## Included

Planned focus areas:

- shared simulation module boundary
- fixed-step or controlled-step simulation ownership
- authoritative action-intent contract
- clear client-side presentation versus simulation responsibilities

## What should be seen visually

- local gameplay should still behave correctly
- diagnostic output should make simulation ownership visible
- the codebase should clearly show where shared rules now live

## Why this is important

M33 is the foundation for every serious multiplayer, persistence, and gameplay milestone that follows.

## What should be coming up in the next milestone

M34 - Headless World Host / Dedicated Server Bootstrap

## Title

M34 - Headless World Host / Dedicated Server Bootstrap

## Description

M34 should stand up the first headless authoritative host for the new game loop.

The milestone does not need a fully featured server stack yet, but it must prove that world ownership can live outside the client process.

## Download

Planned package name:

`WAR_M34_code_package.zip`

The milestone document should live at:

`Milestones/M34_Headless_World_Host_Dedicated_Server_Bootstrap.md`

## Included

Planned focus areas:

- headless world-host executable or mode
- basic boot flow
- local connection lane
- server-start diagnostics and configuration surface

## What should be seen visually

- the project should be able to boot a separate host process
- the client should connect to that host in a controlled local scenario
- failures should be diagnosable instead of silent

## Why this is important

M34 is the milestone where the project stops being architecture theatre and begins proving authority in runtime form.

## What should be coming up in the next milestone

M35 - Client Intent Protocol / Authoritative Movement And Interaction

## Title

M35 - Client Intent Protocol / Authoritative Movement And Interaction

## Description

M35 should move movement and interaction truth into the host by way of a clean intent and response contract.

The client should request actions.
The host should validate and resolve them.

## Download

Planned package name:

`WAR_M35_code_package.zip`

The milestone document should live at:

`Milestones/M35_Client_Intent_Protocol_Authoritative_Movement_And_Interaction.md`

## Included

Planned focus areas:

- movement intent messages
- interaction intent messages
- host validation and resolution
- client-side correction handling

## What should be seen visually

- movement and interactions should still feel responsive
- the host should clearly own the truth
- mispredictions or rejected actions should be visible and understandable in diagnostics

## Why this is important

M35 converts authority from a conceptual architecture goal into actual gameplay behavior.

## What should be coming up in the next milestone

M36 - Replication / Latency Harness / Divergence Diagnostics

## Title

M36 - Replication / Latency Harness / Divergence Diagnostics

## Description

M36 should harden the new authoritative lane by adding state replication discipline, local latency simulation, and clear divergence diagnostics.

This is where the team proves that networking problems can be observed and debugged instead of guessed at.

## Download

Planned package name:

`WAR_M36_code_package.zip`

The milestone document should live at:

`Milestones/M36_Replication_Latency_Harness_Divergence_Diagnostics.md`

## Included

Planned focus areas:

- world-state replication basics
- latency and jitter simulation hooks
- divergence logging or overlays
- connection-state diagnostics

## What should be seen visually

- the client should remain coherent under controlled latency
- replicated state should be visible and understandable
- divergence conditions should be observable during testing

## Why this is important

M36 makes future gameplay features safer because it gives the team real visibility into authority and replication behavior.

## What should be coming up in the next milestone

M37 - Persistence Schema / Save-Load / Versioned Migration

## Title

M37 - Persistence Schema / Save-Load / Versioned Migration

## Description

M37 should establish a versioned persistence foundation for the new slice.

The milestone should define what must persist, how it is serialized, how schema changes are handled, and how rollback or migration discipline will work over time.

## Download

Planned package name:

`WAR_M37_code_package.zip`

The milestone document should live at:

`Milestones/M37_Persistence_Schema_Save_Load_Versioned_Migration.md`

## Included

Planned focus areas:

- persistence boundary definition
- save and load path
- schema versioning
- migration discipline and failure handling

## What should be seen visually

- a session should retain meaningful state after restart or reconnect
- persistence behavior should be predictable and documented
- schema changes should feel controlled rather than fragile

## Why this is important

M37 is mandatory for alpha because the target product is not a disposable session toy.
It is a persistent game.

## What should be coming up in the next milestone

M38 - Actor Runtime / Inventory / Equipment / Loot

## Title

M38 - Actor Runtime / Inventory / Equipment / Loot

## Description

M38 should add the first serious actor-state layer to the slice.

Players and world entities need inventory, equipment, loot, and stateful ownership rules that are compatible with persistence and authority.

## Download

Planned package name:

`WAR_M38_code_package.zip`

The milestone document should live at:

`Milestones/M38_Actor_Runtime_Inventory_Equipment_Loot.md`

## Included

Planned focus areas:

- actor-state extension
- inventory container model
- equipment slots or equipment rules
- loot and transfer interactions
- persistence-aware item handling

## What should be seen visually

- items should exist as meaningful gameplay state rather than decoration
- loot and equipment should be visible in play and in diagnostics
- the player loop should start to feel like an RPG rather than a movement prototype

## Why this is important

M38 gives interaction, persistence, and future combat real stakes.

## What should be coming up in the next milestone

M39 - Survival Hazards / Terrain Consequence / World State

## Title

M39 - Survival Hazards / Terrain Consequence / World State

## Description

M39 should introduce environmental pressure to the slice.

The goal is to make the planetary space feel dangerous and systemic through hazards, terrain consequences, and stateful environmental rules.

## Download

Planned package name:

`WAR_M39_code_package.zip`

The milestone document should live at:

`Milestones/M39_Survival_Hazards_Terrain_Consequence_World_State.md`

## Included

Planned focus areas:

- hazard model
- terrain consequence rules
- damage or condition hooks
- interaction between hazards, movement, and persistence

## What should be seen visually

- dangerous spaces should read clearly
- movement and decisions should have consequence
- the slice should gain tension beyond navigation and interaction alone

## Why this is important

M39 creates meaningful environmental gameplay and supports the hard-science-fiction survival tone of the product.

## What should be coming up in the next milestone

M40 - Six-Second Combat / Encounter Resolution

## Title

M40 - Six-Second Combat / Encounter Resolution

## Description

M40 should implement the first complete six-second combat loop for the new game.

This is not the time for deep combat breadth.
It is the time to prove one authoritative encounter loop with readable actions, consequences, and replay value.

## Download

Planned package name:

`WAR_M40_code_package.zip`

The milestone document should live at:

`Milestones/M40_Six_Second_Combat_Encounter_Resolution.md`

## Included

Planned focus areas:

- combat turn cadence or tick ownership
- action resolution
- damage and status handling
- combat messaging and diagnostics
- one complete encounter lane

## What should be seen visually

- at least one full combat encounter should be playable end to end
- the player should understand what happened during the encounter
- combat outcomes should persist correctly

## Why this is important

M40 is one of the central proof points for alpha.
Without a real encounter loop, the slice is not commercially convincing.

## What should be coming up in the next milestone

M41 - Planetary Mission Slice / Objectives / Gate Logic

## Title

M41 - Planetary Mission Slice / Objectives / Gate Logic

## Description

M41 should turn the planetary content into a mission-bearing slice with explicit objectives, gating, and progression through the local environment.

The player should no longer simply wander a prototype map.
They should move through a controlled gameplay path with purpose.

## Download

Planned package name:

`WAR_M41_code_package.zip`

The milestone document should live at:

`Milestones/M41_Planetary_Mission_Slice_Objectives_Gate_Logic.md`

## Included

Planned focus areas:

- objective and mission-state framework
- gated progression on the first destination
- terminal, inspect, or interaction-driven mission beats
- stable mission-state persistence

## What should be seen visually

- the planetary slice should feel like a directed experience
- mission objectives should be understandable
- progression through the first destination should be controlled and testable

## Why this is important

M41 transforms gameplay systems into a true vertical-slice flow rather than a pile of isolated mechanics.

## What should be coming up in the next milestone

M42 - Ship Runtime / Boarding / Ownership State

## Title

M42 - Ship Runtime / Boarding / Ownership State

## Description

M42 should introduce a real ship runtime with boarding and ownership state.

The ship should not be a cutscene excuse.
It should be a world object, a state carrier, and the bridge between planetary and space layers.

## Download

Planned package name:

`WAR_M42_code_package.zip`

The milestone document should live at:

`Milestones/M42_Ship_Runtime_Boarding_Ownership_State.md`

## Included

Planned focus areas:

- ship entity or ship runtime model
- boarding flow
- ownership and occupancy state
- persistence and transition compatibility

## What should be seen visually

- the player should be able to board a ship cleanly
- the ship should feel like a persistent gameplay object
- planetary-to-ship transition should be understandable and stable

## Why this is important

M42 is the first concrete step from the planetary slice into the full planets-and-space promise of the game.

## What should be coming up in the next milestone

M43 - Orbital Space Layer / Travel State / Navigation Rules

## Title

M43 - Orbital Space Layer / Travel State / Navigation Rules

## Description

M43 should establish the local space or orbital travel layer.

The goal is not galaxy-scale breadth.
The goal is one believable local travel layer with readable state, rules, and transition integrity.

## Download

Planned package name:

`WAR_M43_code_package.zip`

The milestone document should live at:

`Milestones/M43_Orbital_Space_Layer_Travel_State_Navigation_Rules.md`

## Included

Planned focus areas:

- local space map or travel-state model
- ship movement rules
- travel hazards or constraints
- navigation feedback and diagnostics

## What should be seen visually

- the player should be able to move through a local space layer
- travel state should be legible
- the project should now prove planet-to-space continuity

## Why this is important

M43 is a core part of the market promise.
Without it, the product is only a planetary prototype with a ship theme.

## What should be coming up in the next milestone

M44 - Docking / Landing / Cross-Layer Transition Persistence / Return Loop

## Title

M44 - Docking / Landing / Cross-Layer Transition Persistence / Return Loop

## Description

M44 should connect ship travel to the second destination and back again.

This milestone must prove docking, landing, return-state continuity, and mission progression across layer changes so the full loop stops breaking at travel boundaries.

## Download

Planned package name:

`WAR_M44_code_package.zip`

The milestone document should live at:

`Milestones/M44_Docking_Landing_Cross_Layer_Transition_Persistence_Return_Loop.md`

## Included

Planned focus areas:

- docking and landing logic
- second-destination arrival flow
- return-loop continuity
- mission-state and persistence across transitions

## What should be seen visually

- the player should be able to travel to a second destination and return
- state should survive cross-layer transitions
- the slice should finally demonstrate the intended planet-space-planet loop

## Why this is important

M44 is the milestone where the alpha promise becomes structurally real.

## What should be coming up in the next milestone

M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline

## Title

M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline

## Description

M45 should package the working slice into an internally hostable alpha candidate with the minimum operational visibility required for disciplined testing.

This is the first milestone that should feel like a product handoff to an internal test group rather than a pure dev build.
It is also the first milestone where the slice must begin proving its graphical-MUD identity more explicitly through text presentation, not only through mission logic.

## Download

Planned package name:

`WAR_M45_code_package.zip`

The milestone document should live at:

`Milestones/M45_Internal_Alpha_Package_Hosted_Deploy_Telemetry_Baseline.md`

## Included

Planned focus areas:

- internal alpha packaging
- hosted deploy lane
- telemetry baseline
- smoke-test and boot validation
- room and interior description support for key authored spaces
- first typed command-bar direction for the internal playtest MVP
- prompt or vitals-strip direction so testers are not only reading debug output
- engineering/art handoff discipline for the current asset set

## What should be seen visually

- the slice should boot in a hosted environment
- internal testers should be able to connect through a controlled flow
- core session behavior should be visible in telemetry and diagnostics
- entering important interiors or destination spaces should now produce authored descriptive text
- the slice should start to feel like a graphical MUD session rather than only a top-down mission demo

## Why this is important

M45 marks the shift from development milestone to real internal product evaluation.

## What should be coming up in the next milestone

M46 - Trust Boundary / Environment Config / Secrets Baseline

## Title

M46 - Trust Boundary / Environment Config / Secrets Baseline

## Description

M46 should define how the hosted product is configured and trusted.

This milestone should establish the first real environment model for local, staging, and hosted alpha, including how secrets, runtime config, and trust boundaries are owned.

## Download

Planned package name:

`WAR_M46_code_package.zip`

The milestone document should live at:

`Milestones/M46_Trust_Boundary_Environment_Config_Secrets_Baseline.md`

## Included

Planned focus areas:

- trust-boundary document
- environment configuration model
- secrets handling policy
- protocol and runtime config separation
- save-path and runtime-root hygiene in hosted environments

## What should be seen visually

- local, staging, and hosted environments should be clearly distinguishable in diagnostics
- config failures should be explicit instead of ambiguous
- operators should be able to see the runtime ownership model without exposing secrets

## Why this is important

Security and environment mistakes are much cheaper to prevent here than after onboarding real testers.

## What should be coming up in the next milestone

M47 - Account Session Ticket Handoff / Authenticated Entry

## Title

M47 - Account Session Ticket Handoff / Authenticated Entry

## Description

M47 should establish how a real player session enters the hosted runtime.

The client and host should stop behaving like they can be coupled casually.
This milestone should define the first trustworthy session-entry model and reconnect identity direction.

## Download

Planned package name:

`WAR_M47_code_package.zip`

The milestone document should live at:

`Milestones/M47_Account_Session_Ticket_Handoff_Authenticated_Entry.md`

## Included

Planned focus areas:

- account/session ownership direction
- trusted ticket or session handoff model
- authenticated session entry
- reconnect token or resume identity approach
- session failure states and expiry handling

## What should be seen visually

- session entry should feel intentional rather than ad hoc
- a hosted player session should be validated and denied coherently
- reconnect identity should have a believable product-facing seam

## Why this is important

The game cannot become a real hosted product if entry into the world is still undefined.

## What should be coming up in the next milestone

M48 - Structured Telemetry / Crash Capture / Runtime Event Pipeline

## Title

M48 - Structured Telemetry / Crash Capture / Runtime Event Pipeline

## Description

M48 should establish the first serious observability baseline for hosted play.

This milestone should make session, runtime, and crash behavior visible enough that engineering and support are no longer guessing.

## Download

Planned package name:

`WAR_M48_code_package.zip`

The milestone document should live at:

`Milestones/M48_Market_Foundation_Release_Ops_Analytics_Readiness_Gate.md`

## Included

Planned focus areas:

- release-ops and go/no-go criteria
- analytics direction
- support and incident-response baseline
- market-readiness checklist foundation
- leadership judgment on whether the current slice reads as a graphical MUD product and not only as a top-down RPG prototype

## What should be seen visually

- the project should read as a product pipeline, not just a prototype repo
- release readiness should be measurable
- leadership should be able to judge whether the game is moving cleanly toward beta and market

## Why this is important

M48 is the milestone where the project demonstrates that it can be managed toward market, not merely built toward another internal prototype.

## What should be coming up in the next milestone

M49 - Beta Content Scale / Retention Expansion / Release Candidate Discipline
