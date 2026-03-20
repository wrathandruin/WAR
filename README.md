# WAR — Milestone 31 (Canonical Content Contract / Runtime Boundary Cleanup)

> Current development milestone: M31 — Canonical Content Contract / Runtime Boundary Cleanup

## Focus
Turn repo layout and runtime ownership into explicit production rules instead of tribal knowledge.

M29 established authored encounter-ready anchors.
M30 improved readability and interaction affordances.
M31 formalizes what belongs in source control, what belongs in runtime-only directories, and how the client should resolve and create runtime roots during local development versus packaged execution.

## What this milestone does
- adds a runtime-boundary report and directory resolver in `RuntimePaths`
- establishes explicit runtime roots for `Config`, `Logs`, `Saves`, and `CrashDumps`
- creates those runtime directories on boot so mutable state has a defined home
- distinguishes source-tree execution from packaged execution at runtime
- surfaces repo root, asset root, runtime root, and boundary issues in diagnostics
- updates `.gitignore` so runtime-only mutable data stops competing with versioned source
- adds a dedicated runtime-boundary contract document for the repo

## Canonical versus runtime after M31
Canonical source-controlled truth now means:

- `src/` for code
- `assets/` for versioned shaders and textures
- `Docs/` for production docs and policy
- `Milestones/` for milestone handoffs
- build and project files that define the trunk

Runtime-only mutable state now means:

- `Runtime/Config/`
- `Runtime/Logs/`
- `Runtime/Saves/`
- `Runtime/CrashDumps/`

Local source-tree runs resolve runtime state into the repo `Runtime/` root.
Packaged runs resolve runtime state into an executable-local `runtime/` root.

## Operational proof after M31
On startup, diagnostics should make these things visible:

- whether the build is running from a source-tree or packaged layout
- which repo and asset roots were resolved
- which runtime root is active
- whether required runtime directories were created
- whether the asset/runtime contract has warnings

## Why this matters
M31 is a production milestone.

It reduces future risk in:

- persistence work
- packaging and demo lanes
- deployable server/client layout
- asset-pipeline discipline
- save, log, and crash-data hygiene

This is the milestone that stops the repo from drifting into source-versus-runtime ambiguity before persistence, packaging, and server authority expand the cost of mistakes.

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

And the shared sprite atlas at:

```text
assets/textures/world_atlas.bmp
```

## Next Milestone
### M32 — Local Demo Lane / Packaging / Diagnostics Baseline
- create a repeatable local build-and-launch lane
- formalize demo preparation and startup checks
- move from repo contract into reproducible packaging discipline
