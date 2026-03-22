# SourceManifests

This directory is the consolidated source-of-truth home for package and operations manifests that used to live as multiple top-level folders in the repo root.

Source layout:

- `SourceManifests/BetaContent/`
- `SourceManifests/Launcher/`
- `SourceManifests/Installer/`
- `SourceManifests/Onboarding/`
- `SourceManifests/LiveOps/`
- `SourceManifests/ReleaseManagement/`
- `SourceManifests/Support/`
- `SourceManifests/IncidentResponse/`

Important rule:

- source manifests live here in the repo
- staged build packages still preserve the historical top-level package layout such as `BetaContent/`, `Launcher/`, and `Installer/`

That means runtime/package validation stays compatible while the repo root stays cleaner.
