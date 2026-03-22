Title

M41 - Planetary Mission Slice / Objectives / Gate Logic

Description

M41 turns the signed-off M40 planetary runtime into a real mission-bearing vertical slice.

The player now has a directed progression path instead of a free-roaming prototype loop.
Mission truth lives in the shared/server runtime, advances from authored inspect/interact/combat beats, and persists through the authoritative save lane.

The implemented mission chain is intentionally narrow:

- recover route data from the Transit Service Terminal
- inspect the Diagnostic Station in the MedLab
- unlock and secure the Quarantine Access Gate through the six-second combat lane
- restore the corridor at the Quarantine Control Terminal
- finish with an explicit M42 ship-runtime handoff-ready state

This milestone does not attempt to become a generic quest framework.
It establishes one clean authoritative mission lane, one real gate, and one durable progression state model.

Download

WAR_M41_code_package.zip

The milestone document is included inside the package at:

Milestones/M41_Planetary_Mission_Slice_Objectives_Gate_Logic.md

Included

- CMakeLists.txt
- README.md
- Milestones/WAR_Strategic_Roadmap.md
- Milestones/M41_Planetary_Mission_Slice_Objectives_Gate_Logic.md
- Docs/Reports/Wrath and Ruin - M41 Validation and Acceptance Checklist.md
- scripts/build_local_demo_package_win64.bat
- scripts/acceptance_m41_planetary_mission_slice_win64.bat
- scripts/acceptance_m41_planetary_mission_slice_win64.ps1
- src/engine/world/WorldGrid.h
- shared/src/world/WorldGrid.cpp
- src/engine/simulation/SimulationIntent.h
- src/engine/simulation/SimulationRuntime.h
- shared/src/simulation/SimulationRuntime.cpp
- shared/src/gameplay/ActionSystem.cpp
- shared/src/runtime/host/AuthoritativeHostProtocol.cpp
- shared/src/runtime/host/HeadlessHostPresence.cpp
- src/engine/host/HeadlessHostMode.cpp
- src/engine/render/DebugOverlayRenderer.cpp
- src/engine/render/BgfxDebugFrameRenderer.h
- src/engine/render/BgfxDebugFrameRenderer.cpp
- src/game/GameLayer.cpp

What should be seen visually

- the overlay or bgfx debug view shows a readable current objective
- the player starts with a directed mission instruction instead of aimless sandbox drift
- the quarantine route is gated until the MedLab diagnostic beat is completed
- the event log explains each mission advancement beat
- the Quarantine Access Gate combat beat matters to progression instead of feeling optional
- the mission completes with an explicit ship handoff-ready state for M42
- after restart, the objective, gate state, and mission completion state restore correctly

Why this is important

M41 is the milestone that makes the current systems feel like one product slice instead of five isolated proofs.

Combat, hazards, inventory, persistence, and authority are now chained together by a purpose-driven mission path.
That is the minimum structure the project needs before ship runtime and space traversal can be added safely in M42.

What should be coming up in the next milestone

M42 - Ship Runtime / Boarding / Ownership State
