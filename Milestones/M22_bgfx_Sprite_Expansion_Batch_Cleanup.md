# Title

**M22 — bgfx Sprite Expansion / Batch Cleanup**

# Description

M22 expands atlas-backed sprite rendering further into the world and tightens the scene submission flow around a cleaner batched structure.

M21 introduced a shared atlas and material descriptors for the first set of textured sprites.
M22 builds directly on that by moving the world tile layer onto the atlas-backed path and consolidating actor rendering into a single textured actor layer.

# Download

This milestone document is included inside the delivery package at:

```text
Milestones/M22_bgfx_Sprite_Expansion_Batch_Cleanup.md
```

# Included

- atlas-backed textured tiles for floor and wall visuals
- expanded sprite material set for tiles plus existing actors
- combined actor sprite layer for entities and player
- cleaner textured world submission structure
- updated atlas asset with floor, wall, player, crate, terminal, and locker regions
- updated README and milestone event messaging
- milestone document included inside the package

# What should be seen visually

The scene should still behave the same in terms of camera control, input, pathing, hovering, and interaction flow.

The intended visible changes in M22 are:

- the world tiles should now render as atlas-backed textured tiles rather than only flat-color placeholders
- walls and floor should have clearer visual separation through sprite materials
- the player and world entities should remain textured
- the overall scene should feel more coherent because both tiles and actors are now coming from the same shared visual source

# Why this is important

M22 matters because it broadens the atlas/material approach from “key actors” to “the scene itself”.

That is important because it:

- pushes the renderer further away from placeholder geometry
- makes the world visually more unified
- tightens the batching story around a cleaner submission structure
- prepares the codebase for broader scene variation and richer content authoring
- turns the atlas/material pipeline into something that affects most of what the player sees

# What should be coming up in the next milestone

**M23 — bgfx Tile Variant / Authoring Prep**

The next milestone should focus on richer scene variation and authoring readiness.

That should include:

- additional floor and wall material variants
- cleaner authoring-friendly expansion of atlas regions
- broader preparation for scene diversity without renderer churn
- continued reduction of visual placeholder behavior

M22 expands atlas-backed scene coverage.
M23 should make that scene easier to vary, author, and grow.
