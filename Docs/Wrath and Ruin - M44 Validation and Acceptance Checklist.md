# Wrath and Ruin - M44 Validation and Acceptance Checklist

## Required Build Lane

Run on the real Windows/MSBuild lane.

## Package Validation

- `scripts/build_local_demo_package_win64.bat Release`
- confirm staged package root is `out/local_demo/WAR_M44_Release`
- confirm `WAR.exe` and `WARServer.exe` are both staged
- confirm `runtime/Saves/authoritative_world_primary.txt` is produced after host activity
- confirm `acceptance_m44_return_loop_win64.bat` is present in the staged root
- confirm `smoke_test_local_demo_win64.bat` is present in the staged root

## Runtime Validation

1. change into `out/local_demo/WAR_M44_Release`
2. launch `launch_headless_host_win64.bat`
3. launch `launch_local_client_against_host_win64.bat`
4. confirm `authority_mode=headless-host`
5. confirm `host_online=yes`
6. complete the M41 chain through `Quarantine Control Terminal`
7. board `Responder Shuttle Khepri`
8. claim command with `Shuttle Helm Terminal`
9. depart through the orbital lane and complete `Debris Survey Orbit` and `Relay Holding Track`
10. dock with `Dust Frontier Relay Platform`
11. disembark to `Dust Frontier Landing Pad`
12. use `Frontier Relay Beacon`
13. re-board the shuttle
14. use `Shuttle Helm Terminal` to return home
15. disembark at Khepri Dock
16. run `acceptance_m44_return_loop_win64.bat`
17. review overlay, event log, host status, and `client_replication_status.txt`

## Must Be Visible

- readable objective text for the active beat
- authoritative headless-host lane still active
- `orbital_relay_platform_docked=yes`
- `orbital_return_route_authorized=yes`
- `orbital_home_dock_reached=yes`
- `mission_complete=yes`
- `mission_phase=mission-complete`
- `player_runtime_context=planet-surface`
- `ship_docked=yes`
- `ship_location=cargo-bay-dock`
- final-state proof that the Dust Frontier landing and relay-secure checkpoint actually occurred before the return route was authorized
- stable localhost movement with no obvious rubber-banding when the latency harness is disabled
- persisted return-loop state after host restart

## Persistence Drill

- stop after Dust Frontier docking and restart host
- confirm docking and mission state restore
- stop again after securing `Frontier Relay Beacon` and restart host
- confirm return-launch authorization restores
- stop again after home docking and restart host
- confirm the player can still disembark and complete the loop cleanly

## Sign-Off Questions

- does relay-track completion hand off cleanly into docking?
- does the second destination feel like a real landing beat instead of a fake flag flip?
- does return-launch authority depend on the surface relay beat as intended?
- does home docking restore a clean disembark beat?
- do the staged packaged acceptance and smoke lanes now require real authoritative return-loop completion rather than key presence only?
- does persistence keep each stage of the loop?
- did the authority lane, packaging lane, and localhost stability all remain intact?
