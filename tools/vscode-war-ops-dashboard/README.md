# WAR Ops: Dashboard

This is the VS Code operations dashboard for the `WAR` repo.

It keeps the same compact shell as the older project dashboard, but trims the control surface down to what this repo actually owns today.

## What It Covers

- sticky status strip at the top of the view
- current staged local demo package status
- latest packaged smoke and M40 acceptance report status
- runtime artifact presence for host snapshot, host status, client status, and save files
- build and package actions for the local demo lane
- shader rebuild buttons
- host/client launch actions
- M36 through M40 acceptance wrappers
- quick links to the current roadmap and production docs

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

The panel is intended to be the fastest way to reach the repo-owned build, launch, smoke, and acceptance lanes without bouncing between scripts and docs.
