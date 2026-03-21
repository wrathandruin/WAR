# Wrath and Ruin - M40-M44 Sign-Off Report

Date: 2026-03-21

## Scope

This review covers the delivered M40 through M44 chain:

- M40 - Six-Second Combat / Encounter Resolution
- M41 - Planetary Mission Slice / Objectives / Gate Logic
- M42 - Ship Runtime / Boarding / Ownership State
- M43 - Orbital Space Layer / Travel State / Navigation Rules
- M44 - Docking / Landing / Cross-Layer Transition Persistence / Return Loop

The purpose of this report is to determine whether the current working tree is ready for formal milestone sign-off and handoff into the post-M44 roadmap section.

## Evidence Reviewed

### Milestone and checklist documents

- `Milestones/M40_Six_Second_Combat_Encounter_Resolution.md`
- `Milestones/M41_Planetary_Mission_Slice_Objectives_Gate_Logic.md`
- `Milestones/M42_Ship_Runtime_Boarding_Ownership_State.md`
- `Milestones/M43_Orbital_Space_Layer_Travel_State_Navigation_Rules.md`
- `Milestones/M44_Docking_Landing_Cross_Layer_Transition_Persistence_Return_Loop.md`
- `Docs/Wrath and Ruin - M40 Validation and Acceptance Checklist.md`
- `Docs/Wrath and Ruin - M41 Validation and Acceptance Checklist.md`
- `Docs/Wrath and Ruin - M42 Validation and Acceptance Checklist.md`
- `Docs/Wrath and Ruin - M43 Validation and Acceptance Checklist.md`
- `Docs/Wrath and Ruin - M44 Validation and Acceptance Checklist.md`

### Runtime and build surfaces reviewed

- `shared/src/simulation/SimulationRuntime.cpp`
- `shared/src/runtime/host/AuthoritativeHostProtocol.cpp`
- `shared/src/runtime/host/HeadlessHostPresence.cpp`
- `src/game/GameLayer.cpp`
- `src/engine/core/Application.cpp`
- `scripts/build_local_demo_package_win64.bat`
- `scripts/smoke_test_headless_host_win64.bat`
- `scripts/smoke_test_local_demo_win64.bat`
- `scripts/acceptance_m43_orbital_space_layer_win64.ps1`
- `scripts/acceptance_m44_return_loop_win64.ps1`

### Windows validation commands executed

- `cmd.exe /C "scripts\build_local_demo_package_win64.bat" Release`
- `cmd.exe /C "out\local_demo\WAR_M44_Release\smoke_test_headless_host_win64.bat"`
- `cmd.exe /C "out\local_demo\WAR_M44_Release\smoke_test_local_demo_win64.bat"`
- `cmd.exe /C "out\local_demo\WAR_M44_Release\acceptance_m43_orbital_space_layer_win64.bat"`
- `cmd.exe /C "out\local_demo\WAR_M44_Release\acceptance_m44_return_loop_win64.bat"`

### Runtime artifacts reviewed

- `out/local_demo/WAR_M44_Release/runtime/Logs/headless_host_smoke_test.txt`
- `out/local_demo/WAR_M44_Release/runtime/Logs/local_demo_smoke_test.txt`
- `out/local_demo/WAR_M44_Release/runtime/Logs/m43_orbital_space_acceptance_report.txt`
- `out/local_demo/WAR_M44_Release/runtime/Logs/m44_return_loop_acceptance_report.txt`
- `out/local_demo/WAR_M44_Release/runtime/Logs/client_replication_status.txt`
- `out/local_demo/WAR_M44_Release/runtime/Host/headless_host_status.txt`
- `out/local_demo/WAR_M44_Release/runtime/Logs/headless_host_log.txt`
- `out/local_demo/WAR_M44_Release/runtime/Saves/authoritative_world_primary.txt`

## Summary Decision

Formal full sign-off for the complete M40-M44 chain is not granted yet.

The current tree is in a strong engineering state:

