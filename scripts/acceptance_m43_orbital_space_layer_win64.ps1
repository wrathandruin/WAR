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
$reportPath = Join-Path $logDir 'm43_orbital_space_acceptance_report.txt'

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
    Require-ExactValue -Values $status -Key 'ship_command_claimed' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'player_runtime_context' -Expected 'orbital-space' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'orbital_survey_orbit_reached' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'orbital_relay_track_reached' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'orbital_phase' -Expected 'relay-holding' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'orbital_current_node' -Expected 'relay-holding-track' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
    Require-ExactValue -Values $status -Key 'mission_phase' -Expected 'dock-relay-platform' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'client status'
}

if (Test-Path -LiteralPath $hostStatusPath) {
    $hostStatus = Get-KeyValueMap -Path $hostStatusPath
    Require-ExactValue -Values $hostStatus -Key 'state' -Expected 'running' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'host_authority_active' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'ship_command_claimed' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'orbital_layer_active' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'orbital_survey_orbit_reached' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'orbital_relay_track_reached' -Expected 'yes' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
    Require-ExactValue -Values $hostStatus -Key 'player_runtime_context' -Expected 'orbital-space' -Details $details -Failed ([ref]$failed) -SurfaceLabel 'host status'
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
    'PASS now means the packaged authoritative lane has actually reached relay-track completion state.',
    'This lane no longer passes on key presence alone.',
    ''
) + $details

Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    exit 1
}

exit 0
