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
$reportPath = Join-Path $logDir 'm40_combat_acceptance_report.txt'

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
        'last_position_drift_units',
        'last_path_divergence',
        'last_entity_divergence',
        'player_health',
        'player_armor',
        'suit_integrity',
        'inventory_stack_count',
        'inventory_item_count',
        'combat_active',
        'combat_label',
        'combat_round',
        'combat_ticks_remaining',
        'hostile_label',
        'hostile_health',
        'hostile_max_health',
        'combat_rounds_resolved',
        'encounter_wins'
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
    foreach ($key in @('state','host_authority_active','persistence_save_present')) {
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
    'WAR M40 Acceptance Report',
    "Demo root: $demoRoot",
    "Runtime root: $runtimeRoot",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    '',
    'Manual operator step still required: move the player into an encounter hotspot and let at least one six-second round resolve before final sign-off.',
    'This scripted lane verifies that the telemetry and persistence surfaces needed for that review are actually present.',
    ''
) + $details

Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    exit 1
}

exit 0
