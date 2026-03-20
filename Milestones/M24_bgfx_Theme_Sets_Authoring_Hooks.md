# Title

**M24 — bgfx Theme Sets / Authoring Hooks**

# Description

M24 groups tile visuals into explicit scene themes and adds lightweight runtime hooks for switching them.

M23 introduced tile variants and centralized tile visual selection.
M24 builds directly on that by introducing theme-oriented material sets and storing the active theme in `WorldState`, so scene styling can change through authored intent instead of renderer rewrites.

# Download

This milestone document is included inside the delivery package at:

```text
Milestones/M24_bgfx_Theme_Sets_Authoring_Hooks.md
```

# Included

- new `BgfxWorldTheme` theme/material grouping layer
- `WorldState` support for storing the active visual theme
- updated `BgfxTileVisuals` to resolve materials and tints through the active theme
- runtime theme hotkeys in `GameLayer`
- updated atlas layout for Industrial, Sterile, and Emergency tile themes
- milestone document included inside the package

# What should be seen visually

The scene should still behave the same in terms of movement, camera control, panning, zoom, hovering, and interaction.

The intended visible changes in M24 are:

- the world should now support distinct theme looks for tiles
- pressing **1** should switch to an Industrial theme
- pressing **2** should switch to a Sterile theme
- pressing **3** should switch to an Emergency theme
- actors should remain intact while the tile styling shifts around them

# Why this is important

M24 matters because it turns tile visuals from static variation into something that can be steered.

That is important because it:

- makes scene styling easier to author
- reduces renderer churn when visual direction changes
- provides a clear bridge toward map-driven or region-driven visual themes
- improves iteration speed for content-facing rendering decisions
- keeps the renderer stable while making visuals more flexible

# What should be coming up in the next milestone

**M25 — bgfx Content Tagging / Region Hooks**

The next milestone should focus on making theme selection less manual and more world-driven.

That should include:

- map-region visual tagging hooks
- theme selection based on authored world zones
- cleaner separation between world logic and visual theming
- continued reduction of hardcoded scene assumptions

M24 introduces theme sets and switching hooks.
M25 should begin wiring those hooks into the world itself.
