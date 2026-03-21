# Wrath and Ruin - Art Audit and Direction Lock

Date: 2026-03-21

## Purpose

This document audits the current art-direction documents against the actual `WAR` codebase and locks the production decisions that must not drift during lead-artist onboarding.

Reviewed documents:

- `Docs/Wrath and Ruin - Art Direction Asset List.md`
- `Docs/Wrath and Ruin - Lead Artist Considerations.md`
- `Docs/Wrath and Ruin - Art Production and Asset Planning Track.md`
- `Docs/ChatGPT Lead Artist Prompt.md`
- `Docs/Wrath and Ruin - GDD.md`
- `Docs/Feature List.md`
- `Docs/Wrath and Ruin - Master Production Plan to Alpha.md`

Reviewed code and runtime constraints:

- `shared/src/world/WorldState.cpp`
- `src/engine/world/WorldGrid.h`
- `shared/src/world/WorldGrid.cpp`
- `src/engine/render/BgfxViewTransform.cpp`
- `src/engine/render/Camera2D.cpp`
- `src/engine/render/BgfxSpriteMaterial.cpp`
- `src/engine/render/BgfxTextureAsset.cpp`
- `src/engine/render/RenderAssetPaths.cpp`

## Executive Call

Yes, the art lane should now be formalized around a locked top-down specification.

Yes, lore from the older `Wrath-and-Ruin` repo should be brought forward and rewritten as an art/theme guide.

No, the current two art documents are not equally safe to hand to a lead artist as-is.

Current source-of-truth ranking after this audit:

1. `Docs/Wrath and Ruin - Lead Artist Onboarding Package.md`
2. `Docs/Wrath and Ruin - Lore and Visual Theme Guide.md`
3. `Docs/Wrath and Ruin - Art Production and Asset Planning Track.md`
4. `Docs/Wrath and Ruin - Lead Artist Considerations.md`
5. `Docs/Wrath and Ruin - Art Direction Asset List.md`

## What Is Strong Already

### `Lead Artist Considerations` is the stronger brief

`Docs/Wrath and Ruin - Lead Artist Considerations.md` already does several important things correctly:

- explicitly states top-down tactical, not isometric diamond
- locks `1 world tile = 48 x 48 px`
- defines exact export sizes for key asset families
- distinguishes footprint sizes from larger character and prop canvases
- includes state-variant expectations
- includes naming convention and definition-of-done guidance

This document is much closer to a production art bible than the raw asset list.

### The project direction is clearly top-down

The game documentation and code both support a top-down reading:

- the GDD defines the project as a top-down hard-science-fiction RPG
- the world is a flat 2D tile grid
- the camera is a 2D camera
- the bgfx projection is orthographic

That means the art direction should support a true tactical top-down read, not an isometric, dimetric, or faux-3D treatment.

### The current slice already has readable authored spaces

The current playable slice is not generic. It already has named, authored spaces that art can target:

- Cargo Bay
- Transit Spine
- MedLab
- Command Deck
- Hazard Containment
- Responder Shuttle Khepri
- Dust Frontier Landing Pad / relay destination

That gives the art lane enough structure to build a real environment identity pass instead of a speculative asset pile.

## Confirmed Technical Facts From The Current Repo

These are locked facts from the current implementation.

### World and camera

- Current world size: `48 x 36` tiles
- Current gameplay tile footprint: `48 x 48` world units
- Default client window: `1600 x 900`
- Approximate visible area at zoom `1.0`: about `33 x 18` tiles
- Current zoom range: `0.25` to `4.0`
- Projection: orthographic 2D projection

Important clarification:

The engine using an orthographic projection is acceptable.
The thing that must be avoided is visual drift into isometric or 3/4 map language.

### Current runtime texture constraints

The current runtime art lane is still limited:

- texture loader only supports uncompressed Windows BMP
- supported BMP bit depths are `24-bit` and `32-bit`
- current atlas path is `assets/textures/world_atlas.bmp`
- current atlas size is `320 x 128`
- current atlas layout is `5 x 2`
- current atlas cell size is `64 x 64`

Current hard-coded atlas slots:

