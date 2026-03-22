# Wrath and Ruin - Workspace Realignment Implementation Brief

## Purpose

This brief turns the approved workspace-direction decision into an execution plan.

It is the companion to:

- `Docs/Planning/Wrath and Ruin - Workspace Structure Realignment Plan.md`

That document explains the architectural direction.
This document explains how to implement it without breaking the build, packaging lane, or M36 authority baseline.

## Executive Outcome

The repo should move from one prototype-shaped native application tree to three deliberate native targets plus a contracts boundary:

- `WAR`
- `WARShared`
- `WARServer`
- `contracts/`

Transitional fallback target:

- `WARLegacy`

Recommended executable names:

- desktop output: `WAR.exe`
- server output: `WARServer.exe`

Recommended rule:
- keep the legacy `WAR.vcxproj` and legacy root `WAR` CMake target alive as a temporary fallback until the new desktop and server targets both build and pass smoke validation
- remove the legacy monolithic target only after the split is proven

## Current Implementation Status

Status date:
- 2026-03-20

Stage 1 scaffold, Stage 2 boot separation, and the first shared implementation move batch are now landed and validated.

Implemented:

- `desktop/`, `server/`, `shared/`, and `contracts/` folder scaffolding
- new desktop and server entry points
- transitional `shared/CMakeLists.txt`, `desktop/CMakeLists.txt`, and `server/CMakeLists.txt`
- `WAR.Shared.vcxproj`
- `WAR.Desktop.vcxproj`
- `WAR.Server.vcxproj`
- solution wiring that keeps legacy `WAR.vcxproj` in place as the fallback target
- extracted headless-host startup into `HeadlessHostBootstrap` so desktop boot no longer owns host startup
- updated launch, smoke, package, and acceptance scripts to prefer `WARServer.exe`
- upgraded the packaged M36 acceptance lane to a PowerShell-backed runner behind the existing batch entry point
- moved the first shared implementation batch into `shared/src/` for math, world, gameplay, simulation, and runtime-host helpers

Solution-facing target names now resolve as:

- `WAR` -> new desktop/client split target
- `WARShared` -> shared gameplay/runtime library target
- `WARServer` -> new server split target
- `WARLegacy` -> old monolithic fallback target

Validated on the Windows/MSBuild lane:

- `WAR.Shared.vcxproj` `Debug|x64`: pass
- `WAR.Desktop.vcxproj` `Debug|x64`: pass
- `WAR.Server.vcxproj` `Debug|x64`: pass
- legacy `WAR.vcxproj` `Debug|x64`: pass
- solution target `/t:WARShared`: pass
- solution target `/t:WAR`: pass
- solution target `/t:WARServer`: pass
- solution target `/t:WARLegacy`: pass
- `bin/Debug/split/server/WARServer.exe --host-run-seconds=1`: pass
- `scripts/build_local_demo_package_win64.bat Debug`: pass
- staged `smoke_test_headless_host_win64.bat`: pass
- staged `acceptance_m36_localhost_authority_win64.bat`: pass

Current transitional notes:
- the legacy `WARLegacy` target remains intentionally alive as the fallback lane
- the legacy `WAR.exe --headless-host` path remains available only through `WARLegacy`
- shared implementation `.cpp` files have begun moving, but most headers still live under the legacy `src/` include root
- render, platform, diagnostics, and server host-loop source files have not yet been physically relocated into their final folders

## Non-Negotiable Constraints

1. The repo must remain buildable after every migration stage.
2. The local demo lane must continue to work throughout the transition.
3. The M36 localhost authority validation must not regress.
4. `shared` must not take a dependency on rendering, Win32 windowing, or desktop-only diagnostics.
5. `server` must not depend on bgfx, GDI, or client presentation code.
6. `web` remains a reserved surface until real product work exists for it.

## Exact Target Build Topology

### Root

The root should become an orchestration layer:

```text
WAR/
  CMakeLists.txt
  WAR.sln
  desktop/
  server/
  shared/
  contracts/
  assets/
  Runtime/
  Docs/
  Milestones/
  scripts/
  tests/
  third_party/
```

### CMake

The root `CMakeLists.txt` should ultimately do only orchestration:

