# Title

**M27 — bgfx Region Data Expansion / Scene Semantics**

# Description

M27 expands the world-region model beyond a simple three-band split and starts giving regions actual scene meaning.

M26 made region overlays visible and palette tuning useful.
M27 builds directly on that by replacing the broad Industrial / Sterile / Emergency bands with more semantic region tags such as Cargo Bay, Transit Spine, Med Lab, Command Deck, and Hazard Containment.

# Download

This milestone document is included inside the delivery package at:

```text
Milestones/M27_bgfx_Region_Data_Expansion_Scene_Semantics.md
```

# Included

- expanded semantic `WorldRegionTag` set
- richer authored test-region layout in `WorldState`
- preserved theme resolution from world regions into render themes
- preserved overlay and palette iteration controls from M26
- updated startup messaging for semantic region awareness
- milestone document included inside the package

# What should be seen visually

The scene should still behave the same in terms of movement, camera, panning, zoom, hovering, and interaction.

The intended visible changes in M27 are:

- the map should no longer read like three simple vertical theme bands
- you should see a more authored arrangement of themed spaces
- a horizontal and vertical transit spine should cut through the layout
- the upper middle should read like a sterile Med Lab
- the upper right should read like a sterile Command Deck
- the lower right should read like Hazard Containment
- region overlay seams should now trace these richer zone boundaries rather than only two simple vertical splits

# Why this is important

M27 matters because it starts making region data describe scene purpose instead of only scene color.

That is important because it:

- gives the world more meaningful authored structure
- prepares region tags to drive more than palette selection
- creates a better base for prop placement and semantic dressing
- reduces the sense that regions are only a rendering trick
- strengthens the link between world intent and visual output

# What should be coming up in the next milestone

**M28 — bgfx Semantic Prop Hooks / Region-Aware Dressing**

The next milestone should begin tying scene props and visual dressing to semantic region data so authored spaces influence more than just tile theming.
