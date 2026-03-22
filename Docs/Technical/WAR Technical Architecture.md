# WAR Technical Architecture

## Canonical Repo Layout

The repo root should stay small and stable.

Primary folders:

- `Assets/`: runtime-facing art, shaders, and texture sources
- `Bin64/`: built binaries and staged build outputs
- `Desktop/`: desktop application shell and desktop-only sources
- `Docs/`: concise planning, technical, and art documentation
- `Project/`: scripts, environment profiles, runtime roots, governance, source manifests
- `Server/`: dedicated host shell and server-only sources
- `Tools/`: developer tooling and editor integrations
- `WarEngine/`: shared engine, gameplay, rendering, runtime, world, and platform code
- `third_party/`: external dependencies

## Source Ownership

`WarEngine/` owns:

- engine systems
- gameplay rules
- rendering
- runtime boundaries
- world simulation
- shared data types
- player-facing game logic shared by desktop and server

`Desktop/` owns:

- app entry
- desktop shell
- desktop-only diagnostics and startup wrappers

`Server/` owns:

- host entry
- dedicated-host shell
- host-only bootstrap glue

## Build Rules

- Visual Studio solution is the primary Windows build path.
- Repo-owned automation lives under `Project/scripts/`.
- Binaries publish to `Bin64/`.
- Intermediate files may remain under `obj/` until a later cleanup pass.
- Repo source assets come from `Assets/`.
- Packaged builds may still stage runtime-facing `assets/` for compatibility.

## Runtime Rules

Source-tree runtime surfaces live under `Project/`:

- `Project/Environment/`
- `Project/Runtime/`
- `Project/SourceManifests/`
- `Project/Governance/`

Operational rule:

- deployable configuration belongs under `Project/Environment`
- mutable runtime state belongs under `Project/Runtime`
- package source manifests belong under `Project/SourceManifests`
- confidential owner governance material belongs under `Project/Governance`

## Packaging Rules

Every packaging script must:

1. resolve repo root from `Project/scripts`
2. read binaries from `Bin64`
3. read source assets from `Assets`
4. stage runtime data using the compatibility package layout expected by the executable
5. include concise docs only

## Documentation Rules

Keep documentation intentionally small.

Allowed working set:

- one product blueprint
- one alpha handoff
- one technical architecture doc
- one art direction and asset target doc
- one short docs index

If information is not important enough to live in one of those files, it should not become a standalone document.
