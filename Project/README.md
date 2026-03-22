# Project

This umbrella keeps repo-level operational and planning support material out of the root.

Current subdirectories:

- `Project/Environment/`: deployable source-tree environment profiles and templates
- `Project/Runtime/`: mutable source-tree runtime roots and tracked layout placeholders
- `Project/SourceManifests/`: source-of-truth package and ops manifests
- `Project/Governance/`: governance boundaries and owner-only private workspace rules

Packaged builds still stage their historical top-level runtime and lane folders such as `Environment/`, `Runtime/`, `BetaContent/`, and `Installer/`.
