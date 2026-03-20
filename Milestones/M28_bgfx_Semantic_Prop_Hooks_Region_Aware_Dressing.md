# Title

**M28 — bgfx Semantic Prop Hooks / Region-Aware Dressing**

# Description

M28 begins tying scene props and visual dressing to semantic region data.

M27 expanded the world into semantic regions.
M28 builds directly on that by introducing `WorldSemanticDressing`, which seeds entities from region intent and resolves them into valid tiles inside those semantic spaces. It also makes prop coloring region-aware so the scene begins to read differently from one authored space to another.

# Download

This milestone document is included inside the delivery package at:

```text
Milestones/M28_bgfx_Semantic_Prop_Hooks_Region_Aware_Dressing.md
```

# Included

- new `WorldSemanticDressing` hook for region-aware test-world prop population
- semantic prop seeding inside `WorldState`
- region-aware entity coloring in bgfx rendering
- region-aware entity coloring in GDI fallback rendering
- updated milestone messaging for semantic prop dressing
- milestone document included inside the package

# What should be seen visually

The scene should still behave the same in terms of movement, camera, panning, zoom, hovering, and interaction.

The intended visible changes in M28 are:

- the map should contain more props distributed according to semantic regions
- Cargo Bay should read more like a storage-heavy space with clustered crates
- Transit Spine should show service-oriented props along circulation routes
- Med Lab should read cleaner and more equipment-driven
- Command Deck should feel more controlled and console-oriented
- Hazard Containment should show more danger-coded props and locked storage
- props should now pick up color treatment that matches the region they belong to

# Why this is important

M28 matters because it turns semantic regions into something that influences actual scene contents.

That is important because it:

- makes regions feel authored rather than merely labeled
- prepares the project for region-aware prop expansion
- helps visual storytelling emerge from the world layer
- provides a bridge from semantic regions into encounter and gameplay hooks
- keeps renderer mechanics stable while content meaning grows

# What should be coming up in the next milestone

**M29 — bgfx Regional Set Dressing / Encounter Hooks**

The next milestone should expand semantic prop density and begin connecting authored regions to gameplay-facing encounter hooks and regional set dressing logic.