```cmake
add_subdirectory(shared)
add_subdirectory(desktop)
add_subdirectory(server)
```

Recommended target layout:

- `shared/CMakeLists.txt`
  - `add_library(WARShared STATIC ...)`
- `desktop/CMakeLists.txt`
  - `add_executable(WAR WIN32 ...)`
  - `set_target_properties(... OUTPUT_NAME "WAR")`
  - link against `WARShared`
- `server/CMakeLists.txt`
  - `add_executable(WARServer ...)`
  - `set_target_properties(... OUTPUT_NAME "WARServer")`
  - link against `WARShared`

Desktop-only dependencies:

- bgfx
- bx
- bimg
- `user32`
- `gdi32`

Server target dependencies:

- shared runtime only
- no render libraries
- no Win32 windowing surface

### Visual Studio

The root solution should ultimately contain:

- `WAR.Shared.vcxproj`
- `WAR.Desktop.vcxproj`
- `WAR.Server.vcxproj`

Transitional rule:
- retain `WAR.vcxproj` during the split
- mark it as `WARLegacy` in the solution
- remove it only when the new projects fully replace its build and smoke coverage

## Initial Project Ownership

### `WARShared`

Should own these current source groups first:

- `src/engine/math/*`
- `src/engine/world/*`
- `src/engine/gameplay/*`
- `src/engine/simulation/*`
- `src/engine/host/AuthoritativeHostProtocol.*`
- `src/engine/host/HeadlessHostPresence.*`
- `src/engine/host/ReplicationHarness.*`
- `src/engine/core/Timer.*`
- `src/engine/core/Log.*`
- `src/engine/core/RuntimePaths.*`

Key rule:
- `WARShared` is where gameplay truth lives
- if a file needs rendering or window input to compile, it does not belong here

### `WAR`

Should own these current source groups first:

- `src/engine/core/Application.*`
- `src/engine/core/LocalDemoDiagnostics.*`
- `src/engine/render/*`
- `src/platform/IWindow.h`
- `src/platform/win32/*`
- `src/game/*`
- transitional desktop entry point derived from `src/main.cpp`

Key rule:
- desktop is the player-facing shell, renderer, input surface, and local demo packaging surface

### `WARServer`

Should own these current source groups first:

- `src/engine/host/HeadlessHostMode.*`
- new server entry point derived from the current `--headless-host` path

Key rule:
- the server target owns process boot, headless host loop, and future deployable dedicated-host packaging

## First Folder Creation Pass

This pass should be non-destructive.
Create the structure before moving the code.

### Create Now

```text
desktop/
  src/app/
  src/game/
  src/render/
  src/diagnostics/
  src/platform/win32/
  include/
  tests/

server/
  src/app/
  src/host/
  src/diagnostics/
  include/
  tests/

shared/
  src/core/
  src/math/
  src/world/
  src/gameplay/
  src/simulation/
  src/runtime/
  src/runtime/host/
  include/war/
  tests/

contracts/
  protocol/
  persistence/
  content/
```

### Do Not Move Yet

During the first pass, do not immediately relocate `src/` contents.
Instead:

1. create the new folders
2. add the new CMake files and `.vcxproj` files
3. point them at the current legacy source locations
4. prove the split builds before the physical moves begin

This is the safest way to preserve velocity.

That stage is now complete.
The repo has already moved into the next step: split targets build successfully, host boot is separated, and the first shared implementation batch has begun moving physically into `shared/src/`.

## Exact Non-Destructive Build Split

### Stage 1 - Introduce `WARShared`

Status:
- complete

Build `WARShared` first from existing source locations.

Use the current files in place, but compile them into a static library owned by the new shared project.

Initial shared source list:

- `src/engine/core/Timer.cpp`
- `src/engine/core/Log.cpp`
- `src/engine/core/RuntimePaths.cpp`
- `src/engine/host/AuthoritativeHostProtocol.cpp`
- `src/engine/host/HeadlessHostPresence.cpp`
- `src/engine/host/ReplicationHarness.cpp`
- `src/engine/math/Vec2.cpp`
- `src/engine/world/WorldGrid.cpp`
- `src/engine/world/WorldRegionTag.cpp`
- `src/engine/world/WorldSemanticDressing.cpp`
- `src/engine/world/Pathfinding.cpp`
- `src/engine/world/WorldState.cpp`
- `src/engine/gameplay/EntityManager.cpp`
- `src/engine/gameplay/ActionSystem.cpp`
- `src/engine/simulation/SimulationRuntime.cpp`

