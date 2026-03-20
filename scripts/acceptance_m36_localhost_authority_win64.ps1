$ErrorActionPreference = 'Stop'

function Get-KeyValueMap {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

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

function Write-HarnessConfig {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Enabled,
        [Parameter(Mandatory = $true)]
        [int]$IntentLatencyMs,
        [Parameter(Mandatory = $true)]
        [int]$AckLatencyMs,
        [Parameter(Mandatory = $true)]
        [int]$SnapshotLatencyMs,
        [Parameter(Mandatory = $true)]
        [int]$JitterMs
    )

    $lines = @(
        'version=1',
        "enabled=$Enabled",
        "intent_latency_ms=$IntentLatencyMs",
        "ack_latency_ms=$AckLatencyMs",
        "snapshot_latency_ms=$SnapshotLatencyMs",
        "jitter_ms=$JitterMs"
    )

    Set-Content -LiteralPath $Path -Value $lines -Encoding ascii
}

function Wait-ForFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [int]$TimeoutSeconds,
        [Parameter(Mandatory = $true)]
        [string]$Label,
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)]
        [ref]$Failed
    )

    for ($index = 0; $index -lt $TimeoutSeconds; $index++) {
        if (Test-Path -LiteralPath $Path) {
            $Details.Add("[PASS] $Label")
            return $true
        }

        Start-Sleep -Seconds 1
    }

    $Details.Add("[FAIL] $Label missing: $Path")
    $Failed.Value = $true
    return $false
}

function Snapshot-ClientStatus {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePath,
        [Parameter(Mandatory = $true)]
        [string]$SnapshotPath,
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)]
        [ref]$Failed
    )

    try {
        Copy-Item -LiteralPath $SourcePath -Destination $SnapshotPath -Force
        return $true
    } catch {
        $Details.Add("[FAIL] Could not snapshot client replication status file: $($_.Exception.Message)")
        $Failed.Value = $true
        return $false
    }
}

function Wait-ForStatusValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePath,
        [Parameter(Mandatory = $true)]
        [string]$SnapshotPath,
        [Parameter(Mandatory = $true)]
        [string]$Key,
        [Parameter(Mandatory = $true)]
        [string]$ExpectedValue,
        [Parameter(Mandatory = $true)]
        [int]$TimeoutSeconds,
        [Parameter(Mandatory = $true)]
        [string]$PassLabel,
        [Parameter(Mandatory = $true)]
        [string]$FailLabel,
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)]
        [ref]$Failed
    )

    $lastValues = @{}

    for ($index = 0; $index -lt $TimeoutSeconds; $index++) {
        if (Snapshot-ClientStatus -SourcePath $SourcePath -SnapshotPath $SnapshotPath -Details $Details -Failed $Failed) {
            $lastValues = Get-KeyValueMap -Path $SnapshotPath
            if ($lastValues.ContainsKey($Key) -and $lastValues[$Key] -eq $ExpectedValue) {
                $Details.Add("[PASS] $PassLabel")
                return $lastValues
            }
        }

        Start-Sleep -Seconds 1
    }

    $Details.Add("[FAIL] $FailLabel")
    $Failed.Value = $true
    return $lastValues
}

function Get-StatusSnapshot {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePath,
        [Parameter(Mandatory = $true)]
        [string]$SnapshotPath,
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)]
        [ref]$Failed
    )

    if (-not (Snapshot-ClientStatus -SourcePath $SourcePath -SnapshotPath $SnapshotPath -Details $Details -Failed $Failed)) {
        return @{}
    }

    return Get-KeyValueMap -Path $SnapshotPath
}

function Check-NumericPresent {
    param(
        [Parameter(Mandatory = $true)]
        [AllowEmptyString()]
        [string]$Value,
        [Parameter(Mandatory = $true)]
        [string]$Label,
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)]
        [ref]$Failed
    )

    if ([string]::IsNullOrWhiteSpace($Value)) {
        $Details.Add("[FAIL] $Label")
        $Failed.Value = $true
        return
    }

    $parsedValue = 0
    if (-not [int]::TryParse($Value, [ref]$parsedValue)) {
        $Details.Add("[FAIL] $Label is not numeric: $Value")
        $Failed.Value = $true
        return
    }

    $Details.Add("[PASS] $Label = $Value")
}

