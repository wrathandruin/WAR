# M49 - Beta Content Scale / Retention Expansion / Release-Candidate Discipline

## Title

M49 - Beta Content Scale / Retention Expansion / Release-Candidate Discipline

## Description

M49 is the next packaging and validation step after the M48 internal-alpha hardening lane.

This milestone moves the slice from alpha-hardening posture into a disciplined beta-candidate posture around three linked concerns:

- beta content scale
- retention expansion
- release-candidate discipline

## Download

Planned package name:

`WAR_M49_code_package.zip`

The milestone document is included inside the package at:

`Milestones/M49_Beta_Content_Scale_Retention_Expansion_Release_Candidate_Discipline.md`

## Included

- `BetaContent/m49_beta_content_manifest.txt`
- `BetaContent/m49_retention_journey_cards.txt`
- `src/engine/core/BetaRetentionProtocol.h`
- `src/engine/core/ReleaseCandidateProtocol.h`
- client launch retention and release-candidate capture
- host bootstrap retention and release-candidate capture
- `scripts/build_beta_release_candidate_package_win64.bat`
- `scripts/validate_m49_beta_content_scale_win64.bat`
- `scripts/validate_m49_retention_expansion_win64.bat`
- `scripts/validate_m49_release_candidate_discipline_win64.bat`
- `scripts/validate_m49_beta_candidate_package_win64.bat`
- `scripts/open_beta_release_candidate_folder_win64.bat`

## Validation lane

M49 validation scripts are:

- `scripts/validate_m49_beta_content_scale_win64.bat`
- `scripts/validate_m49_retention_expansion_win64.bat`
- `scripts/validate_m49_release_candidate_discipline_win64.bat`
- `scripts/validate_m49_beta_candidate_package_win64.bat Release`

The aggregate M49 package validation must preserve the M45 through M48 packaged lanes and then prove:

- beta content manifests are staged inside `BetaContent/`
- retention summaries and launch journals are emitted under `runtime/Logs/Retention/`
- release-candidate manifest, checklist, and latest pointer are emitted under `runtime/Logs/ReleaseCandidate/`

## Beta truth for M49

The packaged beta candidate should now stage:

- a real `BetaContent/` directory with authored content manifests
- preserved internal-alpha runtime surfaces under `runtime/`
- preserved M45 through M48 validation scripts alongside the new M49 validators
- a beta candidate manifest at `beta_release_candidate_manifest.txt`

## What should be seen visually

- packaged beta-candidate content manifests should now exist and stage cleanly
- packaged launches should leave behind retention summaries and launch journals
- packaged host/bootstrap runs should leave behind release-candidate manifests and checklists
- the beta package should look more like a product candidate than only an internal-alpha bundle

## Why this is important

Beta must expand content credibility and retention visibility without losing the production discipline earned in alpha.

M49 is where the project starts treating repeated tester return, candidate packaging, and content breadth presentation as first-class product work.

## What should be coming up in the next milestone

M50 - Launcher / Installer / Update Productionization
