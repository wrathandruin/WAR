# M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline

## Title

M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline

## Description

M45 packages the current slice into an internally hostable alpha candidate with enough runtime visibility for disciplined internal testing.

This milestone is complete only when:
- package truth is honest
- localhost fallback remains protected
- the hosted lane is described honestly for this stage
- telemetry makes failures diagnosable
- validation is explicit
- docs match the actual code and scripts
- room text, prompt direction, and typed entry are visible in the packaged lane

## Included

- internal-alpha packaging lane
- protected localhost fallback lane
- interim hosted bootstrap lane
- explicit connect target model for this stage
- persisted build identity
- persisted host instance identity
- persisted session identity
- persisted client instance identity
- persisted client session identity
- persisted transport kind
- persisted connect target identity
- persisted protocol version visibility
- persisted connect success / failure / disconnect visibility
- persisted restore visibility
- room descriptions
- prompt / vitals direction
- typed command shell
- M45 validation scripts

## Hosted truth for M45

The hosted lane delivered here is an **interim hosted bootstrap**.

It is still file-backed for this stage.
It is not final network transport.

The required explicit distinction is:

- `file-backed-localhost-fallback`
- `file-backed-hosted-bootstrap`

That distinction must exist in:
- launch scripts
- diagnostics
- host status
- client status
- validation scripts
- milestone documentation

## Validation lane

M45 validation scripts are:

- `scripts/smoke_test_headless_host_win64.bat`
- `scripts/smoke_test_local_demo_win64.bat`
- `scripts/validate_m45_hosted_bootstrap_win64.bat`
- `scripts/validate_m45_internal_alpha_package_win64.bat`

## Why this is important

M45 is where the slice stops pretending localhost packaging is product truth and starts behaving like an internal-alpha candidate with explicit hosted-bootstrap identity, diagnosable telemetry, and first product-facing graphical-MUD text surfaces.

## What should be coming up next

M46 - Trust Boundary / Environment Config / Secrets Baseline
