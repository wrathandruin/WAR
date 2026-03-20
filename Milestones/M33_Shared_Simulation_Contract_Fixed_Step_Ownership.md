# M33 - Shared Simulation Contract / Fixed-Step Ownership

## Summary

M33 introduces the first explicit shared simulation boundary in WAR.

The purpose of this milestone is not gameplay breadth.
It is to establish where gameplay truth lives, how it updates, and how the client talks to it.

## What Was Added

- `SimulationIntent` as the client-to-simulation request contract
- `SimulationRuntime` as the local shared gameplay runtime
- fixed-step simulation ownership for movement, action processing, and path progression
- presentation interpolation derived from authoritative simulation state
- runtime diagnostics for simulation ownership, cadence, accumulator state, and intent counts
- updated README and architecture documentation for the new boundary

## Ownership After M33

### Client Shell

The client now owns:

- input capture
- camera motion
- renderer invocation
- review hotkeys
- local diagnostics presentation

### Shared Simulation Runtime

The simulation runtime now owns:

- `WorldState`
- authoritative player position
- pending intents
- gameplay action processing
- current path and path index
- event log
- fixed-step update cadence
- movement-target truth

## Fixed-Step Cadence

Initial cadence for this milestone:

- fixed step: **0.05 seconds**
- local authority mode only
- visible diagnostics for accumulator, frame count, tick count, queued intents, processed intents, and last processed sequence

## Why This Exists Before M34

M34 needs to move world ownership outside the client process.
That is safer now because the client has already stopped being the owner of gameplay progression.

## Verification Expectations

Reviewers should confirm:

- local play still works
- diagnostics clearly identify the simulation owner and fixed-step cadence
- clicks now become explicit simulation intents
- `GameLayer` reads as a client shell rather than a gameplay-state owner

## Known Validation Boundary

This milestone package was prepared without a full Windows compile/run in the packaging environment used for repo integration.
Final compile/runtime confirmation must still happen in the real WAR lane.

## Next Milestone

M34 - Headless World Host / Dedicated Server Bootstrap
