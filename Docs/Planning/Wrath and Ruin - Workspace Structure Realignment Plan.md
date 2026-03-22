# Wrath and Ruin - Workspace Structure Realignment Plan

## Purpose

This document defines the recommended long-term workspace structure for the `WAR` repo as it grows from a single Windows prototype executable into a production-shaped client/server product.

Execution detail lives in:

- `Docs/Planning/Wrath and Ruin - Workspace Realignment Implementation Brief.md`

The goal is not to rename folders for cosmetic reasons.
The goal is to create a workspace that supports:

- separate desktop and server deliverables
- a real shared gameplay/runtime library
- future web and identity surfaces without polluting native code boundaries
- cleaner persistence and protocol ownership
- packaging, deployment, and test discipline that scales beyond one prototype executable

This direction takes direct inspiration from the legacy `Wrath-and-Ruin` workspace shape:

- `desktop/`
- `server/`
- `web/`
- `tools/`

while adapting it to the new C++ roguelike/runtime architecture.

## Executive Decision

The repo should move toward a top-level structure built around:

- `desktop/`
- `server/`
- `web/`
- `shared/`
- `contracts/`

with `assets/`, `Runtime/`, `Docs/`, `Milestones/`, `tools/`, `tests/`, and `third_party/` remaining repo-level concerns.

This should be treated as a real architecture migration, not a casual cleanup.

## Current State Snapshot

The current `WAR` repo is still physically organized like a single native application workspace:

- `src/engine/`
- `src/game/`
- `src/platform/`
- `src/tools/`
- root-level `assets/`, `Runtime/`, `scripts/`, `tests/`, and `third_party/`

That shape was acceptable while the project was proving rendering, local simulation, and the first localhost authority lane.
It becomes progressively less correct once persistence, dedicated server packaging, account/session flow, and future web or admin surfaces become first-class concerns.

The legacy `Wrath-and-Ruin` repo already proved a more product-shaped top level:

- `desktop/`
- `server/`
- `web/`
- `tools/`

This plan deliberately takes that lesson forward without copying the old codebase structure blindly.

## Recommended Target Structure

```text
WAR/
  desktop/
    src/
      app/
      game/
      render/
      diagnostics/
      platform/win32/
    include/
    tests/
    CMakeLists.txt
    WAR.Desktop.vcxproj

  server/
    src/
      app/
      host/
      diagnostics/
    include/
    tests/
    CMakeLists.txt
    WAR.Server.vcxproj

  web/
    app/
    relay/
    lib/
    styles/
    package.json

  shared/
    src/
      core/
      math/
      world/
      gameplay/
      simulation/
      runtime/
      protocol/
    include/war/
    tests/
    CMakeLists.txt

  contracts/
    protocol/
    persistence/
    content/

  assets/
    shaders/
    textures/

  Runtime/
    Config/
    CrashDumps/
    Host/
    Logs/
    Saves/

  Docs/
  Milestones/
  tools/
  tests/
  third_party/
  CMakeLists.txt
  WAR.sln
```

## Ownership Rules

### `desktop/`

Owns:

- native desktop executable boot
- windowing and input
- rendering
- client-facing diagnostics and HUD
- desktop packaging scripts and installer direction

Does not own:

- canonical gameplay rules
- authoritative truth
- persistence schema definitions

### `server/`

Owns:

- headless host executable boot
- authoritative runtime host loop
- server bootstrap and packaging
- server diagnostics, health, and process-facing controls

Does not own:

- rendering
- windowing
- client-only input affordances

### `shared/`

Owns:

- world model
- gameplay rules
- simulation runtime
- math and utility primitives
- runtime-path and protocol helpers shared between desktop and server
- deterministic or strongly controlled rules that must not drift between client and server

This should become the real native shared library for the project.

### `contracts/`

Owns:

- persistence schemas
- save/load versioning rules
- migration definitions
- wire/protocol document formats that may need to be consumed by multiple runtimes
- canonical content manifest structures

This exists because not everything that is shared conceptually should live as C++ code in `shared/`.

### `web/`

Owns:

- account/session-facing surfaces
- launcher/update portal if needed
- relay, telemetry dashboard, admin web, or support UI if they become product requirements

Important rule:
`web/` should not consume raw native C++ sources from `shared/`.
Cross-language boundaries should be carried through `contracts/`.

## What Should Stay At Repo Root

These are repo-level concerns and should remain at root:

- `assets/`
- `Runtime/`
- `Docs/`
- `Milestones/`
- `tools/`
- `tests/`
- `third_party/`
- top-level orchestration build files

Reason:
those are not owned by only one executable surface.

## Current-To-Target Move Map

### Current `src/main.cpp`

Target:

- split into `desktop/src/main.cpp`
- add `server/src/main.cpp`

Reason:
desktop and headless server should be separate binaries, not one executable with mode flags forever.

### Current `src/engine/core/`

Move to:

- `shared/src/core/`
  - `Timer.*`
  - `Log.*`
- `shared/src/runtime/`
  - `RuntimePaths.*`
- `desktop/src/diagnostics/`
  - `LocalDemoDiagnostics.*`
- `desktop/src/app/`
  - `Application.*` or its desktop equivalent after split

