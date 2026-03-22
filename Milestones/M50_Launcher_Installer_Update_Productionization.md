# M50 - Launcher / Installer / Update Productionization

## Title

M50 - Launcher / Installer / Update Productionization

## Description

M50 should move the project from beta-candidate discipline into the first real market-facing distribution discipline.

The focus is:

- launcher productionization
- installer productionization
- update productionization
- preservation of the validation culture established in M45 through M49

## Download

Planned package name:

`WAR_M50_code_package.zip`

The milestone document is included inside the package at:

`Milestones/M50_Launcher_Installer_Update_Productionization.md`

## Included

- `Launcher/m50_launcher_manifest.txt`
- `Installer/m50_installer_manifest.txt`
- `Installer/m50_update_channel_manifest.txt`
- `src/engine/core/LauncherDistributionProtocol.h`
- client launcher-session runtime capture
- host bootstrap installer and update runtime capture
- `scripts/build_market_candidate_package_win64.bat`
- `scripts/validate_m50_launcher_productionization_win64.bat`
- `scripts/validate_m50_installer_productionization_win64.bat`
- `scripts/validate_m50_update_productionization_win64.bat`
- `scripts/validate_m50_market_candidate_package_win64.bat`
- `scripts/open_market_candidate_folder_win64.bat`

## Validation lane

M50 validation scripts are:

- `scripts/validate_m50_launcher_productionization_win64.bat`
- `scripts/validate_m50_installer_productionization_win64.bat`
- `scripts/validate_m50_update_productionization_win64.bat`
- `scripts/validate_m50_market_candidate_package_win64.bat Release`

The aggregate M50 package validation must preserve the M45 through M49 lanes and then prove:

- launcher session manifests and latest pointers are emitted under `runtime/Logs/Launcher/`
- installer runtime manifests are emitted under `runtime/Logs/Installer/`
- updater runtime manifests are emitted under `runtime/Logs/Updater/`
- the staged package contains `Launcher/` and `Installer/` source-of-truth manifests

## Market truth for M50

The packaged market candidate should now stage:

- a real `Launcher/` lane with product-entry source manifests
- a real `Installer/` lane with install and update source manifests
- preserved beta content and internal runtime surfaces
- a market candidate manifest at `market_candidate_manifest.txt`

## What should be seen visually

- the packaged market candidate should now stage a dedicated `Launcher/` lane
- the packaged market candidate should now stage a dedicated `Installer/` lane
- packaged runs should leave behind launcher, installer, and update runtime manifests
- the package should now read like a real distributed product candidate instead of only a beta content candidate

## Why this is important

Market-candidate credibility requires that launcher entry, installation, and patch/update direction stop being implied and start becoming explicit, reviewable, and testable.

## What should be coming up in the next milestone

M51 - Room And Interior Description Runtime / Location Titles
