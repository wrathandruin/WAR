# Title

**M25 — bgfx Content Tagging / Region Hooks**

# Description

M25 drives tile theming from authored world-region tags instead of a single manual theme switch.

M24 introduced theme sets and runtime authoring hooks.
M25 builds directly on that by adding `WorldRegionTag` and storing region tags per tile inside `WorldState`, so scene visuals can be selected from world structure rather than direct runtime toggles.

# Download

This milestone document is included inside the delivery package at:

```text
Milestones/M25_bgfx_Content_Tagging_Region_Hooks.md
```

# Included

- new `WorldRegionTag` world-side content tagging layer
- `WorldState` storage for per-tile region tags
- region-driven theme resolution through `visualThemeForTile`
- updated `BgfxTileVisuals` to pull theme choice from world region tags
- updated startup messaging to reflect authored world zones
- milestone document included inside the package

# What should be seen visually

The scene should still behave the same in terms of movement, camera, panning, zoom, hovering, and interaction.

The intended visible changes in M25 are:

- the map should now show distinct themed regions without needing manual theme hotkeys
- the left side of the test map should read as Industrial
- the middle band should read as Sterile
- the right side should read as Emergency
- actors should remain visually stable while tile theming changes by region

# Why this is important

M25 matters because it begins moving visual theme selection out of ad hoc controls and into authored world structure.

That is important because it:

- connects world organization to rendering outcomes
- prepares the project for zone-based scene styling
- reduces reliance on manual debug-style visual control
- strengthens the separation between renderer mechanics and content intent
- provides a cleaner base for future authoring tools and region-driven workflows

# What should be coming up in the next milestone

**M26 — bgfx Region Authoring Overlay / Palette Hooks**

The next milestone should focus on exposing region-driven styling more clearly for iteration.

That should include:

- clearer debug or overlay visibility for region boundaries
- palette/theme hooks that are easier to adjust while iterating
- preparation for world-driven styling data beyond the hardcoded test layout
- continued reduction of hardcoded visual assumptions

M25 wires themes into authored world regions.
M26 should make those regions easier to inspect, tune, and expand.
