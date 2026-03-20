# M31 - Canonical Content Contract / Runtime Boundary Cleanup

## Summary

M31 formalizes the repo's canonical-versus-runtime boundary.

The milestone adds an explicit runtime path resolver, startup creation of mutable runtime directories, diagnostics that show the current runtime contract, `.gitignore` cleanup for mutable data, and a dedicated runtime-boundary policy document.

This is a production milestone.
It is not a gameplay-expansion milestone.

## Scope Delivered

- runtime boundary reporting via `RuntimePaths`
- source-tree versus packaged runtime-root resolution
- runtime directory creation for config, logs, saves, and crash dumps
- debug and startup visibility for repo root, asset root, and runtime root
- `.gitignore` cleanup for runtime-only mutable data
- dedicated documentation for repo boundary rules
- README update reflecting the current milestone state

## Files Updated Or Added

- `.gitignore`
- `CMakeLists.txt`
- `WAR.vcxproj`
- `README.md`
- `Docs/Wrath and Ruin - Runtime Boundary Contract.md`
- `src/engine/core/RuntimePaths.h`
- `src/engine/core/RuntimePaths.cpp`
- `src/game/GameLayer.h`
- `src/game/GameLayer.cpp`
- `src/engine/render/DebugOverlayRenderer.h`
- `src/engine/render/DebugOverlayRenderer.cpp`
- `src/engine/render/BgfxWorldRenderer.h`
- `src/engine/render/BgfxWorldRenderer.cpp`
- `Runtime/Config/.gitkeep`
- `Runtime/Logs/.gitkeep`
- `Runtime/Saves/.gitkeep`
- `Runtime/CrashDumps/.gitkeep`

## Runtime Contract Implemented

### Canonical source-controlled truth

- code in `src/`
- versioned assets in `assets/`
- production docs in `Docs/`
- milestone docs in `Milestones/`
- versioned build/project files at repo root

### Mutable runtime-only state

- `Runtime/Config/`
- `Runtime/Logs/`
- `Runtime/Saves/`
- `Runtime/CrashDumps/`

### Layout behavior

- source-tree execution resolves runtime state into repo `Runtime/`
- packaged execution resolves runtime state into executable-local `runtime/`

## What Was Built

- a runtime boundary report object
- a runtime directory bootstrap path
- updated startup diagnostics
- repo hygiene and ignore-rule updates
- a written policy contract for contributors

## What Was Verified

- file package completeness
- runtime-path design consistency against the documented M31 scope
- milestone and README documentation alignment with the new boundary rules

## What Remains Unverified

This package was prepared without a Windows build-and-run environment in the container.

The following still need validation in the actual repo environment:

- Visual Studio compile and launch
- CMake configure/build
- startup creation of runtime directories under source-tree execution
- packaged-path fallback behavior when repo markers are absent
- bgfx and GDI diagnostics showing the new runtime contract data

## Dependency Impact

M32 can now build on an explicit runtime contract instead of guessing where mutable outputs belong.

That reduces risk for:

- local packaging baseline
- demo-lane reproducibility
- startup diagnostics
- later persistence and hosted runtime work
