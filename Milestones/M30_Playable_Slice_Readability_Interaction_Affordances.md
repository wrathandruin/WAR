Title

M30 - Playable Slice Readability / Interaction Affordances

Description

M30 improves the current playable slice so it reads as an intentional short-session demo instead of an internal rendering test.

This delivery focuses on clarity rather than new mechanics: stronger hover and interaction affordances, selected-tile and move-target visibility, more legible path presentation, clearer inspect and interact text, and demo-facing diagnostics that explain what the player is looking at and what the next input will do.

The milestone keeps scope disciplined. It does not try to add inventory, hazards, combat, or mission systems early. It makes the existing slice readable enough to support demo review, usability review, and cleaner integration into the runtime-boundary and packaging milestones that follow.

Download

WAR_M30_code_package.zip

The milestone document is included inside the package at:

Milestones/M30_Playable_Slice_Readability_Interaction_Affordances.md

Included

- CMakeLists.txt
- README.md
- src/game/GameLayer.h
- src/game/GameLayer.cpp
- src/engine/gameplay/ActionSystem.cpp
- src/engine/render/WorldRenderer.h
- src/engine/render/WorldRenderer.cpp
- src/engine/render/DebugOverlayRenderer.h
- src/engine/render/DebugOverlayRenderer.cpp
- src/engine/render/BgfxRenderData.h
- src/engine/render/BgfxRenderData.cpp
- src/engine/render/BgfxWorldRenderer.h
- src/engine/render/BgfxWorldRenderer.cpp
- Milestones/M30_Playable_Slice_Readability_Interaction_Affordances.md

What should be seen visually

- Hovered tiles should now read differently depending on whether they are blocked, walkable, interactable, or authored hotspots.
- The last selected tile should remain visible as a blue focus marker.
- An active movement destination should remain visible as a gold move-target marker until the path completes.
- Path presentation should read more clearly, with a stronger sense of destination.
- Hovered entities and selected entities should stand out more cleanly.
- The debug overlay should now show hovered affordance, prompt text, selected tile, move target, path destination, and clearer hotspot/entity detail.
- In bgfx, the debug status text should now surface current hover context and action prompt instead of only a generic renderer status line.

Why this is important

M30 gives the slice the minimum clarity required for disciplined demos and future gameplay integration.

That matters strategically because M31 and M32 are production-contract and packaging milestones. Those milestones are more valuable when the current slice is understandable enough that the team can judge what is actually working, what is confusing, and what is ready to be formalized.

This milestone also protects scope. It improves the quality of the current slice without derailing the roadmap into premature feature breadth.

What should be coming up in the next milestone

M31 - Canonical Content Contract / Runtime Boundary Cleanup
