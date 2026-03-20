Title

M29 - bgfx Regional Set Dressing / Encounter Hooks

Description

M29 closes the immediate renderer-forward authoring pass by making semantic regions read more deliberately and by introducing authored hotspot hooks that later gameplay systems can consume.

This delivery expands region-aware prop dressing density, adds encounter-ready and mission-ready hotspot anchors to the world layer, exposes those anchors through inspect and interact messaging, and renders them in both bgfx and GDI. The intent is not to broaden mechanics prematurely. The intent is to finish the visual-authoring bridge cleanly and leave the repo with a stronger gameplay-facing content contract for M30 and beyond.

Download

WAR_M29_code_package.zip

The milestone document is included inside the package at:

Milestones/M29_bgfx_Regional_Set_Dressing_Encounter_Hooks.md

Included

- CMakeLists.txt
- README.md
- src/engine/world/WorldState.h
- src/engine/world/WorldState.cpp
- src/engine/world/WorldSemanticDressing.h
- src/engine/world/WorldSemanticDressing.cpp
- src/engine/gameplay/ActionSystem.cpp
- src/game/GameLayer.h
- src/game/GameLayer.cpp
- src/engine/render/DebugOverlayRenderer.h
- src/engine/render/DebugOverlayRenderer.cpp
- src/engine/render/WorldRenderer.h
- src/engine/render/WorldRenderer.cpp
- src/engine/render/BgfxRenderData.h
- src/engine/render/BgfxRenderData.cpp
- src/engine/render/BgfxWorldRenderer.cpp
- Milestones/M29_bgfx_Regional_Set_Dressing_Encounter_Hooks.md

What should be seen visually

- Cargo Bay, Transit Spine, Med Lab, Command Deck, and Hazard Containment should look more intentionally dressed because prop density and placement variety have increased.
- Authoring hotspot anchors should now be visible in the world as distinct overlay markers when hotspot overlay is enabled.
- Shift+RMB inspect on hotspot tiles should report region, hotspot label, hotspot type, and whether the hook is encounter-ready.
- RMB interact on empty hotspot tiles should emit future-facing gameplay anchor messages instead of doing nothing.
- Pressing O should still toggle the region boundary overlay.
- Pressing H should now toggle the authored hotspot overlay.
- The debug overlay should report hovered region, hovered hotspot details, hotspot count, and overlay state.

Why this is important

M29 is the last milestone where rendering is allowed to remain the visible center of gravity.

Its real value is structural, not cosmetic:
the world layer now owns authored hotspot data that future encounter, hazard, mission, authority, and persistence work can consume without inventing anchor points late. That reduces future integration risk and closes the renderer-first chapter in a disciplined way instead of dragging it out.

What should be coming up in the next milestone

M30 - Playable Slice Readability / Interaction Affordances