Reason:
`Application` is not truly shared once desktop and server become separate apps.

### Current `src/engine/host/`

Move to:

- `server/src/host/`
  - `HeadlessHostMode.*`
- `shared/src/runtime/host/`
  - `HeadlessHostPresence.*`
  - `AuthoritativeHostProtocol.*`
  - `ReplicationHarness.*`

Reason:
the host loop is server-owned, but the protocol/status/harness surfaces are shared between client and server.

### Current `src/engine/math/`

Move to:

- `shared/src/math/`

### Current `src/engine/world/`

Move to:

- `shared/src/world/`

### Current `src/engine/gameplay/`

Move to:

- `shared/src/gameplay/`

### Current `src/engine/simulation/`

Move to:

- `shared/src/simulation/`

This is one of the most important moves.
The simulation contract should not remain visually buried under a client-oriented folder tree.

### Current `src/engine/render/`

Move to:

- `desktop/src/render/`

Reason:
rendering is desktop-owned.
It should not sit beside shared gameplay code once the server becomes a first-class deliverable.

### Current `src/platform/win32/`

Move to:

- `desktop/src/platform/win32/`

Reason:
Win32 windowing is a desktop concern, not a shared/runtime concern.

### Current `src/game/`

Move to:

- `desktop/src/game/`

Reason:
the current `GameLayer` is the desktop presentation/controller layer.
It is not the canonical gameplay rules layer.

### Current `src/tools/`

Move to:

- `tools/native/` or another explicit tool-owned location

Reason:
tooling should not pretend to be runtime code.

## Build-System Direction

The root build should become an orchestration layer:

- root `CMakeLists.txt`
  - `add_subdirectory(shared)`
  - `add_subdirectory(desktop)`
  - `add_subdirectory(server)`
- root Visual Studio solution
  - `WAR`
  - `WARShared`
  - `WARServer`
  - `WARLegacy` during the transition

Recommended native shape:

- `shared` builds as a static library
- `desktop` links against `shared`
- `server` links against `shared`

This is the cleanest route to preventing gameplay drift between client and host.

## Runtime And Packaging Implications

The runtime boundary should stay conceptually the same:

- repo root `Runtime/` for source-tree execution
- package-local `runtime/` for packaged builds

However, once desktop and server are split, packaging should become explicit:

- desktop package owns desktop-local `runtime/`
- server package owns server-local `runtime/`
- shared code owns no package root of its own

## Web Guidance

The `web/` folder should exist as a reserved product surface, but it should not be forced into premature implementation.

Recommended rule:

- create the structural placeholder early
- move real code there only when session/account/relay/admin work actually begins

This avoids two bad outcomes:

- pretending web is irrelevant until too late
- creating fake web scaffolding with no real product owner

## Recommended Execution Order

### Phase A - Non-Destructive Split

Do first:

1. create `desktop/`, `server/`, `shared/`, and `contracts/`
2. create separate desktop/server project files and a root solution
3. build `shared` as a library while source files may still temporarily live in old locations
4. keep the existing executable working throughout the transition

Goal:
separate build ownership before large physical file moves.

### Phase B - Physical Source Moves

Then:

1. move shared gameplay/runtime code into `shared/`
2. move render/platform/client shell code into `desktop/`
3. move host boot and server-specific runtime into `server/`
4. update include paths and filters

Goal:
make the physical tree match the ownership model.

### Phase C - Contract Extraction

Then:

1. move persistence schemas into `contracts/persistence/`
2. move canonical protocol and content manifest definitions into `contracts/`
3. make desktop/server/web consume those contracts intentionally

Goal:
avoid cross-language coupling and make migration/versioning clearer before beta.

### Phase D - Web Activation

Only when needed:

1. add real web app or relay code under `web/`
2. connect it to account/session/ops workflows
3. keep `contracts/` as the shared semantic boundary

## Recommended Timing

Recommended execution window:

- start immediately after M36 sign-off
- complete before M40 at the latest
- ideally establish the split during M37-M38 while persistence and actor runtime are still being formalized

Reason:
persistence, inventory, actor state, and combat will all become harder to split cleanly if the workspace remains prototype-shaped too long.

## What Not To Do

- do not mix this with a broad gameplay rewrite
- do not physically move everything in one destructive commit without a working intermediate build
- do not let `web/` directly depend on native C++ internals
- do not keep one forever-executable with both desktop and server identity once dedicated packaging matters
- do not move runtime mutable state into app-owned source folders

## Acceptance Criteria For The Refactor

The structure realignment can be considered successful when:

- `desktop` builds and runs independently
- `server` builds and runs independently
- `shared` is the actual home of gameplay/runtime code used by both
- the local demo package still works
- the headless-host package still works
- source-tree and package-local runtime boundaries still behave correctly
- persistence and protocol definitions have a clear home
- the repo reads as a product workspace, not a renderer prototype tree

## Final Recommendation

Yes, the repo should move toward the old project’s top-level product/workspace separation.

The best target is:

- `desktop/`
- `server/`
- `web/`
- `shared/`
- `contracts/`

with the current repo root continuing to own assets, runtime, docs, milestones, tools, tests, and third-party dependencies.

This is the right time to plan it.
It is also early enough to do it cleanly.
