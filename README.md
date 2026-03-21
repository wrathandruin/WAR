# WAR — Milestone 44 (Docking / Landing / Cross-Layer Transition Persistence / Return Loop)

> Current development milestone: M44 — Docking / Landing / Cross-Layer Transition Persistence / Return Loop

## Focus
Turn the M43 orbital route proof into a complete first planet-orbit-planet return loop.

This milestone is intentionally narrow:
- preserve the directed M41 mission chain
- preserve the M42 ship runtime and M43 orbital lane
- add one docking beat, one second-destination landing beat, one surface handoff beat, and one home-dock return beat
- keep authority, persistence, packaging, and localhost stability intact
- stop before M45 hosted-alpha hardening breadth

## What this milestone does
- audits the M41-M43 chain and folds the cleanup into M44 instead of carrying debt forward
- keeps relay-track completion as the handoff into M44 rather than falsely treating it as the final mission end-state
- relocates shipboard interactables with the active dock/landing context so the ship runtime reads more coherently across M42-M44
- adds authoritative Dust Frontier docking, landing, surface relay, return launch, and home-dock restoration beats
- persists mission, ship, orbital, and return-loop state through authoritative snapshots and host save/load
- adds schema-version migration from the prior M43 persistence surface into the M44 docking stage
- surfaces docking, landing-site, and return-route state in overlay, bgfx debug, host status, and client replication status
- stages an M44 local demo package, packaged authoritative smoke lane, M43 regression acceptance wrapper, and M44 return-loop acceptance wrapper

## Manual validation procedure
1. Build/stage with `scripts/build_local_demo_package_win64.bat Release`
2. Change into the staged package root:
   - `cd out\local_demo\WAR_M44_Release`
3. Run the packaged launch scripts from that staged root, not from the repo-level `scripts/` directory:
   - `launch_headless_host_win64.bat`
   - `launch_local_client_against_host_win64.bat`
4. Complete the directed chain through M43:
   - Transit Service Terminal
   - Diagnostic Station
   - Quarantine Access Gate combat beat
   - Quarantine Control Terminal
   - Responder Shuttle Khepri boarding
   - Shuttle Helm Terminal command claim
   - Shuttle Helm Terminal orbital departure
   - Orbital Navigation Console -> Debris Survey Orbit
   - Orbital Navigation Console -> Relay Holding Track
5. Run the packaged M43 regression acceptance wrapper from the staged root:
   - `acceptance_m43_orbital_space_layer_win64.bat`
6. Continue the M44 chain:
   - Orbital Navigation Console -> Dust Frontier Relay Platform
   - Responder Shuttle Khepri -> Dust Frontier Landing Pad disembark
   - Frontier Relay Beacon
   - Responder Shuttle Khepri re-board
   - Shuttle Helm Terminal -> Khepri Dock return
   - Responder Shuttle Khepri -> disembark at Khepri Dock
7. Run the packaged M44 acceptance wrapper from the staged root:
   - `acceptance_m44_return_loop_win64.bat`
8. Confirm:
   - `smoke_test_local_demo_win64.bat` proves the staged client-against-host boot lane instead of a standalone local client fallback boot
   - `acceptance_m43_orbital_space_layer_win64.bat` only passes after the authoritative orbital progression has actually reached relay-track completion state
   - `acceptance_m44_return_loop_win64.bat` only passes after authoritative docking, landing, relay, and home-return continuity have actually completed
   - mission objectives advance through docking, landing, surface handoff, and return
   - ship/helm/nav positions remain coherent with the active dock context
   - `orbital_relay_platform_docked`, `orbital_return_route_authorized`, and `orbital_home_dock_reached` update coherently
   - `player_runtime_context` transitions through `planet-surface`, `aboard-docked-ship`, `orbital-space`, `second-destination-surface`, and back again
   - the return loop completes without authority or persistence regressions
9. Close and restart the host at any intermediate step, then confirm mission/ship/orbital/return state restores from `runtime/Saves/authoritative_world_primary.txt`

## Known limits
- localhost authority lane only
- file-backed transport only
- one narrow docking and landing slice, not a broad flight or planetary-travel sandbox
- one second destination only
- no broad ship roster, orbital combat, or multi-destination routing yet
- this is the M44 return-loop proof, not the M45 hosted-alpha hardening pass

## Why this matters
M44 is the first point where the project proves a complete directed continuity lane instead of a stack of disconnected milestones.

The player can now move from planet-side mission beats into ship runtime, into orbit, into a second destination, and back again with persistent authoritative state.
