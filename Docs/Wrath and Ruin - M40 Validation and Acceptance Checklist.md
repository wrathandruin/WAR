# Wrath and Ruin - M40 Validation and Acceptance Checklist

## Required Build Lane

Run on the real Windows/MSBuild lane.

## Package Validation

- `scripts/build_local_demo_package_win64.bat Release`
- confirm staged package root is `out/local_demo/WAR_M40_Release`
- confirm `WAR.exe` and `WARServer.exe` are both staged
- confirm `runtime/Saves/authoritative_world_primary.txt` is produced after host activity

## Runtime Validation

1. launch host
2. launch client
3. confirm heartbeat, snapshot, and client replication status files exist
4. confirm normal movement feels stable with the latency harness disabled
5. review `client_replication_status.txt` and confirm corrections are not climbing continuously during ordinary movement
6. move into an encounter hotspot
7. let at least one six-second combat round resolve
8. review overlay/event log/status output

## Must Be Visible

- combat active state
- encounter label
- combat round number
- hostile health
- player health, armor, and suit integrity
- low-drift movement with no obvious rubber-banding on the localhost lane
- visible correction diagnostics including position drift and path/entity divergence
- inventory/equipment state
- persisted save path
- host-authoritative runtime still active

## Persistence Drill

- trigger encounter
- allow at least one round to resolve
- close host cleanly
- restart host
- confirm actor/combat state restores from authoritative save
- confirm movement remains stable after restart and reconnect

## Sign-Off Questions

- does one encounter resolve end to end?
- is the outcome explained clearly?
- does persistence keep the outcome?
- did the authority lane remain intact?
- did localhost movement remain smooth enough to demo without visible rubber-banding?
- did packaging and runtime boundaries stay clean?
