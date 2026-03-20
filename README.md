# WAR — Milestone 32 (Local Demo Lane / Packaging / Diagnostics Baseline)

> Current development milestone: M32 — Local Demo Lane / Packaging / Diagnostics Baseline

## Focus
Turn the repo from a manually prepared local prototype into a repeatable local demo lane with package staging, startup reporting, and smoke-test discipline.

M31 established source-versus-runtime ownership.
M32 uses that contract to define how a local demo build should be staged, launched, checked, and diagnosed.

## What this milestone does
- adds `LocalDemoDiagnostics` so startup generates a local demo readiness report in `Runtime/Logs/`
- adds a repo-side packaging script at `scripts/build_local_demo_package_win64.bat`
- adds launch and smoke-test starter scripts for staged demo builds
- surfaces build configuration, startup-report path, packaged-lane readiness, and script availability in runtime diagnostics
- updates the README and dedicated M32 docs so demo prep stops living in memory

## Local demo lane after M32
A disciplined local demo lane now means:

- the build can be staged into `out/local_demo/`
- the staged package uses executable-local `assets/` and `runtime/` roots
- startup writes a concrete report describing runtime and packaging state
- launch and smoke-test scripts exist as first-class repo assets
- diagnostics explain whether the package is really ready or still only a source-tree run

## Intended operator flow
1. Build WAR in `Release|x64`.
2. Run `scripts/build_local_demo_package_win64.bat Release`.
3. Launch the staged package with `launch_local_demo_win64.bat`.
4. Run `smoke_test_local_demo_win64.bat` to verify demo prerequisites and emit a report.

## Why this matters
M32 is where demoability becomes production work instead of habit.

That matters because the roadmap requires a clean local demo lane before shared simulation, authority, persistence, and hosted runtime work expand the support surface.

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
### M33 — Shared Simulation Contract / Fixed-Step Ownership
- define a shared gameplay-state contract between client and future host
- establish fixed-step ownership and simulation cadence boundaries
- stop treating local presentation as the source of gameplay truth
