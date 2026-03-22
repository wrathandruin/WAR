# M48 - Crash Capture / Failure Bundles / Operator Triage Baseline

## Title

M48 - Crash Capture / Failure Bundles / Operator Triage Baseline

## Description

M48 hardens the internal alpha around operational failure handling.

The milestone backbone is:

- capture startup/runtime failure evidence cleanly
- write crash/failure bundles operators can actually use
- make packaged failure states diagnosable
- improve host/client/bootstrap failure tracing
- keep M45, M46, and M47 validation green

## Included through this batch set

- failure bundle runtime contract
- failure-bundle ownership model
- runtime bundle directory preparation
- startup diagnostics visibility for the failure-bundle lane
- host / client / bootstrap failure bundle capture
- failure-bundle manifests with copied runtime evidence attachments
- operator-facing triage summary artifacts
- packaged operator triage helper surfaces
- M48 validation scripts
- packaged M48 validation entrypoint

## Validation lane

M48 validation scripts are:

- `scripts/validate_m48_failure_bundle_capture_win64.bat`
- `scripts/validate_m48_operator_triage_artifacts_win64.bat`
- `scripts/validate_m48_internal_alpha_package_win64.bat`

These validate that:

- packaged fail-fast paths emit structured failure bundles
- bundle manifests and attachment manifests are written
- operator-triage artifacts are promoted from the latest failure
- the packaged operator-triage helper surface exists
- M45, M46, and M47 validation lanes remain runnable from the packaged lane

## Remaining M48 backbone after this batch

- Windows validation execution
- final candidate/sign-off pass after real runs

## Why this is important

M48 is where packaged failures stop being guesswork and start becoming structured operator evidence.

## What should be coming up next

M48 Final Candidate Pass - run the lane, fix surfaced issues, then classify honestly.
