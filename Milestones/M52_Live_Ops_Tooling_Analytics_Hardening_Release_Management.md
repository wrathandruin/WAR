# M52 - Live-Ops Tooling / Analytics Hardening / Release Management

## Title

M52 - Live-Ops Tooling / Analytics Hardening / Release Management

## Description

M52 should move the market-onboarding candidate into a more supportable and operable product posture.

The focus is:

- live-ops tooling
- analytics hardening
- release management
- preservation of the production discipline established in M45 through M51

## Download

Planned package name:

`WAR_M52_code_package.zip`

The milestone document is included inside the package at:

`Milestones/M52_Live_Ops_Tooling_Analytics_Hardening_Release_Management.md`

## Included

- `LiveOps/m52_live_ops_tooling_manifest.txt`
- `LiveOps/m52_analytics_hardening_manifest.txt`
- `ReleaseManagement/m52_release_management_manifest.txt`
- `src/engine/core/LiveOpsProtocol.h`
- `src/engine/core/ReleaseManagementProtocol.h`
- client live-ops runtime capture
- host bootstrap live-ops and release runtime capture
- `scripts/build_market_ops_candidate_package_win64.bat`
- `scripts/open_market_ops_candidate_folder_win64.bat`

## Market truth for M52

The packaged market-ops candidate should now stage:

- a real `LiveOps/` lane with source manifests
- a real `ReleaseManagement/` lane with source manifests
- runtime live-ops summaries and analytics counters under `runtime/Logs/LiveOps/`
- runtime release state and gate summaries under `runtime/Logs/ReleaseManagement/`
- preserved package discipline from M45 through M51

## What should be seen visually

- packaged launches should emit live-ops operator-facing runtime summaries
- packaged launches should emit analytics counters and session summaries
- packaged host/bootstrap runs should emit release gate and release-state artifacts
- the product should look more supportable and more release-managed than a pure onboarding candidate

## Why this is important

Market readiness requires not only onboarding polish, but also the ability to operate the product, trust its runtime counts, and carry forward release discipline as a first-class concern.

## What should be coming up in the next milestone

M53 - Support Workflow / Player Messaging / Incident Response Baseline
