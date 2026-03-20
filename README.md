# WAR — Milestone 30 (Playable Slice Readability / Interaction Affordances)

> Current development milestone: M30 — Playable Slice Readability / Interaction Affordances

## Focus
Turn the current prototype from an internal rendering demonstration into a more legible, more self-explanatory playable slice.

M29 established authored hotspot anchors and stronger regional dressing.
M30 builds on that by improving hover feedback, selection readability, interaction prompts, inspect clarity, path legibility, and short-session demo usability.

## What this milestone does
- adds clearer hover affordances for blocked tiles, walkable tiles, interactables, and authored hotspots
- adds selected-tile and move-target presentation so click intent is easier to follow
- improves path readability with stronger destination emphasis
- improves inspect and interact messaging so runtime feedback reads like slice diagnostics instead of placeholder logging
- improves debug-overlay context with prompt text, selected tile state, move target state, hotspot detail, and demo-facing controls
- keeps authored hotspot overlay, region overlay, and palette controls intact

## Readability improvements after M30
The current test world should now be easier to read in a short demo because:

- hovered tiles communicate whether the next action is move, inspect, or interact
- the last selected tile remains visible as a point of focus
- an active movement destination is visible as a distinct target marker
- inspect text reports region, entity, hotspot, and state more clearly
- interact text names the specific object or authored anchor being used
- debug text better explains what the player is looking at and what input will do

## Demo controls
- `LMB`: move / set movement target
- `RMB`: interact
- `Shift + RMB`: inspect
- `MMB drag`: pan camera
- `Mouse wheel`: zoom
- `O`: toggle region boundary overlay
- `H`: toggle authored hotspot overlay
- `7 / 8 / 9`: Default / Muted / Vivid palette modes

## Why this matters
M30 is intentionally not a mechanics expansion milestone.

Its job is to make the existing slice legible enough that:

- a new viewer can understand it faster
- usability and demo review become more honest
- future mechanics can attach to clearer interaction and feedback patterns
- the repo can move into M31 and M32 with fewer “what is this supposed to mean?” problems

This keeps the roadmap disciplined: readability and affordances now, production contract and packaging discipline next.

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
### M31 — Canonical Content Contract / Runtime Boundary Cleanup
- formalize source-controlled truth versus runtime-only mutable state
- clean up content, asset, and generated-artifact policy boundaries
- reduce future persistence, packaging, and deployment ambiguity
