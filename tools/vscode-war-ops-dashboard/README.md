# WAR Ops: Dashboard

This is the VS Code operations dashboard for the `WAR` repo.

It keeps the compact shell from the earlier dashboard, but now tracks the package and validation lanes that matter through `M50`.

## What It Covers

- sticky status strip at the top of the view
- current staged package status across internal-alpha, beta-candidate, market-candidate, and local-demo lanes
- latest packaged smoke, M45 bootstrap, M46 baseline, M47 session, M48 triage, M49 beta, and M50 market validation status
- runtime artifact presence for host snapshot, host status, client status, and save files
- build and package actions for internal-alpha, beta-candidate, and market-candidate lanes
- shader rebuild buttons
- host/client launch actions
- M45 through M50 validation wrappers
- quick links to the current roadmap, milestone docs, and validation evidence

## Install

```bash
bash ./scripts/install_vscode_ops_dashboard.sh
```

Then reload VS Code.

## Use

Open the `WAR Ops` activity-bar view or run:

- `WAR Ops: Dashboard: Open Dashboard`
- `WAR Ops: Dashboard: Open Output`

The action registry lives in:

- [registry.js](./registry.js)

The panel is intended to be the fastest way to reach the repo-owned build, launch, smoke, and milestone validation lanes without bouncing between scripts and docs.
