# Wrath and Ruin

Wrath and Ruin is a Windows-first, server-authoritative, top-down hard-sci-fi RPG that is being built as a narrow vertical slice before it grows into a broader sandbox.

## Start Here

- product architecture: `Docs/Planning/Wrath and Ruin - Alpha Vertical Slice Architecture and Milestone Blueprint.md`
- exact execution packs: `Docs/Planning/Wrath and Ruin - 0.1 Slice Execution Packages and Asset Schedule.md`
- strategic roadmap: `Milestones/WAR_Strategic_Roadmap.md`
- delivery rules: `Docs/Standards/Wrath and Ruin - Milestone Delivery Standard.md`
- lead developer prompt: `Docs/Prompts/Wrath and Ruin - 0.1.1 Lead Developer Start Prompt.md`

## Root Layout

The repo root is now organized around product ownership instead of ad hoc milestone clutter.

- `desktop/`, `server/`, `shared/`, `src/`: active runtime and gameplay code
- `contracts/`: protocol, persistence, and content contracts
- `assets/`: runtime-facing shaders and local texture workspace
- `Docs/`: planning, art, technical, reports, prompts, standards, validation
- `Milestones/`: milestone definitions and milestone validation wrappers
- `scripts/`: Windows build, stage, smoke, and validation scripts
- `Environment/`: deployable environment profiles and config inputs
- `Runtime/`: mutable runtime state roots
- `SourceManifests/`: consolidated source-of-truth package and ops manifests
- `Governance/`: governance and owner-only material boundaries
- `out/`, `bin/`, `obj/`, `Debug/`: generated outputs

## Source Manifest Rule

Repo source manifests now live under `SourceManifests/`.

Staged build packages still preserve the historical package layout:

- `BetaContent/`
- `Launcher/`
- `Installer/`
- `Onboarding/`
- `LiveOps/`
- `ReleaseManagement/`
- `Support/`
- `IncidentResponse/`

This keeps the repo cleaner without breaking packaged validation or operator expectations.

## Current Production Direction

The active planning target is the `0.1.x` vertical-slice runway toward the first internal alpha candidate.

The immediate operating rules are:

- keep the build green
- keep localhost and packaged validation truthful
- deliver complete milestone handoff packages
- no partial code deliveries
- no widening scope before the current slice is truly complete
