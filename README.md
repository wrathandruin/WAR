# WAR — M45 Closeout Batch 03

> Current milestone: M45 — Internal Alpha Package / Hosted Deploy / Telemetry Baseline

This batch continues the M45 correction lane by closing the client-facing telemetry and presentation surfaces that still needed to line up with the hosted-bootstrap truth work.

This batch focuses on:
- client identity and connect-state telemetry
- visible connect success / failure / disconnect state
- room-entry description runtime
- prompt / vitals presentation
- typed command-bar shell
- validation checks that assert those surfaces in localhost fallback and hosted-bootstrap runs

Hosted truth remains honest:
- `file-backed-localhost-fallback`
- `file-backed-hosted-bootstrap`

The hosted lane in M45 is still an interim file-backed hosted bootstrap.
