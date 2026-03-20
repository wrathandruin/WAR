Title

M32 - Local Demo Lane / Packaging / Diagnostics Baseline

Description

M32 creates the first repeatable local demo path for WAR. The milestone turns local demo prep into an explicit package-and-check lane instead of a manually assembled routine.

This delivery builds directly on M31. Runtime boundaries were defined there; M32 uses those boundaries to stage a package, launch it in a predictable layout, emit startup diagnostics, and provide a first smoke-test starter script.

Download

WAR_M32_code_package.zip

The milestone document is included inside the package at:

Milestones/M32_Local_Demo_Lane_Packaging_Diagnostics_Baseline.md

Included

- local demo packaging script
- launch script baseline
- smoke-test starter lane
- startup diagnostics report writer
- README and docs updates for the new operator flow
- build-system updates for the new diagnostics module

What should be seen visually

- startup should report M32 in the event log
- diagnostics should show build configuration, startup report path, and package readiness state
- source-tree runs should clearly show whether repo-side packaging scripts are present
- packaged runs should clearly show whether assets, runtime roots, and launch/smoke scripts were staged correctly
- a local demo package should be stageable into out/local_demo without ad hoc copying

Why this is important

M32 is the production handoff from repo-contract cleanup into repeatable demo discipline.

This milestone reduces ambiguity before M33 and later authority work, because simulation, persistence, and hosted runtime are much safer when the local build-and-demo lane already has structure, scripts, and observable startup state.

What should be coming up in the next milestone

M33 - Shared Simulation Contract / Fixed-Step Ownership