1. `IndustrialFloor`
2. `SterileFloor`
3. `EmergencyFloor`
4. `IndustrialWall`
5. `SterileWall`
6. `EmergencyWall`
7. `Player`
8. `Crate`
9. `Terminal`
10. `Locker`

### Immediate implication

There are two different art lanes now:

1. The current engine-integration lane
   - limited
   - BMP-only
   - placeholder atlas constrained
   - useful for immediate replacement art

2. The production source-art lane
   - should use the final measurement rules
   - should support larger canvases and layered modular work
   - will later be atlased or integrated through a broader pipeline

These two lanes must not be confused.

## Risks And Gaps

### High

- `Docs/Wrath and Ruin - Art Direction Asset List.md` is a useful backlog, but it is not safe as the main artist handoff.
  - It does not lock camera rules.
  - It does not lock export sizes.
  - It does not distinguish source format from engine format.
  - It mixes art, text, audio, and technical-support items into one ownership pool.

- The current documents did not explicitly explain the difference between top-down art language and orthographic rendering.
  - If this remains vague, an artist can still drift into the wrong visual grammar.
  - The correct lock is:
    - overhead tactical top-down read
    - no isometric diamond tiles
    - no 3/4 map facades
    - no perspective convergence

- The current documents did not explain the mismatch between:
  - `48 x 48` gameplay tile footprint
  - `64 x 64` current placeholder atlas cells
  - larger production canvases like `64 x 96` characters and `48 x 96` walls

Without that clarification, the artist could build to the wrong target.

### Medium

- The art docs previously asked for PNG delivery, but the runtime currently loads BMP only.
- The raw asset list includes text content and audio backlog, which should not sit entirely under lead-artist ownership.
- The current art docs were still light on canon lore translation.
  - They described materials and asset families well enough.
  - They did not yet give enough grounded faction, institution, and system identity for a confident visual theme pass.

### Low

- The current placeholder atlas is still tiny and flat relative to the production target.
- That is fine for the current moment, but the onboarding package must say clearly that it is an interim runtime pack, not the final art target.

## Decisions Locked By This Audit

### 1. Camera and viewpoint lock

Wrath and Ruin must use:

- top-down tactical readability
- overhead planar floors and terrain
- slight front-face cheat only where readability truly requires it
- clear tile footprint discipline

Wrath and Ruin must not use:

- isometric diamond layouts
- 3/4 tactics facades
- side-on building fronts
- perspective vanishing lines
- decorative angle cheats that obscure walkability

### 2. Gameplay footprint lock

- one gameplay tile footprint = `48 x 48`
- floors and ground overlays are designed to that footprint
- most single-tile interactables are designed around that footprint
- larger art may use taller or wider canvases, but their occupied gameplay footprint must be documented

### 3. Runtime-format lock

For current integration:

- engine-ready world textures must include BMP delivery
- BMP must be uncompressed Windows BMP
- `24-bit` is acceptable for fully opaque atlas content
- `32-bit` is acceptable where alpha is needed

For production source control and review:

- layered source file required
- PNG review/export required

### 4. Ownership lock

Lead artist owns:

- visual style and asset direction
- readable environment identity
- interactable and prop readability
- UI art language and presentation support
- portrait and faction readability

Lead artist does not solely own:

- gameplay text writing
- mission copy writing
- audio design backlog
- engine-side import tooling

Those must be treated as adjacent cross-discipline tracks.

### 5. Lore lock

The old `Solar Frontier` lore should be treated as the baseline worldbuilding reference for art, then adapted to the current `WAR` slice.

Art must read as:

- human-only hard sci-fi
- inner-Solar-System industry and infrastructure
- scarcity, repair, patchwork survival, and logistics
- institutional factions and corporate infrastructure
- frontier danger without alien fantasy visual drift

## Final Assessment

Use `Docs/Wrath and Ruin - Lead Artist Considerations.md` as the stronger assessor draft.

Use `Docs/Wrath and Ruin - Art Direction Asset List.md` as a broad backlog reference only.

Do not onboard a lead artist from those two docs alone.

Use the onboarding package and lore/theme guide created after this audit as the real handoff surface.
