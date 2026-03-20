Title

M38 - Actor Runtime / Inventory / Equipment / Loot

Description

This milestone extends the signed-off authority and persistence base into the first real player actor-state loop.

The package introduces player actor runtime, starter equipment, inventory stacks, equipped state, container loot profiles, one-time loot depletion, replicated actor runtime, and persisted actor runtime. It is intentionally narrow: it makes the slice feel more like a game without dragging in economy, crafting, or a desktop-owned inventory UI too early.

Download

WAR_M38_code_package.zip

The milestone document is included inside the package at:

Milestones/M38_Actor_Runtime_Inventory_Equipment_Loot.md

Included

- README.md
- Docs/Wrath and Ruin - Actor Runtime Inventory Equipment Loot.md
- Docs/Wrath and Ruin - Persistence Schema and Migration Contract.md
- Milestones/M38_Actor_Runtime_Inventory_Equipment_Loot.md
- src/engine/core/Application.cpp
- src/engine/gameplay/Entity.h
- src/engine/gameplay/ActionSystem.h
- shared/src/gameplay/ActionSystem.cpp
- src/engine/simulation/SimulationIntent.h
- src/engine/simulation/SimulationRuntime.h
- shared/src/simulation/SimulationRuntime.cpp
- src/engine/host/AuthoritativeHostProtocol.h
- shared/src/runtime/host/AuthoritativeHostProtocol.cpp
- src/engine/host/HeadlessHostMode.cpp
- src/game/GameLayer.cpp
- src/engine/render/DebugOverlayRenderer.cpp
- src/engine/render/BgfxWorldRenderer.cpp
- shared/src/world/WorldSemanticDressing.cpp
- scripts/build_local_demo_package_win64.bat
- scripts/launch_headless_host_win64.bat
- scripts/launch_local_client_against_host_win64.bat
- scripts/launch_local_demo_win64.bat
- scripts/smoke_test_headless_host_win64.bat
- scripts/smoke_test_local_demo_win64.bat
- scripts/acceptance_m38_persistence_inventory_win64.bat
- scripts/acceptance_m38_persistence_inventory_win64.ps1

What should be seen visually

- the overlay should now expose player health, armor, inventory counts, equipped weapon/suit/tool, and loot collections
- nearby crates and lockers should now grant real items instead of behaving like pure state toggles
- first compatible recovered gear should auto-equip safely
- looted containers should remain depleted after restart
- the staged package should now stage as `WAR_M38_<Config>` and include the M38 persistence acceptance drill

Why this is important

M38 is the first milestone where the player runtime starts accumulating durable personal state rather than only moving through authored space.

That is the right prerequisite for survival pressure, terrain consequence, and combat. Hazards and six-second encounters need something meaningful to act on. M38 provides that base without expanding into systems that belong later.

What should be coming up in the next milestone

M39 - Survival Hazards / Terrain Consequence / World State
