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
$reportPath = Join-Path $logDir 'm42_ship_runtime_acceptance_report.txt'

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
        'ship_runtime_prep_ready',
        'active_ship_id',
        'ship_name',
        'ship_boarded',
        'ship_docked',
        'ship_power_online',
        'ship_airlock_pressurized',
        'ship_command_claimed',
        'ship_launch_prep_ready',
        'ship_ownership',
        'ship_occupancy',
        'ship_boarding_count',
        'player_runtime_context'
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
        'ship_boarded',
        'ship_command_claimed',
        'ship_launch_prep_ready'
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
    'WAR M42 Acceptance Report',
    "Demo root: $demoRoot",
    "Runtime root: $runtimeRoot",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    '',
    'Manual operator steps still required: finish the planetary mission chain, board Responder Shuttle Khepri, and claim Shuttle Helm Terminal before final sign-off.',
    'This scripted lane verifies that the telemetry and persistence surfaces needed for that review are actually present.',
    ''
) + $details

Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    exit 1
}

exit 0
