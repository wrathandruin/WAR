$ErrorActionPreference = 'Stop'

function Get-KeyValueMap {
    param([string]$Path)

    $values = @{}
    if (-not (Test-Path -LiteralPath $Path)) {
        return $values
    }

    foreach ($line in Get-Content -LiteralPath $Path) {
        if ($line -match '^(.*?)=(.*)$') {
            $values[$matches[1]] = $matches[2]
        }
    }

    return $values
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$demoRoot = $scriptDir
if (-not (Test-Path -LiteralPath (Join-Path $demoRoot 'WAR.exe'))) {
    $demoRoot = Split-Path -Parent $scriptDir
}

$runtimeRoot = Join-Path $demoRoot 'runtime'
if (-not (Test-Path -LiteralPath $runtimeRoot)) {
    $runtimeRoot = Join-Path $demoRoot 'Runtime'
}

$logDir = Join-Path $runtimeRoot 'Logs'
$hostRoot = Join-Path $runtimeRoot 'Host'
$savePath = Join-Path $runtimeRoot 'Saves\authoritative_world_primary.txt'
$statusPath = Join-Path $logDir 'client_replication_status.txt'
$hostStatusPath = Join-Path $hostRoot 'headless_host_status.txt'
$reportPath = Join-Path $logDir 'm43_orbital_space_acceptance_report.txt'

New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$details = [System.Collections.Generic.List[string]]::new()
$failed = $false

if (-not (Test-Path -LiteralPath $statusPath)) {
    $details.Add("[FAIL] client_replication_status.txt missing: $statusPath")
    $failed = $true
} else {
    $status = Get-KeyValueMap -Path $statusPath
    foreach ($key in @(
        'mission_phase',
        'mission_objective',
        'mission_complete',
        'ship_name',
        'ship_command_claimed',
        'player_runtime_context',
        'orbital_layer_active',
        'orbital_departure_authorized',
        'orbital_travel_in_progress',
        'orbital_survey_orbit_reached',
        'orbital_relay_track_reached',
        'orbital_phase',
        'orbital_current_node',
        'orbital_target_node',
        'orbital_rule',
        'orbital_last_beat',
        'orbital_transfer_count',
        'orbital_travel_ticks_remaining'
    )) {
        if ($status.ContainsKey($key)) {
            $details.Add("[PASS] client status key visible: $key = $($status[$key])")
        } else {
            $details.Add("[FAIL] client status key missing: $key")
            $failed = $true
        }
    }
}

if (Test-Path -LiteralPath $hostStatusPath) {
    $hostStatus = Get-KeyValueMap -Path $hostStatusPath
    foreach ($key in @(
        'state',
        'host_authority_active',
        'ship_active',
        'ship_command_claimed',
        'orbital_layer_active',
        'orbital_departure_authorized',
        'orbital_phase',
        'orbital_current_node',
        'orbital_target_node'
    )) {
        if ($hostStatus.ContainsKey($key)) {
            $details.Add("[PASS] host status key visible: $key = $($hostStatus[$key])")
        } else {
            $details.Add("[FAIL] host status key missing: $key")
            $failed = $true
        }
    }
} else {
    $details.Add("[FAIL] headless_host_status.txt missing: $hostStatusPath")
    $failed = $true
}

if (Test-Path -LiteralPath $savePath) {
    $details.Add("[PASS] authoritative save present: $savePath")
} else {
    $details.Add("[FAIL] authoritative save missing: $savePath")
    $failed = $true
}

$reportLines = @(
    'WAR M43 Acceptance Report',
    "Demo root: $demoRoot",
    "Runtime root: $runtimeRoot",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    '',
    'Manual operator steps still required: complete the directed mission chain, board and command Responder Shuttle Khepri, enter the orbital lane, reach Debris Survey Orbit, and stabilize Relay Holding Track before final sign-off.',
    'This scripted lane verifies that the telemetry and persistence surfaces needed for that review are actually present.',
    ''
) + $details

Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    exit 1
}

exit 0