Headers remain where they are for the first stage.

### Stage 2 - Introduce `WAR`

Status:
- complete

Create `WAR` and point it to:

- new `desktop/src/main.cpp`
- existing desktop-owned sources still in legacy locations
- `WARShared`

Initial desktop-owned source list:

- `src/engine/core/Application.cpp`
- `src/engine/core/LocalDemoDiagnostics.cpp`
- all current `src/engine/render/*.cpp`
- `src/platform/win32/Win32Window.cpp`
- `src/game/GameLayer.cpp`

Initial desktop include roots:

- repo `src/`
- desktop-local include root
- shared include root

Transitional behavior:
- `desktop/src/main.cpp` should become the new app boot
- it may temporarily include/adapt logic from the current `src/main.cpp`

Current result:
- desktop boot is now separate from headless-host startup
- the `WAR` desktop target no longer needs `HeadlessHostMode.cpp`

### Stage 3 - Introduce `WARServer`

Status:
- complete

Create `WARServer` and point it to:

- new `server/src/main.cpp`
- `src/engine/host/HeadlessHostMode.cpp`
- `WARShared`

Transitional behavior:
- the new server entry point should replace the old `--headless-host` process mode
- keep the old mode path temporarily available in the legacy target until server validation is green

Current result:
- `WARServer.exe` is now the preferred host process in scripts and staged local demo packages

## Safe Physical Move Order

Once the new targets build from legacy paths, move code physically in this order.

Current progress:
- the first implementation batch has already moved into `shared/src/`
- the move is intentionally implementation-first; headers still remain under `src/` as the transitional include root
- the current moved `.cpp` set includes math, world, gameplay, simulation, runtime paths, and localhost authority helper implementations

### Move Order 1 - Lowest Blast Radius Shared Primitives

Move first:

- `src/engine/math/Vec2.*` -> `shared/src/math/`
- `src/engine/gameplay/Action.*` -> `shared/src/gameplay/`
- `src/engine/gameplay/ActionQueue.h` -> `shared/src/gameplay/`
- `src/engine/gameplay/Entity.*` -> `shared/src/gameplay/`
- `src/engine/gameplay/EntityManager.*` -> `shared/src/gameplay/`
- `src/engine/gameplay/ActionSystem.*` -> `shared/src/gameplay/`
- `src/engine/simulation/*` -> `shared/src/simulation/`

Reason:
these are gameplay-side units and should move before render or process boot code.

### Move Order 2 - Shared World Layer

Move next:

- `src/engine/world/*` -> `shared/src/world/`

Reason:
world state, region tagging, and pathfinding are core gameplay/runtime ownership, not desktop ownership.

### Move Order 3 - Shared Runtime Helpers

Move next:

- `src/engine/core/Timer.*` -> `shared/src/core/`
- `src/engine/core/Log.*` -> `shared/src/core/`
- `src/engine/core/RuntimePaths.*` -> `shared/src/runtime/`
- `src/engine/host/AuthoritativeHostProtocol.*` -> `shared/src/runtime/host/`
- `src/engine/host/HeadlessHostPresence.*` -> `shared/src/runtime/host/`
- `src/engine/host/ReplicationHarness.*` -> `shared/src/runtime/host/`

Reason:
these files sit near process boundaries and diagnostics, so they should move only after the pure gameplay layers are already stable.

### Move Order 4 - Desktop Presentation Layer

Move next:

- `src/engine/render/*` -> `desktop/src/render/`
- `src/platform/IWindow.h` -> `desktop/src/platform/`
- `src/platform/win32/*` -> `desktop/src/platform/win32/`
- `src/game/*` -> `desktop/src/game/`
- `src/engine/core/LocalDemoDiagnostics.*` -> `desktop/src/diagnostics/`
- `src/engine/core/Application.*` -> `desktop/src/app/`

Reason:
these are presentation/process-shell files and should move only after the shared layer no longer depends on them.

### Move Order 5 - Server Host Loop

Move next:

