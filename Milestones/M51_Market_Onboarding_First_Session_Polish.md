# M51 - Market Onboarding / First-Session Polish

## Title

M51 - Market Onboarding / First-Session Polish

## Description

M51 should move the market candidate from distribution-ready packaging into a more credible first-use experience.

The focus is:

- market onboarding
- first-session polish
- preservation of the production discipline established in M45 through M50

## Download

Planned package name:

`WAR_M51_code_package.zip`

The milestone document is included inside the package at:

`Milestones/M51_Market_Onboarding_First_Session_Polish.md`

## Included

- `SourceManifests/Onboarding/m51_market_onboarding_manifest.txt`
- `SourceManifests/Onboarding/m51_first_session_polish_manifest.txt`
- `src/engine/core/MarketOnboardingProtocol.h`
- client onboarding runtime capture
- first-session text guidance surfaces
- `scripts/build_market_onboarding_candidate_package_win64.bat`
- `scripts/open_market_onboarding_folder_win64.bat`

## Market truth for M51

The packaged market-onboarding candidate should now stage:

- a real `SourceManifests/Onboarding/` lane with source manifests
- runtime onboarding summaries and first-session brief files under `runtime/Logs/Onboarding/`
- preserved package discipline from M45 through M50

## What should be seen visually

- a fresh player launch should show clearer onboarding direction
- the text layer should feel more intentional for first-session play
- the typed shell should expose onboarding commands instead of only debug utility
- packaged launches should emit onboarding runtime artifacts for support and review

## Why this is important

Market readiness is not only packaging and updater discipline.
It also requires that the first real player-facing session feels intentional, guided, and supportable.

## What should be coming up in the next milestone

M52 - Live-Ops Tooling / Analytics Hardening / Release Management
