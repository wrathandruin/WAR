Title

M42 - Ship Runtime / Boarding / Ownership State

Description

M42 extends the M41 planetary mission slice into the first real docked-ship runtime.

The ship is no longer a future placeholder.
It is now a persistent gameplay object with authoritative boarding, occupancy, power, airlock, and command-ownership state.

This implementation stays intentionally narrow:

- finish the M41 quarantine-restoration chain
- authorize boarding only after the mission path earns it
- board the docked responder shuttle in the cargo bay
- claim command at the shuttle helm
- persist ship state cleanly for M43 travel work

M42 does not attempt orbital movement, launch execution, docking at another destination, or a broad ship roster.
It establishes one real ship-state lane that M43 can build on without architectural drift.

Download

WAR_M42_code_package.zip

The milestone document is included inside the package at:

Milestones/M42_Ship_Runtime_Boarding_Ownership_State.md

Included

- CMakeLists.txt
- README.md
- Milestones/WAR_Strategic_Roadmap.md
- Milestones/M42_Ship_Runtime_Boarding_Ownership_State.md
- Docs/Wrath and Ruin - M42 Validation and Acceptance Checklist.md
- scripts/build_local_demo_package_win64.bat
- scripts/acceptance_m42_ship_runtime_boarding_win64.bat
- scripts/acceptance_m42_ship_runtime_boarding_win64.ps1
- shared/src/world/WorldSemanticDressing.cpp
- src/engine/gameplay/Entity.h
- shared/src/gameplay/ActionSystem.cpp
- src/engine/simulation/SimulationIntent.h
- src/engine/simulation/SimulationRuntime.h
- shared/src/simulation/SimulationRuntime.cpp
- shared/src/runtime/host/AuthoritativeHostProtocol.cpp
- shared/src/runtime/host/HeadlessHostPresence.cpp
- src/engine/host/HeadlessHostMode.cpp
- src/engine/render/DebugOverlayRenderer.cpp
- src/engine/render/BgfxDebugFrameRenderer.h
- src/engine/render/BgfxDebugFrameRenderer.cpp
- src/game/GameLayer.cpp

What should be seen visually

- the objective chain now continues from quarantine restoration into boarding and ship command
- `Responder Shuttle Khepri` exists as a real interactable ship object in the cargo bay
- boarding remains locked until the planetary mission is resolved
- after boarding, diagnostics show the player is aboard the docked ship
- after helm interaction, diagnostics show player-command ownership and launch-prep readiness
- ship occupancy and ownership restore correctly after restart

Why this is important

M42 is the bridge between the planetary slice and the future orbital layer.

Without a real docked-ship runtime, M43 would have to invent travel on top of a placeholder.
This milestone makes the ship part of the authoritative product instead of a promise in the roadmap.

What should be coming up in the next milestone

M43 - Orbital Space Layer / Travel State / Navigation Rules
