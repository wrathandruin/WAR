# Wrath and Ruin - M43 Validation and Acceptance Checklist

## Required Build Lane

Run on the real Windows/MSBuild lane.

## Package Validation

- `scripts/build_local_demo_package_win64.bat Release`
- confirm staged package root is `out/local_demo/WAR_M43_Release`
- confirm `WAR.exe` and `WARServer.exe` are both staged
- confirm `runtime/Saves/authoritative_world_primary.txt` is produced after host activity

## Runtime Validation

1. launch host
2. launch client
3. confirm heartbeat, snapshot, and client replication status files exist
4. complete the directed mission chain through shuttle command claim
5. interact with `Shuttle Helm Terminal` to enter local orbit
6. interact with `Orbital Navigation Console` to reach `Debris Survey Orbit`
7. interact with `Orbital Navigation Console` again to stabilize `Relay Holding Track`
8. review overlay/event log/status output

## Must Be Visible

- objective advances from ship command into orbital navigation
- `player_runtime_context=orbital-space`
- orbital phase
- orbital current node and target node
- orbital rule text
- orbital transfer count and travel ticks remaining
- ship occupancy remains aboard-ship while the ship is undocked
- host-authoritative runtime still active
- mission/ship/orbital state all persist after restart

## Persistence Drill

- complete the M42 chain through shuttle command claim
- enter orbital lane
- begin or complete at least one orbital transfer
- close host cleanly
- restart host
- confirm mission, ship, and orbital state restore from authoritative save
- confirm client reconnect still reports coherent orbital state

## Sign-Off Questions

- does the player move cleanly from ship command claim into local orbit?
- is the orbital route ladder understandable?
- do orbital diagnostics make the current state obvious?
- does persistence keep mission, ship, and orbital continuity together?
- did packaging and authority behavior remain intact?
- is M44 now set up to add docking, landing, and return continuity without reworking M43?
