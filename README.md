# Wrath and Ruin

Wrath and Ruin is a Windows-first, server-authoritative, top-down hard-sci-fi RPG that is being built as a narrow vertical slice before it grows into a broader sandbox.

## Start Here

- product blueprint: `Docs/Planning/WAR Product Blueprint.md`
- alpha handoff: `Docs/Planning/WAR Alpha Handoff.md`
- technical architecture: `Docs/Technical/WAR Technical Architecture.md`
- art targets: `Docs/Art/WAR Art Direction and Asset Targets.md`

## Root Layout

The repo root is organized around stable product ownership.

- `Assets/`: runtime-facing shaders and texture sources
- `Bin64/`: built binaries and staged build outputs
- `Content/`: authored scenario, mission, and validation-support content
- `Desktop/`: desktop shell and desktop-only sources
- `Docs/`: concise planning, technical, and art documentation
- `Project/`: repo automation, source manifests, governance, environment profiles, runtime roots
- `Server/`: dedicated-host shell and server-only sources
- `Tools/`: developer tooling and editor integrations
- `WarEngine/`: shared engine, gameplay, runtime, rendering, and world code
- `obj/`: intermediate build output
- `third_party/`: external dependencies

## Source Manifest Rule

Repo source manifests now live under `Project/SourceManifests/`.

Source-tree operations roots also live under `Project/`:

- `Project/Environment/`
- `Project/Runtime/`
- `Project/Governance/`

Staged build packages may still preserve compatibility package lanes such as:

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

The active target is the internal-alpha vertical slice defined in the product blueprint and alpha handoff.

The immediate operating rules are:

- keep the build green
- keep the repo structure canonical
- keep localhost and packaged validation truthful
- deliver one complete zip per milestone
- no partial code deliveries
- no widening scope before the current slice is truly complete
