# Wrath and Ruin - M41 Validation and Acceptance Checklist

## Required Build Lane

Run on the real Windows/MSBuild lane.

## Package Validation

- `scripts/build_local_demo_package_win64.bat Release`
- confirm staged package root is `out/local_demo/WAR_M41_Release`
- confirm `WAR.exe` and `WARServer.exe` are both staged
- confirm `runtime/Saves/authoritative_world_primary.txt` is produced after host activity
- confirm the staged package includes the M41 acceptance wrapper and the M40 regression acceptance wrapper

## Runtime Validation

1. launch host
2. launch client
3. confirm heartbeat, snapshot, and client replication status files exist
4. confirm normal movement still feels stable with the latency harness disabled
5. confirm the initial objective is visible
6. interact with `Transit Service Terminal`
7. inspect `Diagnostic Station`
8. confirm the quarantine gate is now unlocked
9. move into `Quarantine Access Gate`
10. let the required six-second combat beat resolve
11. interact with `Quarantine Control Terminal`
12. review overlay, event log, and status output

## Must Be Visible

- readable mission objective text
- readable mission phase text
- mission gate state
- ship handoff-ready state
- combat active state during the gate encounter
- combat label
- hostile health
- player health, armor, and suit integrity
- low-drift movement with no obvious rubber-banding on the localhost lane
- visible correction diagnostics including position drift and path/entity divergence
- inventory/equipment state
- persisted save path
- host-authoritative runtime still active

## Persistence Drill

- advance the mission through at least the MedLab diagnostic beat
- unlock the quarantine route
- resolve the Quarantine Access Gate combat beat
- close host cleanly
- restart host
- confirm mission phase, gate state, actor state, and combat outcome restore from authoritative save
- confirm movement remains stable after restart and reconnect

## Sign-Off Questions

- does the player have a readable objective at boot?
- does progression feel intentionally gated instead of sandbox-random?
- does at least one interaction or inspect beat advance the mission?
- does the combat beat materially advance progression?
- does mission state persist across restart?
- did the authority lane remain intact?
- did localhost movement remain smooth enough to demo without visible rubber-banding?
- did packaging and runtime boundaries stay clean?
- is M42 now clearly unblocked without ship runtime already bloating M41?
