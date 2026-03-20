# WAR Strategic Roadmap

## Strategic Intent

Wrath and Ruin must become a **deployable, demoable, server-authoritative vertical-slice product** before it becomes a broad sandbox.

The project now has two useful histories:

- the new `WAR` repo proves a cleaner client and roguelike presentation direction
- the legacy `Wrath-and-Ruin` repo proves production operations, identity flow, runtime discipline, and deployability lessons

The roadmap must merge those strengths.

---

## Hard Truths

### The New Repo Currently Proves

- local prototype gameplay
- top-down rendering
- bgfx rendering and shader ownership
- first sprite and atlas groundwork

### The New Repo Does Not Yet Prove

- dedicated authoritative server
- persistence
- deploy workflow
- telemetry or crash capture
- installer/update path
- market-facing onboarding

### The Legacy Repo Already Proved

- account and portal flow
- desktop release thinking
- staging/live runbooks
- runtime data boundaries
- smoke, regression, and hotboot culture
- operational discipline

### Directional Conclusion

Renderer work is no longer the strategic center.

The strategic center is now:

- architecture
- deployability
- persistence
- product usability
- a narrow but real vertical slice

---

## North Star

The game must ship a first slice in which a player can:

- join a hosted session
- explore a planetary location
- move, inspect, loot, and survive
- fight through a six-second combat encounter
- modify terrain in a meaningful but limited way
- board a ship
- enter a space layer
- travel to a second destination
- complete a mission chain
- return to a persistent world state

If work does not move the project toward that loop or make it more deployable and demoable, it is lower priority than it appears.

---

## Delivery Ladder

## Foundation

Goal:
Give the new repo the production shape required to scale safely.

Must include:

- runtime data boundaries
- build and packaging discipline
- diagnostics
- deployability planning
- clear client/shared/server contract

## Alpha

Goal:
Prove one complete, hosted, replayable slice that combines planets and space.

Must include:

- authoritative server
- persistence
- six-second combat
- ship and space transition
- mission slice
- internal deploy and playtest flow

## Beta

Goal:
Make the product ready for outside testers.

Must include:

- onboarding
- content reliability
- external support tooling
- accessibility and UX gains
- stronger operations and telemetry

## Market

Goal:
Make the product commercially credible.

Must include:

- installer and update lane
- release management
- analytics
- support readiness
- market-facing onboarding and packaging

---

## Legacy Guide Rails

These lessons from the old project should guide all new planning:

1. Server authority is not optional.
2. Runtime drift must be separated from source control.
3. Staging and live must be runbook-driven.
4. Account and identity flow should be intentional, not bolted on late.
5. Regression and smoke tests are part of shipping, not cleanup.
6. Desktop packaging and updater strategy should be planned before launch crunch.

---

## Milestone Stack

The renderer-focused groundwork is considered current through M21-level work.
The forward roadmap should continue from M22.

## Block 0 - Production Foundation

- M22: Legacy migration assessment and production contract
- M23: Runtime boundaries, asset policy, and canonical content rules
- M24: Local demo lane, reproducible packaging checklist, and observability baseline

## Block 1 - Client Runtime Closure

- M25: bgfx runtime closure and render-path consolidation
- M26: UI shell, interaction HUD, and diagnostics
- M27: asset validation and demo-readiness review

## Block 2 - Shared Simulation And Authority

- M28: shared simulation and fixed-step ownership
- M29: headless dedicated server bootstrap
- M30: command protocol and authoritative movement/interactions
- M31: replication, latency simulation, and divergence diagnostics

## Block 3 - Planetary Core Loop

- M32: persistence foundation and save/load versioning
- M33: content pipeline for zones, prefabs, items, and interactables
- M34: actor model, inventory, and equipment
- M35: six-second combat controller
- M36: survival and environment model
- M37: NPC encounters and planetary slice gate

## Block 4 - Space Vertical Slice

- M38: ship runtime and boarding
- M39: local space or orbital traversal
- M40: landing, docking, and transition flow
- M41: mission, dialogue, terminal, and session flow

## Block 5 - Alpha Readiness

- M42: deployable server package, telemetry, crash capture, and admin tools
- M43: alpha content lock, usability pass, and sign-off

## Block 6 - Beta Readiness

- M44: external onboarding and launcher/update direction
- M45: scalable content authoring and validation pipeline
- M46: progression, economy-lite, and retention pass
- M47: beta ops, moderation, support, and analytics
- M48: accessibility and UX polish
- M49: beta stability and release-gate review

## Block 7 - Market Readiness

- M50: commercial packaging and installer/update lane
- M51: market onboarding and first-session polish
- M52: live-ops tooling and release management
- M53: compliance, support, and storefront readiness
- M54: market candidate and launch rehearsal

---

## What Must Not Happen

- renderer work continuing as the main strategic branch indefinitely
- broad sandbox expansion before one slice works
- persistence arriving after too many systems depend on it
- deployment and packaging deferred until the end
- runtime/source drift polluting the repo
- market planning treated as a final-week concern

---

## Success Conditions By Horizon

### Foundation Success

- the repo has clear ownership boundaries
- demos are repeatable
- the next architecture steps are safer

### Alpha Success

- one complete hosted slice exists
- it is deployable and replayable
- internal testers can use it without constant engineering intervention

### Beta Success

- outside testers can onboard and play repeatedly
- the project is operationally supportable

### Market Success

- the game can be packaged, updated, supported, and presented commercially

---

## Final Strategic Rule

The project should be run as:

- a new gameplay and client foundation
- guided by old operational lessons
- aimed at one excellent slice first
- extended into beta and market only after the base is strong

That is the most attainable path to an actual product.
