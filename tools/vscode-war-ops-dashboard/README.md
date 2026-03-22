# WAR Ops: Dashboard

This is the VS Code operations dashboard for the `WAR` repo.

It is intentionally narrow now: one internal-alpha lane, the core run/smoke actions, and only the two documents the lead dev needs most often.

## What It Covers

- internal-alpha package readiness
- host and client smoke status
- runtime artifact presence for the active stage
- package, validate, launch, and smoke actions
- quick links to the alpha handoff and technical architecture

## Install

```bash
bash ./Project/scripts/install_vscode_ops_dashboard.sh
```

Then reload VS Code.

## Use

Open the `WAR Ops` activity-bar view or run:

- `WAR Ops: Dashboard: Open Dashboard`
- `WAR Ops: Dashboard: Open Output`

The action registry lives in:

- [registry.js](./registry.js)

The panel is intended to be the fastest way to package, validate, launch, smoke-test, and open the two docs that actually matter during development.
