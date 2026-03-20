# Wrath and Ruin - Shared Simulation Contract

## Purpose

This document records the first explicit shared simulation boundary introduced in **M33**.

Its purpose is to make gameplay-critical ownership reviewable before the project moves into a headless host in M34.

## Core Rule

The client shell is **not** the owner of gameplay truth.

The client is responsible for:

- input capture
- camera movement
- presentation
- diagnostics and developer-facing overlays

The shared simulation runtime is responsible for:

- `WorldState` ownership
- player authoritative position and tile state
- fixed-step ticking
- pending action-intent ownership
- action translation and processing
- path progression
- event-log ownership

## Initial M33 Contract

### Intent Surface

The client emits `SimulationIntent` values.

Current intent types:

- `MoveToTile`
- `InspectTile`
- `InteractTile`

This is intentionally narrow.
M33 is about contract clarity, not feature breadth.

### Simulation Ownership

`SimulationRuntime` owns:

- `WorldState`
- pending simulation intents
- gameplay `ActionQueue`
- current path and path index
- authoritative player position
- presented/interpolated player position derived from authoritative state
- simulation diagnostics
- event log

### Update Cadence

Simulation runs on a controlled fixed step.

Initial M33 cadence:

- fixed step: **50 ms**
- target use: local shared runtime only
- diagnostics expose accumulator, tick count, queued intent count, processed intent count, and last processed sequence

### Client Boundary

`GameLayer` is reduced to:

- window and device initialization
- runtime and demo diagnostics bootstrapping
- user input capture
- selected-tile and hover presentation
- render calls
- authoring hotkeys and review controls

It should not own gameplay progression directly anymore.

## Why This Contract Exists Before M34

M34 must prove that world ownership can live outside the client process.
That is much safer if the client has already stopped being the direct owner of movement, action processing, and event logging.

M33 therefore creates a **host-ready gameplay seam** without yet requiring a real server bootstrap.

## What This Does Not Yet Claim

M33 does **not** yet provide:

- a headless host process
- network transport
- replication
- authoritative rejection/correction over a network
- persistence

Those arrive in later milestones.

## Immediate Review Questions

When reviewing M33, ask:

1. Is gameplay state now owned by `SimulationRuntime` rather than `GameLayer`?
2. Is the action contract explicit enough to reuse in a later host process?
3. Is the fixed-step cadence visible and diagnosable?
4. Can M34 reuse this runtime instead of inventing a second gameplay path?

If the answer to any of those is no, M33 is incomplete.