- the Windows package lane is green
- the split client/server/shared build is green
- the staged host smoke test passes
- the staged client smoke test passes
- the staged M43 and M44 acceptance wrappers pass
- the core implementation for mission progression, ship runtime, orbital routing, docking, landing, and return-loop persistence is present and coherent in code

However, the sign-off lane is still too weak to justify a hard milestone closure without caveat.

The main issue is not that the milestone chain looks broken.
The issue is that the current staged acceptance surface does not prove the full authoritative gameplay walkthrough it claims to represent.

Current verdict:

- M40: remains signed off and stable
- M41-M42: implementation looks coherent and no blocking regression was found in this audit
- M43-M44: buildable and packageable, but not yet strongly enough validated for a final formal sign-off
- Overall: CONDITIONAL GO, not HARD GO

## What Passed

### Build and packaging

The current working tree successfully built and staged the M44 package on the real Windows/MSBuild lane.

Observed result:

- `WAR.sln` built successfully
- `WAR.exe` and `WARServer.exe` were staged into `out/local_demo/WAR_M44_Release`
- build completed with `0 Warning(s)` and `0 Error(s)`

### Packaged smoke lane

The packaged smoke lane is green.

Observed result:

- `smoke_test_headless_host_win64.bat`: PASS
- `smoke_test_local_demo_win64.bat`: PASS

This confirms that the staged host and staged client both boot successfully and emit their required runtime artifacts.

### Current implementation shape

The milestone chain is actually implemented in the runtime, not merely described in docs.

Observed in `shared/src/simulation/SimulationRuntime.cpp`:

- mission objective ladder extends from transit recovery through Dust Frontier return
- docking transfer is implemented
- Dust Frontier disembark is implemented
- relay-beacon secure beat is implemented
- homeward launch is implemented
- Khepri Dock return and final disembark are implemented

Observed in persistence surfaces:

- ship runtime state is serialized
- orbital runtime state is serialized
- mission return-loop state is serialized
- schema migration paths now account for M44 state additions

## Findings

### High

- The current M43 and M44 acceptance automation can pass without proving the actual milestone chain on the authoritative client/host lane.
  - In `scripts/acceptance_m43_orbital_space_layer_win64.ps1`, the scripted pass condition only checks that key names exist in status output and save files. It does not require that the player has actually reached orbit, completed a transfer, or stabilized Relay Holding Track.
  - In `scripts/acceptance_m44_return_loop_win64.ps1`, the scripted pass condition only checks that M44-related keys exist. It does not require docking, landing, relay security, return launch, or home docking to have actually happened.
  - The generated staged reports passed while the staged client status still showed the untouched starting state:
    - `authority_mode=local`
    - `host_online=no`
    - `mission_phase=recover-transit-data`
    - `mission_advancement_count=0`
    - `ship_boarded=no`
    - `orbital_relay_platform_docked=no`
    - `orbital_home_dock_reached=no`
  - This means the automation is currently validating telemetry surface availability, not milestone completion.

- The current staged client smoke lane does not validate the real authoritative gameplay lane.
  - `scripts/smoke_test_local_demo_win64.bat` launches `launch_local_demo_win64.bat`, not the combined host/client flow.
  - The resulting staged client status file from this audit shows `authority_mode=local` and `host_online=no`.
  - As a result, the current smoke lane cannot catch regressions where the packaged client no longer behaves correctly against the packaged headless host.

### Medium

- The README validation instructions currently point operators at repo-level launch scripts instead of the staged package root.
  - `README.md` instructs operators to run `scripts/launch_headless_host_win64.bat` and `scripts/launch_local_client_against_host_win64.bat`.
  - Those scripts resolve their executable path relative to the script location and fail if `WAR.exe` is not adjacent or one directory above.
  - In the repo root this is not true; those scripts are meant to be run from the staged package copy.
  - This creates a documentation mismatch in the main manual validation path.

