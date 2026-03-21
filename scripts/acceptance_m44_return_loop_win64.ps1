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

function Require-ExactValue {
    param(
        [hashtable]$Values,
        [string]$Key,
        [string]$Expected,
        [System.Collections.Generic.List[string]]$Details,
        [ref]$Failed,
        [string]$SurfaceLabel
    )

    if (-not $Values.ContainsKey($Key)) {
        $Details.Add("[FAIL] $SurfaceLabel missing required key: $Key")
        $Failed.Value = $true
        return
    }

    $actual = $Values[$Key]
    if ($actual -ne $Expected) {
        $Details.Add("[FAIL] $SurfaceLabel $Key expected '$Expected' but found '$actual'")
        $Failed.Value = $true
        return
    }

    $Details.Add("[PASS] $SurfaceLabel $Key = $actual")
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
$reportPath = Join-Path $logDir 'm44_return_loop_acceptance_report.txt'

New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$details = [System.Collections.Generic.List[string]]::new()
$failed = $false

if (-not (Test-Path -LiteralPath $statusPath)) {
    $details.Add("[FAIL] client_replication_status.txt missing: $statusPath")
    $failed = $true
} else {
    $status = Get-KeyValueMap -Path $statusPath
    Require-ExactValue -Values $status -Key 'authority_mode' -Expected 'headless-host' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'host_online' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'orbital_relay_platform_docked' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'orbital_return_route_authorized' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'orbital_home_dock_reached' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'mission_complete' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'mission_phase' -Expected 'mission-complete' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'player_runtime_context' -Expected 'planet-surface' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'ship_docked' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'ship_location' -Expected 'cargo-bay-dock' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'

    if ($status.ContainsKey('orbital_return_route_authorized') -and $status['orbital_return_route_authorized'] -eq 'yes') {
        $details.Add("[PASS] client status orbital_return_route_authorized=yes is accepted as authoritative proof that the frontier-surface relay checkpoint occurred before the homeward route was opened.")
    }
}

if (Test-Path -LiteralPath $hostStatusPath) {
    $hostStatus = Get-KeyValueMap -Path $hostStatusPath
    Require-ExactValue -Values $hostStatus -Key 'state' -Expected 'running' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'host_authority_active' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'orbital_relay_platform_docked' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'orbital_return_route_authorized' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'orbital_home_dock_reached' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'mission_complete' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'mission_phase' -Expected 'mission-complete' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'player_runtime_context' -Expected 'planet-surface' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'ship_location' -Expected 'cargo-bay-dock' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
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
    'WAR M44 Acceptance Report',
    "Demo root: $demoRoot",
    "Runtime root: $runtimeRoot",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    '',
    'PASS now means the packaged authoritative lane has actually completed docking, frontier relay authorization, home docking, and final return-loop closure state.',
    'Return-route authorization is treated as authoritative evidence that the frontier-surface landing and relay checkpoint occurred before the homeward route was opened.',
    ''
) + $details

Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    exit 1
}

exit 0
