# WAR Strategic Roadmap

## Strategic Intent

Wrath and Ruin must become a deployable, demoable, server-authoritative vertical-slice product before it becomes a broad sandbox.

The project now has two useful histories:

- the new `WAR` repo proves a cleaner top-down client and roguelike presentation direction
- the legacy `Wrath-and-Ruin` repo proves production operations, runtime discipline, release habits, and deployability lessons

The roadmap must merge those strengths without letting rendering remain the strategic center for too long.

## Current Strategic Judgment

The project has enough rendering groundwork to support the next step.

The correct move now is:

- close the current visual-content loop cleanly
- lock down content and runtime boundaries
- establish a local demo and packaging lane
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
- board a ship
- enter a local space or orbital layer
- travel to a second destination
- complete a short mission chain
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
- ship and space transition
- mission continuity
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

## Active Milestone Runway

The implemented milestone train is treated as current through M28.

The active tactical roadmap for the next lead developer is M29 through M48 and is detailed in:

- `Docs/Wrath and Ruin - M29-M48 Lead Developer Milestone Plan.md`
- `Docs/Wrath and Ruin - Milestone Delivery Standard.md`

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

### Weeks 13-16

- M41 through M44
- complete the mission-bearing planet-to-space loop

### Weeks 17-20

- M45 through M48
- host, harden, instrument, support, and evaluate the slice as a product

## What Must Not Happen

- renderer work continuing as the main strategic branch indefinitely
- broad sandbox expansion before one vertical slice works
- persistence arriving after too many systems depend on it
- deployment and packaging deferred until late-stage panic
- runtime and source-control boundaries remaining ambiguous
- market planning treated as a final-week problem

## Success Conditions

### Foundation Success

- the repo has clear ownership boundaries
- local demos are repeatable
- the packaging and diagnostics base is real

### Alpha Success

- one complete hosted slice exists
- it is deployable, replayable, and supportable
- internal testers can use it without constant engineering intervention

### Beta Readiness Success

- outside testers can be onboarded through a controlled process
- distribution, support, and telemetry direction are defined

### Market Readiness Success

- the game can be packaged, updated, monitored, and presented commercially

## Final Strategic Rule

Run the project as:

- a new gameplay and client foundation
- guided by old operational lessons
- aimed at one excellent slice first
- extended into beta and market only after the base is strong

That is the most attainable path to a real product.