- The current M44 acceptance script is lenient on missing host keys.
  - `scripts/acceptance_m44_return_loop_win64.ps1` treats missing host status fields as warnings rather than failures.
  - That weakens sign-off confidence for a milestone whose core promise is cross-layer continuity on the authoritative lane.

### Low

- M40 branding still remains in visible runtime and launch surfaces.
  - `src/engine/core/Application.cpp` still creates the main window as `WAR - Milestone 40`.
  - `scripts/launch_headless_host_win64.bat`, `scripts/launch_local_client_against_host_win64.bat`, `scripts/smoke_test_headless_host_win64.bat`, and `scripts/smoke_test_local_demo_win64.bat` still emit `[M40]` labels.
  - This does not block the gameplay milestone chain, but it is below the delivery standard expected at this stage.

## Assessment By Milestone

### M40

Status: SIGNED OFF AND STILL HEALTHY

Reason:

- previously signed off
- no regression surfaced in current build/package lane
- combat persistence and diagnostics surfaces remain present

### M41

Status: CONDITIONALLY ACCEPTED

Reason:

- objective and mission-state surfaces are present
- progression state is serialized
- no code-level blocker was found in the M41 chain
- current sign-off confidence is weakened by the lack of stronger automated proof of actual mission advancement

### M42

Status: CONDITIONALLY ACCEPTED

Reason:

- docked ship runtime, boarding, helm claim, and ship ownership state are present in code and persistence
- no blocking regression surfaced in package/smoke validation
- current acceptance lane does not strongly prove the boarding/command path in automated form

### M43

Status: NOT FULLY SIGNED OFF

Reason:

- package and acceptance wrapper pass
- orbital state surfaces are present and persisted
- but the current M43 acceptance pass does not require actual orbital progression and can succeed from the untouched starting state

### M44

Status: NOT FULLY SIGNED OFF

Reason:

- package and acceptance wrapper pass
- docking, landing, relay, and return-loop states are implemented in code and persistence
- but the current M44 acceptance pass does not require the full return loop to have actually occurred and can succeed from the untouched starting state

## What Needs To Happen Before Hard Sign-Off

1. Strengthen the staged acceptance lane so it validates actual milestone completion states rather than key presence only.

Minimum expectation:

- M43 acceptance should require:
  - `authority_mode=headless-host`
  - `host_online=yes`
  - `ship_command_claimed=yes`
  - `player_runtime_context=orbital-space`
  - `orbital_survey_orbit_reached=yes`
  - `orbital_relay_track_reached=yes`

- M44 acceptance should require:
  - `authority_mode=headless-host`
  - `host_online=yes`
  - `orbital_relay_platform_docked=yes`
  - `frontier_surface_active=yes` at the correct checkpoint
  - `orbital_return_route_authorized=yes`
  - `orbital_home_dock_reached=yes`
  - `mission_return_loop_complete=yes` or equivalent final-state proof

2. Add a staged integration script or operator-backed scripted lane that actually launches host plus client together before reading acceptance state.

3. Correct the README manual validation procedure so it explicitly tells operators to use the staged package copies of the launch scripts.

4. Clean up milestone branding drift from the visible runtime and batch-script outputs.

## Recommended Follow-Up For The Lead Developer

This should be treated as one focused closeout pass, not another broad milestone.

Required closeout items:

- strengthen M43 acceptance to require real orbital completion state
- strengthen M44 acceptance to require real return-loop completion state
- make the local demo validation lane exercise the packaged client against the packaged host, not just a standalone local client boot
- fix README operator instructions to point at the staged package root
- remove remaining M40 branding from the current runtime and script outputs

## Final Call

The repo is close.

This is not a case where M40-M44 looks fundamentally broken.
It is a case where the implementation appears materially real, but the validation lane is still too weak for a proper hard sign-off.

If the acceptance lane is tightened and a packaged authoritative walkthrough is re-run successfully, the M41-M44 chain should be able to move from conditional acceptance to full sign-off without a major architectural rewrite.
