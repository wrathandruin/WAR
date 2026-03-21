# Wrath and Ruin - M42 Validation and Acceptance Checklist

## Required Build Lane

Run on the real Windows/MSBuild lane.

## Package Validation

- `scripts/build_local_demo_package_win64.bat Release`
- confirm staged package root is `out/local_demo/WAR_M42_Release`
- confirm `WAR.exe` and `WARServer.exe` are both staged
- confirm `runtime/Saves/authoritative_world_primary.txt` is produced after host activity

## Runtime Validation

1. launch host
2. launch client
3. confirm heartbeat, snapshot, and client replication status files exist
4. complete the M41 planetary mission chain through `Quarantine Control Terminal`
5. return to the cargo bay and interact with `Responder Shuttle Khepri`
6. interact with `Shuttle Helm Terminal`
7. review overlay/event log/status output

## Must Be Visible

- readable mission objective and phase progression
- mission gate blocking ship boarding before the corridor is restored
- visible ship object and helm interaction point in the cargo bay
- player runtime context switching from planet surface to docked ship after boarding
- ship ownership state changing to player-command after the helm beat
- launch-prep readiness visible without any orbital traversal yet
- host-authoritative runtime still active
- low-drift localhost movement with the latency harness disabled

## Persistence Drill

- complete the mission through ship command claim
- close host cleanly
- restart host
- confirm mission phase, ship boarding state, ship ownership state, and launch-prep readiness restore from authoritative save
- confirm reconnect and ordinary movement remain stable after restart

## Sign-Off Questions

- does the slice progress from planetary mission completion into a real boarding beat?
- is ship boarding intentionally gated and understandable?
- does the player gain clear command ownership of the docked shuttle?
- does ship state persist across restart?
- did the authority lane remain intact?
- did packaging and runtime boundaries stay clean?
