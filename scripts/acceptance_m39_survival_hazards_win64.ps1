$ErrorActionPreference = 'Stop'

function Get-KeyValueMap {
    param([string]$Path)
    $values = @{}
    if (-not (Test-Path -LiteralPath $Path)) { return $values }
    foreach ($line in Get-Content -LiteralPath $Path) {
        if ($line -match '^(.*?)=(.*)$') { $values[$matches[1]] = $matches[2] }
    }
    return $values
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$demoRoot = $scriptDir
if (-not (Test-Path -LiteralPath (Join-Path $demoRoot 'WAR.exe'))) {
    $demoRoot = Split-Path -Parent $scriptDir
}
$runtimeRoot = Join-Path $demoRoot 'runtime'
if (-not (Test-Path -LiteralPath $runtimeRoot)) { $runtimeRoot = Join-Path $demoRoot 'Runtime' }
$logDir = Join-Path $runtimeRoot 'Logs'
$clientStatusFile = Join-Path $logDir 'client_replication_status.txt'
$reportPath = Join-Path $logDir 'm39_hazard_acceptance_report.txt'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$details = [System.Collections.Generic.List[string]]::new()
$failed = $false
if (-not (Test-Path -LiteralPath $clientStatusFile)) {
    $details.Add('[FAIL] client_replication_status.txt missing')
    $failed = $true
} else {
    $status = Get-KeyValueMap -Path $clientStatusFile
    foreach ($key in @('player_health','player_max_health','player_armor','suit_integrity','oxygen_seconds_remaining','radiation_dose','toxic_exposure','hazard_ticks','terrain_consequence_events','player_in_hazard','current_hazard_label','current_terrain_consequence')) {
        if ($status.ContainsKey($key)) { $details.Add("[PASS] $key visible = $($status[$key])") } else { $details.Add("[FAIL] $key missing"); $failed = $true }
    }
}
$reportLines = @('WAR M39 Hazard Acceptance Report', "Runtime root: $runtimeRoot", $(if ($failed) {'Result: FAIL'} else {'Result: PASS'}), '') + $details
Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath
if ($failed) { exit 1 } else { exit 0 }
