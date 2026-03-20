# Title

**M23 — bgfx Tile Variant / Authoring Prep**

# Description

M23 expands the atlas/material pipeline with explicit tile variants and centralizes tile visual selection into a dedicated resolver.

M22 brought the tile layer onto the atlas-backed path.
M23 builds on that by introducing multiple floor and wall materials and moving tile material choice out of render-data construction and into `BgfxTileVisuals`, which makes future scene variation easier to extend without touching renderer submission code.

# Download

This milestone document is included inside the delivery package at:

```text
Milestones/M23_bgfx_Tile_Variant_Authoring_Prep.md
```

# Included

- expanded tile material set with multiple floor and wall variants
- new `BgfxTileVisuals` helper for centralized tile visual selection
- updated atlas layout to support tile variants plus actor materials
- updated render-data construction to use the new tile visual resolver
- updated project/build files to include the new source files
- milestone document included inside the package

# What should be seen visually

The scene should still behave the same in terms of controls, pathing, interaction, hovering, zoom, and panning.

The intended visible changes in M23 are:

- floor tiles should show more than one visual treatment instead of reading as one repeated surface
- wall tiles should show multiple visual variants instead of one repeated block type
- the scene should feel less uniform and more authored, even though the underlying map logic is unchanged
- player and entity sprite visuals should remain intact on the atlas path

# Why this is important

M23 matters because it starts separating “how the scene is rendered” from “how scene visuals are selected”.

That is important because it:

- makes the atlas/material path easier to extend
- reduces renderer churn when tile visuals evolve
- prepares the project for broader authoring and theme expansion
- improves visual richness without destabilizing the world-space rendering pipeline
- creates a cleaner stepping stone toward real content-driven scene styling

# What should be coming up in the next milestone

**M24 — bgfx Theme Sets / Authoring Hooks**

The next milestone should focus on turning tile/material variation into something more theme-aware and authorable.

That should include:

- theme-oriented grouping of tile and sprite materials
- clearer authoring hooks for scene styling
- easier expansion of atlas regions without renderer rewrites
- continued movement away from placeholder repetition

M23 introduces tile variants and a cleaner selection layer.
M24 should make that selection layer easier to steer from authored intent.