- `src/engine/host/HeadlessHostMode.*` -> `server/src/host/`

Reason:
the host loop should be the last major source move because it sits at the seam between shared logic, runtime paths, diagnostics, and process boot.

### Move Order 6 - Entry Points And Cleanup

Finish with:

- replace `src/main.cpp` with `desktop/src/main.cpp` and `server/src/main.cpp`
- retire legacy include roots no longer needed
- remove empty legacy directories gradually
- remove the legacy monolithic `WAR.vcxproj` and legacy root `WAR` target only after parity is proven

## Include-Path Strategy During Migration

Do not attempt a one-commit include cleanup.

Recommended include strategy:

1. keep `src/` as a transitional include root
2. add `desktop/include/` and `shared/include/war/`
3. move source files physically in batches
4. only after the physical moves stabilize, normalize include paths to their new ownership model

This minimizes churn and reduces the chance of breaking the Visual Studio project and CMake at the same time.

## Script And Packaging Strategy

During the split:

- keep `scripts/` at repo root
- keep `assets/` at repo root
- keep `Runtime/` at repo root for source-tree execution

Then formalize package ownership:

- desktop package owns desktop-local `runtime/`
- server package owns server-local `runtime/`

Recommended rule:
- do not move packaging scripts into `desktop/` or `server/` until the new targets are stable
- update scripts to call the new desktop/server project files only after those project files are proven

## Validation Gates

The split is not healthy unless all of the following continue to pass.

### After Stage 1

- `WARShared` builds cleanly
- legacy `WAR.vcxproj` still builds

### After Stage 2

- `WAR` builds
- `WAR.exe` launches
- existing smoke flow still works

### After Stage 3

- `WARServer` builds
- `WARServer.exe --host-run-seconds=3` succeeds
- host heartbeat and snapshot files still generate correctly

### After Move Orders 1-3

- desktop and server both build against the moved shared code
- M36 authority acceptance still passes

Current status:
- achieved for the first shared implementation batch

### After Move Orders 4-6

- local demo package still stages
- headless host package still stages
- source-tree runtime paths still behave correctly
- package-local runtime paths still behave correctly
- the legacy monolithic target is no longer needed

Current status:
- package staging and package-local runtime behavior are already revalidated
- the legacy monolithic target is still intentionally retained until later M37/M38 work completes the broader file moves

## Dependency Rules To Enforce

These rules should be treated as hard boundaries.

- `shared` may not include from `desktop/src/`
- `shared` may not include from `server/src/`
- `server` may not include from `desktop/src/render/`
- `server` may not link bgfx or GDI
- `desktop` may consume `shared`, but not the reverse
- `web` may consume `contracts`, but not raw C++ internals from `shared`

## Recommended Milestone Placement

Recommended execution window:

- begin during M37
- complete the non-destructive split no later than M38
- avoid letting M39-M40 land on top of the old monolithic source tree

Recommended sequencing against gameplay:

- do the target split first
- do the lowest-blast-radius shared moves next
- let persistence and actor runtime work land into the new ownership model wherever possible

Status note:
- the repo is now entering M37 from exactly that position: split targets are live, the shared move has begun, and persistence can start against the correct ownership model instead of the old monolithic tree

## Lead Developer Checklist

1. Create `desktop/`, `server/`, `shared/`, and `contracts/`.
2. Add `WAR.Shared.vcxproj`, `WAR.Desktop.vcxproj`, and `WAR.Server.vcxproj`.
3. Add `shared/CMakeLists.txt`, `desktop/CMakeLists.txt`, and `server/CMakeLists.txt`.
4. Keep the legacy `WAR.vcxproj` alive temporarily.
5. Boot `WAR` from a new desktop entry point.
6. Boot `WARServer` from a new server entry point.
7. Link both against `WARShared`.
8. Move shared code in the prescribed order.
9. Move desktop and server shells after shared code is stable.
10. Re-run local demo, headless host, and M36 acceptance after every phase.
11. Remove the monolithic target only after parity is real.

## Final Recommendation

Do not start this by moving files.

Start it by:

1. creating the new targets
2. compiling them from the existing legacy source locations
3. proving build parity
4. then moving code in controlled batches

That is the lowest-risk path to a cleaner, market-ready workspace.
