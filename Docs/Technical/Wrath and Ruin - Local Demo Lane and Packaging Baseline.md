# Wrath and Ruin - Local Demo Lane and Packaging Baseline

## Purpose

This document defines the first repeatable local demo lane for WAR.

M31 established canonical source and runtime ownership.
M32 uses that boundary work to describe how a build becomes a reviewable local demo package with explicit startup diagnostics.

## Goals

- make local demo prep repeatable
- stop relying on tribal memory for staging steps
- ensure startup failures and missing package content are observable quickly
- create a minimal script lane before later hosted packaging and deployment work

## Baseline package shape

A staged local demo package should contain these roots next to `WAR.exe`:

- `assets/`
- `runtime/Config/`
- `runtime/Logs/`
- `runtime/Saves/`
- `runtime/CrashDumps/`
- `launch_local_demo_win64.bat`
- `smoke_test_local_demo_win64.bat`

## Repo-side scripts

The repo should treat these scripts as first-class production assets:

- `scripts/build_local_demo_package_win64.bat`
- `scripts/launch_local_demo_win64.bat`
- `scripts/smoke_test_local_demo_win64.bat`

These are not temporary convenience scripts.
They are the first stable local-demo contract that later packaging, installer, and hosted runbooks can evolve from.

## Operator flow

### 1. Build

Build `WAR` for `Release|x64`.

### 2. Stage

Run:

```text
scripts/build_local_demo_package_win64.bat Release
```

This should stage a demo package under:

```text
out/local_demo/
```

### 3. Launch

Launch the staged package with:

```text
launch_local_demo_win64.bat
```

### 4. Smoke test

Run:

```text
smoke_test_local_demo_win64.bat
```

The smoke test should confirm that the executable, shaders, textures, and runtime directories exist and should write a report into the runtime log area.

## Startup report contract

On boot, WAR should write a startup report to:

```text
Runtime/Logs/local_demo_startup_report.txt
```

For packaged runs, the equivalent log location is:

```text
runtime/Logs/local_demo_startup_report.txt
```

The report should include at minimum:

- build configuration
- build timestamp
- runtime mode
- executable path
- repo root if resolved
- asset root
- runtime root
- config/log/save/crash directories
- local demo script readiness
- packaged-lane readiness
- first runtime or demo packaging issues if any

## Exit criteria for M32

M32 is complete when:

- a local build can be staged into a repeatable demo directory
- startup diagnostics explain whether the build is source-tree or packaged
- demo scripts exist in source control
- a smoke-test starter path exists
- local demo preparation no longer depends on undocumented manual assembly

## What this does not try to solve yet

M32 does not attempt to solve:

- installer productionization
- updater flow
- hosted deployment
- dedicated server packaging
- telemetry pipelines
- alpha onboarding

Those belong to later milestones.
M32 only creates the first disciplined local package and diagnostics baseline.
