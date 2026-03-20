# WAR Strategic Roadmap

## Strategic Intent

Wrath and Ruin must transition from a renderer-forward local prototype into a server-authoritative alpha product.

The roadmap is intentionally narrow:

- one complete slice
- one planet-to-space loop
- one persistent multiplayer-ready experience

The project should not chase broad sandbox breadth until that slice is real.

---

## Current State

Milestones M1-M16 are complete and prove:

- client runtime exists
- interaction and stateful objects exist
- bgfx world rendering exists
- shader pipeline bootstrap exists

The project still lacks the systems that define alpha:

- authority
- persistence
- combat
- inventory and progression
- data-driven content
- ships and space travel
- multiplayer session operations

This means the next roadmap must optimize for architecture and gameplay proof, not more prototype-side feature breadth.

---

## Program North Star

Alpha must deliver a playable loop in which a player can:

- join a server session
- explore a planetary location
- interact, loot, survive, and fight
- modify terrain in limited but meaningful ways
- board a ship
- travel through a local space layer
- dock or land at a second destination
- complete a mission chain
- return to a persistent world state

If a task does not move the project closer to that loop, it is a lower priority than it appears.

---

## Guiding Rules

1. Finish what is half-built before adding new surface area.
2. Build the authoritative server path before expanding combat and progression.
3. Make content data-driven before attempting content volume.
4. Keep the first alpha narrow enough to stabilize.
5. Treat diagnostics, telemetry, and packaging as production work, not polish.
6. Stay Windows-first until alpha is proven.

---

## Phase Stack

## Phase A - Client Runtime Closure

Goal:
Make bgfx the clear runtime path and remove rendering ambiguity.

Includes:

- batching
- projection cleanup
- sprite or texture-capable path
- asset and shader packaging
- client diagnostics

Exit:
The renderer no longer blocks architecture or gameplay work.

---

## Phase B - Server-Authoritative Foundation

Goal:
Move the project from local prototype assumptions into authoritative simulation.

Includes:

- shared-client-server split
- fixed-step ownership
- protocol and serialization
- headless server
- localhost replication
- latency and divergence testing

Exit:
The server owns movement and interactions.

---

## Phase C - Planetary Sandbox Core

Goal:
Prove the on-foot game loop in one planetary location.

Includes:

- persistence
- data-driven content
- actors, stats, inventory, and equipment
- six-second combat
- survival hazards
- NPC encounter logic

Exit:
A player can explore, survive, fight, loot, and persist progress in one authored zone.

---

## Phase D - Space Layer and Vertical Slice

Goal:
Connect the on-foot loop to a playable ship and space layer.

Includes:

- ship runtime
- orbital or local space traversal
- landing and docking transitions
- mission scripting
- progression-lite support
- multiplayer session flow

Exit:
One mission chain can move a player from planet to space to a second destination and back.

---

## Phase E - Alpha Readiness

Goal:
Stabilize the slice and prepare it for repeatable outside testing.

Includes:

- functional UI pass
- performance and soak testing
- telemetry and crash capture
- dedicated server packaging
- content lock and bug burn-down

Exit:
The project can support structured alpha playtests.

---

## Milestone Blocks

## Block 1 - Finish the Client Runtime

- M17: Render data cleanup and batching
- M18: Camera, projection, and sprite-material pass
- M19: Asset packaging, shader automation, and reproducible builds
- M20: Client UX shell and diagnostics

## Block 2 - Build the Real Architecture

- M21: Shared-client-server code split
- M22: Fixed-step simulation and authority boundaries
- M23: Command protocol and serialization
- M24: Headless dedicated server
- M25: Authoritative localhost movement and interactions
- M26: Replication, latency simulation, and divergence diagnostics

## Block 3 - Prove the Planetary Loop

- M27: Save, load, and persistence
- M28: Data-driven locations, prefabs, items, and interactables
- M29: Actor model, stats, inventory, and equipment
- M30: Six-second combat controller
- M31: Survival hazards and terrain consequences
- M32: NPC AI and encounter authoring

## Block 4 - Prove Planets and Space Together

- M33: Ship runtime
- M34: Orbital or local space traversal
- M35: Landing, docking, and transitions
- M36: Mission, dialogue, and terminal framework
- M37: Economy-lite, loot, and progression slice
- M38: Multiplayer session flow and onboarding

## Block 5 - Prepare for Alpha

- M39: Interaction, inventory, combat, and travel UI pass
- M40: Performance and soak-test pass
- M41: Telemetry, crash capture, admin tools, and server packaging
- M42: Alpha content lock and stabilization

---

## First Alpha Scope

### Must Be Present

- one planetary surface location
- one orbital or local space map
- one second destination
- one player ship class
- one six-second combat encounter set
- one mission chain
- persistence
- multiplayer-capable server session
- enough UI to play and debug the slice

### Must Not Expand Before Alpha

- large planetary count
- broad faction campaign content
- deep crafting tree
- large economy simulation
- broad social feature set
- cross-platform release support

---

## Strategic Summary

The correct path is:

1. close renderer and client runtime debt
2. pivot into server authority
3. prove the planetary on-foot loop
4. connect that loop to one space layer
5. harden, instrument, package, and stabilize

This is the shortest credible route to a professional alpha.
