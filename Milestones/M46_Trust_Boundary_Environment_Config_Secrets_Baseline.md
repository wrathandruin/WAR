# M46 - Trust Boundary / Environment Config / Secrets Baseline

## Title

M46 - Trust Boundary / Environment Config / Secrets Baseline

## Description

M46 hardens the internal alpha from an operations and runtime-hygiene standpoint.

The milestone backbone is:

- trust boundary
- environment config separation
- secrets handling baseline
- runtime-root and save-path hygiene
- fail-fast validation
- truthful diagnostics
- real validation scripts

## Included

Current batch coverage:

- environment identity foundation
- deployable environment profile root
- mutable runtime config separation
- environment profile loading
- required secret declaration
- runtime secret file resolution
- fail-fast startup for missing required config/secrets
- secret-safe diagnostics
- runtime-owned directory ownership analysis
- deployable environment separation checks
- save-slot sanitization
- owned save-path resolution
- fail-fast startup for invalid runtime/save ownership
- M46 validation scripts
- M45 validation compatibility updates

Remaining M46 backbone still to close:

- Windows validation execution
- candidate/sign-off truth pass after real runs

## Validation lane

M46 validation scripts are:

- `scripts/validate_m46_environment_identity_win64.bat`
- `scripts/validate_m46_missing_required_secrets_win64.bat`
- `scripts/validate_m46_runtime_save_hygiene_win64.bat`
- `scripts/validate_m46_internal_alpha_package_win64.bat`

These validate that:

- environment identity is visible
- config identity is visible
- missing required secret/config inputs fail fast
- secret values are not exposed
- runtime-root and save-path ownership are sane
- M45 localhost fallback still passes
- M45 hosted bootstrap still passes under the M46 secret baseline

## Runtime ownership model

Runtime-owned writable state must remain under the active runtime root:

- `Runtime/Config`
- `Runtime/Logs`
- `Runtime/Saves`
- `Runtime/CrashDumps`
- `Runtime/Host`

Deployable environment configuration must remain separate:

- `Environment/Profiles/<profile>/environment.cfg`

Save paths are resolved from a sanitized slot name and must remain under:

- `Runtime/Saves`

## Why this is important

M46 is where the slice stops treating deployment and mutable runtime state as an informal habit and starts enforcing an operational boundary the team can trust.

## What should be coming up next

M46 Final Candidate Pass - run the lane, fix surfaced issues, then classify honestly.