function Compare-SnapshotAge {
    param(
        [Parameter(Mandatory = $true)]
        [AllowEmptyString()]
        [string]$FirstValue,
        [Parameter(Mandatory = $true)]
        [AllowEmptyString()]
        [string]$SecondValue,
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)]
        [ref]$Failed
    )

    $firstAge = 0
    if (-not [int]::TryParse($FirstValue, [ref]$firstAge)) {
        $Details.Add("[FAIL] Initial snapshot age is not numeric: $FirstValue")
        $Failed.Value = $true
        return
    }

    $secondAge = 0
    if (-not [int]::TryParse($SecondValue, [ref]$secondAge)) {
        $Details.Add("[FAIL] Later snapshot age is not numeric: $SecondValue")
        $Failed.Value = $true
        return
    }

    if ($secondAge -gt $firstAge) {
        $Details.Add("[PASS] Snapshot age grew under harness: $firstAge -> $secondAge")
        return
    }

    $Details.Add("[FAIL] Snapshot age did not grow under harness: $firstAge -> $secondAge")
    $Failed.Value = $true
}

function Stop-WarProcesses {
    foreach ($processName in @('WAR', 'WARServer')) {
        Get-Process -Name $processName -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    }
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$demoRoot = $scriptDir
if (-not (Test-Path -LiteralPath (Join-Path $demoRoot 'WAR.exe'))) {
    $demoRoot = Split-Path -Parent $scriptDir
}

$clientExePath = Join-Path $demoRoot 'WAR.exe'
$hostExePath = Join-Path $demoRoot 'WARServer.exe'
$hostArgumentList = @()
if (-not (Test-Path -LiteralPath $hostExePath)) {
    $hostExePath = $clientExePath
    $hostArgumentList += '--headless-host'
}

$runtimeRoot = Join-Path $demoRoot 'runtime'
if (-not (Test-Path -LiteralPath $runtimeRoot)) {
    $runtimeRoot = Join-Path $demoRoot 'Runtime'
}

$configDir = Join-Path $runtimeRoot 'Config'
$hostRoot = Join-Path $runtimeRoot 'Host'
$logDir = Join-Path $runtimeRoot 'Logs'
$harnessCfg = Join-Path $configDir 'replication_harness.cfg'
$statusFile = Join-Path $hostRoot 'headless_host_status.txt'
$snapshotFile = Join-Path $hostRoot 'authoritative_snapshot.txt'
$clientStatusFile = Join-Path $logDir 'client_replication_status.txt'
$statusSnapshotFile = Join-Path $logDir 'client_replication_status_snapshot.txt'
$reportPath = Join-Path $logDir 'm36_acceptance_report.txt'
$detailsPath = Join-Path $logDir 'm36_acceptance_details.txt'

if (-not (Test-Path -LiteralPath $clientExePath)) {
    Write-Host "[M36] ERROR: WAR.exe not found at `"$clientExePath`"."
    exit 1
}

if (-not (Test-Path -LiteralPath $hostExePath)) {
    Write-Host "[M36] ERROR: host executable not found at `"$hostExePath`"."
    exit 1
}

New-Item -ItemType Directory -Force -Path $configDir, $hostRoot, $logDir | Out-Null
foreach ($path in @($detailsPath, $reportPath, $statusFile, $snapshotFile, $clientStatusFile, $statusSnapshotFile)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force -ErrorAction SilentlyContinue
    }
}

$details = [System.Collections.Generic.List[string]]::new()
$failed = $false
$hostOnline = ''
$statusParseValid = ''
$firstSnapshotAge = ''
$secondSnapshotAge = ''
$correctionsApplied = ''
$divergenceEvents = ''
$pendingInbound = ''
$pendingAcks = ''
$pendingSnapshots = ''

