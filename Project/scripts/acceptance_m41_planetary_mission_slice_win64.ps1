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
$reportPath = Join-Path $logDir 'm41_mission_acceptance_report.txt'

New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$details = [System.Collections.Generic.List[string]]::new()
$failed = $false

if (-not (Test-Path -LiteralPath $statusPath)) {
    $details.Add("[FAIL] client_replication_status.txt missing: $statusPath")
    $failed = $true
} else {
    $status = Get-KeyValueMap -Path $statusPath
    foreach ($key in @(
        'host_online',
        'protocol_lane_ready',
        'client_prediction_enabled',
        'mission_active',
        'mission_id',
        'mission_phase',
        'mission_objective',
        'mission_last_beat',
        'mission_advancement_count',
        'mission_complete',
        'mission_gate_locked',
        'ship_runtime_prep_ready',
        'combat_active',
        'combat_label',
        'combat_round',
        'hostile_label',
        'hostile_health',
        'persistence_schema_version',
        'persistence_loaded_schema_version'
    )) {
        if ($status.ContainsKey($key)) {
            $details.Add("[PASS] client status key visible: $key = $($status[$key])")
        } else {
            $details.Add("[FAIL] client status key missing: $key")
            $failed = $true
        }
    }

    if (
        $status.ContainsKey('latency_harness_enabled') -and
        $status['latency_harness_enabled'] -eq 'no' -and
        $status.ContainsKey('corrections_applied')
    )
    {
        $correctionCount = 0
        [void][int]::TryParse($status['corrections_applied'], [ref]$correctionCount)
        if ($correctionCount -gt 8) {
            $details.Add("[WARN] corrections_applied is currently high with latency harness disabled: $correctionCount")
        } else {
            $details.Add("[PASS] correction count remains within the expected localhost stability envelope: $correctionCount")
        }
    }
}

if (Test-Path -LiteralPath $hostStatusPath) {
    $hostStatus = Get-KeyValueMap -Path $hostStatusPath
    foreach ($key in @('state','host_authority_active','persistence_save_present','mission_phase','mission_complete','ship_runtime_prep_ready')) {
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
    'WAR M41 Acceptance Report',
    "Demo root: $demoRoot",
    "Runtime root: $runtimeRoot",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    '',
    'Manual operator steps required for final sign-off:',
    '1. interact with Transit Service Terminal',
    '2. inspect Diagnostic Station',
    '3. confirm the quarantine route unlocks',
    '4. resolve the Quarantine Access Gate combat beat',
    '5. interact with Quarantine Control Terminal',
    '6. restart the host and confirm mission state persists',
    '',
    'This scripted lane verifies that the telemetry and persistence surfaces required for that review are actually present.',
    ''
) + $details

Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    exit 1
}

exit 0
