# Wrath and Ruin - M43 Validation and Acceptance Checklist

## Required Build Lane

Run on the real Windows/MSBuild lane.

## Package Validation

- `scripts/build_local_demo_package_win64.bat Release`
- confirm staged package root is `out/local_demo/WAR_M44_Release`
- confirm `WAR.exe` and `WARServer.exe` are both staged
- confirm `runtime/Saves/authoritative_world_primary.txt` is produced after host activity
- confirm the staged package includes `acceptance_m43_orbital_space_layer_win64.bat`
- confirm the staged package includes `smoke_test_local_demo_win64.bat`

## Runtime Validation

1. change into `out/local_demo/WAR_M44_Release`
2. launch `launch_headless_host_win64.bat`
3. launch `launch_local_client_against_host_win64.bat`
4. confirm heartbeat, snapshot, and client replication status files exist
5. confirm `authority_mode=headless-host`
6. confirm `host_online=yes`
7. complete the directed mission chain through shuttle command claim
8. interact with `Shuttle Helm Terminal` to enter local orbit
9. interact with `Orbital Navigation Console` to reach `Debris Survey Orbit`
10. interact with `Orbital Navigation Console` again to stabilize `Relay Holding Track`
11. run `acceptance_m43_orbital_space_layer_win64.bat`
12. review overlay, event log, host status, and `client_replication_status.txt`

## Must Be Visible

- objective advances from ship command into orbital navigation
- `authority_mode=headless-host`
- `host_online=yes`
- `ship_command_claimed=yes`
- `player_runtime_context=orbital-space`
- `orbital_survey_orbit_reached=yes`
- `orbital_relay_track_reached=yes`
- `mission_phase=dock-relay-platform`
- `orbital_phase=relay-holding`
- `orbital_current_node=relay-holding-track`
- host-authoritative runtime still active
- mission, ship, and orbital state all persist after restart

## Persistence Drill

- complete the M42 chain through shuttle command claim
- enter orbital lane
- complete the survey orbit transfer and relay-track stabilization beat
- close host cleanly
- restart host
- confirm mission, ship, and orbital state restore from authoritative save
- confirm client reconnect still reports coherent orbital state

## Sign-Off Questions

- does the player move cleanly from ship command claim into local orbit?
- does the orbital route ladder remain understandable?
- do the staged packaged acceptance and smoke lanes now require real authoritative orbital progression rather than key presence only?
- does persistence keep mission, ship, and orbital continuity together?
- did packaging and authority behavior remain intact?
- is M44 now set up to add docking, landing, and return continuity without reworking M43?
