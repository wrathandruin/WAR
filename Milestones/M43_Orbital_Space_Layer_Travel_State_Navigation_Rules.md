Title

M43 - Orbital Space Layer / Travel State / Navigation Rules

Description

M43 turns the M42 docked responder shuttle into the first real local-orbit runtime.

The player no longer stops at “ship command claimed.”
They can now depart the dock, enter a local orbital traffic lane, complete a debris survey orbit transfer, and stabilize a relay holding track through authoritative runtime state that persists across restart.

This implementation is intentionally narrow:

- preserve the full M41 and M42 chain without regression
- keep orbital truth in shared/server runtime
- add one local orbital route ladder with readable rules
- persist orbital mission, node, phase, and transfer state
- stop before docking, landing, second-destination arrival, or return-loop breadth

M43 does not attempt a broad flight model, broad orbital combat, or M44 landing/docking work.
It exists to make the first orbital layer real enough that M44 can extend it safely.

Download

WAR_M43_code_package.zip

The milestone document is included inside the package at:

Milestones/M43_Orbital_Space_Layer_Travel_State_Navigation_Rules.md

Included

- CMakeLists.txt
- README.md
- Milestones/WAR_Strategic_Roadmap.md
- Milestones/M43_Orbital_Space_Layer_Travel_State_Navigation_Rules.md
- Docs/Reports/Wrath and Ruin - M43 Validation and Acceptance Checklist.md
- scripts/build_local_demo_package_win64.bat
- scripts/acceptance_m43_orbital_space_layer_win64.bat
- scripts/acceptance_m43_orbital_space_layer_win64.ps1
- shared/src/world/WorldSemanticDressing.cpp
- src/engine/simulation/SimulationIntent.h
- src/engine/simulation/SimulationRuntime.h
- shared/src/simulation/SimulationRuntime.cpp
- shared/src/runtime/host/AuthoritativeHostProtocol.cpp
- shared/src/runtime/host/HeadlessHostPresence.cpp
- src/engine/host/HeadlessHostMode.cpp
- src/engine/render/DebugOverlayRenderer.cpp
- src/engine/render/BgfxDebugFrameRenderer.cpp
- src/game/GameLayer.cpp

What should be seen visually

- the M41 and M42 mission chain still completes in order
- after shuttle command is claimed, the objective explicitly advances into orbital departure
- `player_runtime_context` changes to `orbital-space` after the shuttle leaves the dock
- overlay and status output show orbital phase, current node, target node, rule text, transfer count, and remaining travel ticks
- the orbital ladder progresses through traffic lane -> debris survey orbit -> relay holding track
- relay holding track completion visibly sets up M44 without pretending docking/landing already exists

Why this is important

M43 is the first proof that Wrath and Ruin can carry a player cleanly from a planetary slice into a persisted orbital runtime.

Without this milestone, M44 would have to fake cross-layer continuity on top of a docked-ship endpoint.
With it, docking, landing, and return persistence can build on an actual orbital state carrier.

What should be coming up in the next milestone

M44 - Docking / Landing / Cross-Layer Transition Persistence / Return Loop
