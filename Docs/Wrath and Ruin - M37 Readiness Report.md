# Wrath and Ruin - M37 Readiness Report

## Decision

GO for M37.

The repo now has the minimum structural, packaging, and validation base needed to begin persistence work without continuing to harden the old monolithic shape by accident.

## What Was Completed

- separated desktop boot from host boot through `HeadlessHostBootstrap`
- made `WARServer.exe` the preferred dedicated-host executable in launch, smoke, package, and acceptance flows
- kept `WARLegacy` alive as the fallback lane so the split is non-destructive
- moved the first shared implementation batch into `shared/src/`
- revalidated package-local runtime behavior from the staged local demo bundle
- replaced the brittle batch-only M36 acceptance logic with a PowerShell-backed acceptance runner behind the same batch entry point
- corrected the public-repo asset policy so local textures stay on disk but no longer remain tracked in Git

## What Was Moved

The first shared implementation move batch is now building from `shared/src/`:

- core runtime helpers
- localhost authority helpers
- math primitives
- world state and pathfinding
- gameplay systems
- simulation runtime

This is intentionally implementation-first.
The repo still uses the legacy `src/` tree as the transitional include root for many headers, which keeps the split stable while further ownership cleanup continues in M37 and M38.

## Validation

Validated on 2026-03-20 on the real Windows/MSBuild lane:

- `WAR` `Debug|x64`: pass, `0 warnings`, `0 errors`
- `WARShared` `Debug|x64`: pass, `0 warnings`, `0 errors`
- `WARServer` `Debug|x64`: pass, `0 warnings`, `0 errors`
- `WARLegacy` `Debug|x64`: pass, `0 warnings`, `0 errors`
- `scripts/build_local_demo_package_win64.bat Debug`: pass
- staged `smoke_test_headless_host_win64.bat`: pass
- staged `acceptance_m36_localhost_authority_win64.bat`: pass

Packaged acceptance evidence:

- `out/local_demo/WAR_M36_Debug/runtime/Logs/m36_acceptance_report.txt`

## What Remains Intentionally Transitional

- `WARLegacy` remains in the solution as a fallback target
- the legacy `WAR.exe --headless-host` path remains available only for that fallback lane
- most headers still resolve from the transitional `src/` include root
- render, platform, desktop diagnostics, and the server host loop still need their later physical moves

These are acceptable transition seams.
They do not block M37.

## Why This Is Sufficient For M37

M37 needs a stable place for persistence to live.

That place now exists:

- gameplay truth is already being compiled through `WARShared`
- desktop and server boots are distinct
- the staged local demo package exercises `WAR.exe` and `WARServer.exe` directly
- the localhost authority lane still passes after the split work

That means persistence can be added into the correct architecture rather than being layered onto a client-shaped runtime shell.