try {
    Stop-WarProcesses
    Write-HarnessConfig -Path $harnessCfg -Enabled 'no' -IntentLatencyMs 0 -AckLatencyMs 0 -SnapshotLatencyMs 0 -JitterMs 0

    $details.Add('[INFO] Starting headless host...')
    $hostProcess = Start-Process -FilePath $hostExePath -ArgumentList ($hostArgumentList + '--host-run-seconds=30') -WindowStyle Minimized -PassThru
    Start-Sleep -Seconds 3
    Wait-ForFile -Path $statusFile -TimeoutSeconds 12 -Label 'headless host status file' -Details $details -Failed ([ref]$failed) | Out-Null

    $details.Add('[INFO] Launching client...')
    $clientProcess = Start-Process -FilePath $clientExePath -WindowStyle Minimized -PassThru
    Wait-ForFile -Path $clientStatusFile -TimeoutSeconds 15 -Label 'client replication status file' -Details $details -Failed ([ref]$failed) | Out-Null
    Wait-ForFile -Path $snapshotFile -TimeoutSeconds 15 -Label 'authoritative snapshot file' -Details $details -Failed ([ref]$failed) | Out-Null

    $initialStatus = Wait-ForStatusValue -SourcePath $clientStatusFile -SnapshotPath $statusSnapshotFile -Key 'host_online' -ExpectedValue 'yes' -TimeoutSeconds 10 -PassLabel 'Host heartbeat online' -FailLabel 'Host heartbeat not online' -Details $details -Failed ([ref]$failed)
    $hostOnline = $initialStatus['host_online']
    $statusParseValid = $initialStatus['status_parse_valid']
    if ($statusParseValid -eq 'yes') {
        $details.Add('[PASS] Host heartbeat parse valid')
    } else {
        $details.Add('[FAIL] Host heartbeat malformed or incomplete')
        $failed = $true
    }

    Write-HarnessConfig -Path $harnessCfg -Enabled 'yes' -IntentLatencyMs 100 -AckLatencyMs 100 -SnapshotLatencyMs 100 -JitterMs 25
    Start-Sleep -Seconds 4
    $firstStatus = Get-StatusSnapshot -SourcePath $clientStatusFile -SnapshotPath $statusSnapshotFile -Details $details -Failed ([ref]$failed)
    $firstSnapshotAge = $firstStatus['last_snapshot_age_ms']
    $correctionsApplied = $firstStatus['corrections_applied']
    $divergenceEvents = $firstStatus['divergence_events']
    $pendingInbound = $firstStatus['host_pending_inbound_intents']
    $pendingAcks = $firstStatus['host_pending_outbound_acks']
    $pendingSnapshots = $firstStatus['host_pending_snapshots']

    $details.Add('[INFO] Cycling higher latency/jitter preset...')
    Write-HarnessConfig -Path $harnessCfg -Enabled 'yes' -IntentLatencyMs 350 -AckLatencyMs 350 -SnapshotLatencyMs 350 -JitterMs 100
    Start-Sleep -Seconds 5
    $secondStatus = Get-StatusSnapshot -SourcePath $clientStatusFile -SnapshotPath $statusSnapshotFile -Details $details -Failed ([ref]$failed)
    $secondSnapshotAge = $secondStatus['last_snapshot_age_ms']
    $correctionsApplied = $secondStatus['corrections_applied']
    $divergenceEvents = $secondStatus['divergence_events']
    $pendingInbound = $secondStatus['host_pending_inbound_intents']
    $pendingAcks = $secondStatus['host_pending_outbound_acks']
    $pendingSnapshots = $secondStatus['host_pending_snapshots']
    $hostOnline = $secondStatus['host_online']
    $statusParseValid = $secondStatus['status_parse_valid']

    Check-NumericPresent -Value $firstSnapshotAge -Label 'Initial snapshot age visible' -Details $details -Failed ([ref]$failed)
    Check-NumericPresent -Value $secondSnapshotAge -Label 'Later snapshot age visible' -Details $details -Failed ([ref]$failed)
    Check-NumericPresent -Value $correctionsApplied -Label 'Correction count visible' -Details $details -Failed ([ref]$failed)
    Check-NumericPresent -Value $divergenceEvents -Label 'Divergence count visible' -Details $details -Failed ([ref]$failed)
    Check-NumericPresent -Value $pendingInbound -Label 'Host inbound queue visibility' -Details $details -Failed ([ref]$failed)
    Check-NumericPresent -Value $pendingAcks -Label 'Host acknowledgement queue visibility' -Details $details -Failed ([ref]$failed)
    Check-NumericPresent -Value $pendingSnapshots -Label 'Host snapshot queue visibility' -Details $details -Failed ([ref]$failed)
    Compare-SnapshotAge -FirstValue $firstSnapshotAge -SecondValue $secondSnapshotAge -Details $details -Failed ([ref]$failed)
} finally {
    Write-HarnessConfig -Path $harnessCfg -Enabled 'no' -IntentLatencyMs 0 -AckLatencyMs 0 -SnapshotLatencyMs 0 -JitterMs 0
    Stop-WarProcesses
}

$reportLines = @(
    'WAR M36 Acceptance Report',
    "Demo root: $demoRoot",
    "Client executable: $clientExePath",
    "Host executable: $hostExePath",
    "Runtime root: $runtimeRoot",
    "Harness config: $harnessCfg",
    "Host status file: $statusFile",
    "Snapshot file: $snapshotFile",
    "Client status file: $clientStatusFile",
    "Host online: $hostOnline",
    "Host status parse valid: $statusParseValid",
    "Initial snapshot age ms: $firstSnapshotAge",
    "Later snapshot age ms: $secondSnapshotAge",
    "Corrections applied: $correctionsApplied",
    "Divergence events: $divergenceEvents",
    "Host pending inbound intents: $pendingInbound",
    "Host pending outbound acks: $pendingAcks",
    "Host pending snapshots: $pendingSnapshots",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    ''
) + $details

Set-Content -LiteralPath $detailsPath -Value $details -Encoding ascii
Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    Write-Host '[M36] Acceptance failed.'
    exit 1
}

Write-Host '[M36] Acceptance passed.'
exit 0
