# WAR — Milestone 33 (Shared Simulation Contract / Fixed-Step Ownership)

> Current development milestone: M33 — Shared Simulation Contract / Fixed-Step Ownership

## Focus
Move gameplay-critical update ownership out of the frame-driven client shell and into a shared local simulation runtime with a fixed-step loop and explicit intent contract.

M32 established a repeatable local demo lane.
M33 uses that safer base to define where gameplay truth lives, how often it updates, and how client input becomes simulation work.

## What this milestone does
- adds `SimulationRuntime` as the first explicit shared-simulation boundary inside the repo
- adds `SimulationIntent` as the client-to-simulation action contract for move, inspect, and interact requests
- moves world state, action processing, path progression, player position ownership, and event-log ownership into the simulation runtime
- updates `GameLayer` so the client shell becomes an input, camera, render, and diagnostics surface rather than the owner of gameplay truth
- runs gameplay updates on a fixed simulation cadence while keeping presentation separate from simulation ownership
- surfaces simulation diagnostics in both the GDI overlay and bgfx status line
- updates milestone and architecture docs so the new boundary is explicit and reviewable

## Shared simulation after M33
The repo should now read more clearly as:

- client: input capture, camera, rendering, local diagnostics
- shared simulation runtime: world state, player state, fixed-step ticking, intent processing, event-log ownership
- future host: next milestone destination for moving that same contract out of process

## Why this matters
M33 is where WAR stops treating gameplay state as an incidental by-product of rendering and starts treating it as an owned runtime.

That matters because M34 through M36 depend on this exact separation:

- M34 needs a headless world-host bootstrap
- M35 needs authoritative intent validation and resolution
- M36 needs replication and divergence visibility

Without M33, those milestones would be architecture theatre instead of grounded runtime work.

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

And the shared sprite atlas at:

```text
assets/textures/world_atlas.bmp
```

## Next Milestone
### M34 — Headless World Host / Dedicated Server Bootstrap
- stand up the first host-owned runtime lane outside the client shell
- reuse the new shared simulation contract instead of inventing a second gameplay path
- prove that authority can live outside the rendering process
