# Wrath and Ruin - M44 Validation and Acceptance Checklist

## Required Build Lane

Run on the real Windows/MSBuild lane.

## Package Validation

- `scripts/build_local_demo_package_win64.bat Release`
- confirm staged package root is `out/local_demo/WAR_M44_Release`
- confirm `WAR.exe` and `WARServer.exe` are both staged
- confirm `runtime/Saves/authoritative_world_primary.txt` is produced after host activity
- confirm `acceptance_m44_return_loop_win64.bat` is present in the staged root

## Runtime Validation

1. launch host
2. launch client
3. complete the M41 chain through `Quarantine Control Terminal`
4. board `Responder Shuttle Khepri`
5. claim command with `Shuttle Helm Terminal`
6. depart through the orbital lane and complete `Debris Survey Orbit` and `Relay Holding Track`
7. dock with `Dust Frontier Relay Platform`
8. disembark to `Dust Frontier Landing Pad`
9. use `Frontier Relay Beacon`
10. re-board the shuttle
11. use `Shuttle Helm Terminal` to return home
12. disembark at Khepri Dock
13. review overlay, event log, host status, and `client_replication_status.txt`

## Must Be Visible

- readable objective text for the active beat
- mission phase and gate state
- Dust Frontier docking state
- frontier surface active state
- home-dock restoration state
- player runtime context changing coherently across the return loop
- ship location and orbital node continuity
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
- does persistence keep each stage of the loop?
- did the authority lane, packaging lane, and localhost stability all remain intact?
