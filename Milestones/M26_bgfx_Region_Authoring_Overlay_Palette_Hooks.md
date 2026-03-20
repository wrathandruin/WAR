# Title

**M26 — bgfx Region Authoring Overlay / Palette Hooks**

# Description

This corrected M26 pass fixes the overlay so it is actually visible.

The previous overlay concept was too subtle and, on the GDI fallback path, effectively absent. This correction makes the overlay explicit in both render paths by drawing strong boundary markers wherever neighboring tiles belong to different world regions.

# Download

This milestone document is included inside the delivery package at:

```text
Milestones/M26_bgfx_Region_Authoring_Overlay_Palette_Hooks.md
```

# Included

- visible region boundary overlay support in `WorldState`
- palette mode support in `WorldState`
- expanded `BgfxWorldTheme` tint and boundary color logic
- region boundary generation in bgfx render data
- visible region boundary rendering in the GDI renderer
- runtime authoring hotkeys in `GameLayer`
- milestone document included inside the package

# What should be seen visually

The scene should still behave the same in terms of movement, camera, panning, zoom, hovering, and interaction.

The intended visible changes in corrected M26 are:

- pressing **O** should clearly show or hide strong region boundary markers
- on the current test map, you should see two obvious vertical seams:
  - one between **Industrial** and **Sterile**
  - one between **Sterile** and **Emergency**
- pressing **7 / 8 / 9** should still change the palette intensity for region-driven tile colors

# Why this is important

This correction matters because the previous M26 overlay was not useful as an authoring tool.

The corrected version:

- makes overlay toggling meaningful
- works in both bgfx and GDI fallback rendering
- makes region-driven structure immediately readable
- improves trust in the world-authoring loop
- keeps the milestone aligned with its intended purpose

# What should be coming up in the next milestone

**M27 — bgfx Region Data Expansion / Scene Semantics**

The next milestone should build on this by making region layouts richer and more semantically meaningful than the current simple zone split.
