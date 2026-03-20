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

function Set-KeyValueInFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Key,
        [Parameter(Mandatory = $true)]
        [string]$Value
    )

    $lines = Get-Content -LiteralPath $Path
    $updated = $false
    for ($i = 0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -match "^$([regex]::Escape($Key))=") {
            $lines[$i] = "$Key=$Value"
            $updated = $true
            break
        }
    }

    if (-not $updated) {
        $lines += "$Key=$Value"
    }

    Set-Content -LiteralPath $Path -Value $lines -Encoding ascii
}

function Wait-ForFile {
    param(
        [Parameter(Mandatory = $true)] [string]$Path,
        [Parameter(Mandatory = $true)] [int]$TimeoutSeconds,
        [Parameter(Mandatory = $true)] [string]$Label,
        [Parameter(Mandatory = $true)] [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)] [ref]$Failed
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

function Wait-ForHostStatusValue {
    param(
        [Parameter(Mandatory = $true)] [string]$StatusPath,
        [Parameter(Mandatory = $true)] [string]$Key,
        [Parameter(Mandatory = $true)] [string]$ExpectedValue,
        [Parameter(Mandatory = $true)] [int]$TimeoutSeconds,
        [Parameter(Mandatory = $true)] [string]$Label,
        [Parameter(Mandatory = $true)] [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)] [ref]$Failed
    )

    for ($index = 0; $index -lt $TimeoutSeconds; $index++) {
        $values = Get-KeyValueMap -Path $StatusPath
        if ($values.ContainsKey($Key) -and $values[$Key] -eq $ExpectedValue) {
            $Details.Add("[PASS] $Label")
            return $values
        }
        Start-Sleep -Seconds 1
    }

    $Details.Add("[FAIL] $Label")
    $Failed.Value = $true
    return @{}
}

function Check-NumericAtLeast {
    param(
        [Parameter(Mandatory = $true)] [string]$Value,
        [Parameter(Mandatory = $true)] [long]$Minimum,
        [Parameter(Mandatory = $true)] [string]$Label,
        [Parameter(Mandatory = $true)] [System.Collections.Generic.List[string]]$Details,
        [Parameter(Mandatory = $true)] [ref]$Failed
    )

    $parsed = 0L
    if (-not [long]::TryParse($Value, [ref]$parsed)) {
        $Details.Add("[FAIL] $Label is not numeric: $Value")
        $Failed.Value = $true
        return
    }

    if ($parsed -ge $Minimum) {
        $Details.Add("[PASS] $Label = $parsed")
    } else {
        $Details.Add("[FAIL] $Label below minimum $Minimum: $parsed")
        $Failed.Value = $true
    }
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

$savesDir = Join-Path $runtimeRoot 'Saves'
$hostRoot = Join-Path $runtimeRoot 'Host'
$logDir = Join-Path $runtimeRoot 'Logs'
$savePath = Join-Path $savesDir 'authoritative_world_primary.txt'
$statusFile = Join-Path $hostRoot 'headless_host_status.txt'
$reportPath = Join-Path $logDir 'm37_persistence_report.txt'
$detailsPath = Join-Path $logDir 'm37_persistence_details.txt'

if (-not (Test-Path -LiteralPath $clientExePath)) {
    Write-Host "[M37] ERROR: WAR.exe not found at `"$clientExePath`"."
    exit 1
}

if (-not (Test-Path -LiteralPath $hostExePath)) {
    Write-Host "[M37] ERROR: host executable not found at `"$hostExePath`"."
    exit 1
}

New-Item -ItemType Directory -Force -Path $savesDir, $hostRoot, $logDir | Out-Null
foreach ($path in @($detailsPath, $reportPath, $statusFile, $savePath)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Force -ErrorAction SilentlyContinue
    }
}

$details = [System.Collections.Generic.List[string]]::new()
$failed = $false
$firstSaveTicks = ''
$secondSaveTicks = ''
$secondSaveSequence = ''
$loadedSchemaVersion = ''
$migratedFromVersion = ''
$loadSucceeded = ''
$saveSucceeded = ''

try {
    Stop-WarProcesses

    $details.Add('[INFO] Starting initial host run to create canonical save...')
    Start-Process -FilePath $hostExePath -ArgumentList ($hostArgumentList + '--host-run-seconds=3') -WindowStyle Minimized | Out-Null
    Wait-ForFile -Path $savePath -TimeoutSeconds 12 -Label 'initial authoritative save file' -Details $details -Failed ([ref]$failed) | Out-Null
    Start-Sleep -Seconds 4

    if (Test-Path -LiteralPath $savePath) {
        $initialSave = Get-KeyValueMap -Path $savePath
        $firstSaveTicks = $initialSave['simulation_ticks']
        Check-NumericAtLeast -Value $firstSaveTicks -Minimum 1 -Label 'Initial saved simulation ticks' -Details $details -Failed ([ref]$failed)

        Set-KeyValueInFile -Path $savePath -Key 'schema_version' -Value '1'
        Set-KeyValueInFile -Path $savePath -Key 'simulation_ticks' -Value '123'
        Set-KeyValueInFile -Path $savePath -Key 'last_processed_intent_sequence' -Value '17'
        $details.Add('[INFO] Rewrote staged save to schema_version=1, simulation_ticks=123, last_processed_intent_sequence=17 for migration/load validation.')
    }

    $details.Add('[INFO] Starting second host run to load, migrate, and republish the save...')
    Start-Process -FilePath $hostExePath -ArgumentList ($hostArgumentList + '--host-run-seconds=3') -WindowStyle Minimized | Out-Null
    $statusValues = Wait-ForHostStatusValue -StatusPath $statusFile -Key 'persistence_last_load_succeeded' -ExpectedValue 'yes' -TimeoutSeconds 12 -Label 'Host reports successful persistence load' -Details $details -Failed ([ref]$failed)
    Start-Sleep -Seconds 4

    if (Test-Path -LiteralPath $savePath) {
        $secondSave = Get-KeyValueMap -Path $savePath
        $schemaVersion = $secondSave['schema_version']
        if ($schemaVersion -eq '2') {
            $details.Add('[PASS] Save republished in canonical schema_version=2')
        } else {
            $details.Add("[FAIL] Save schema_version not republished to 2: $schemaVersion")
            $failed = $true
        }

        $secondSaveTicks = $secondSave['simulation_ticks']
        $secondSaveSequence = $secondSave['last_processed_intent_sequence']
        Check-NumericAtLeast -Value $secondSaveTicks -Minimum 124 -Label 'Loaded runtime advanced persisted simulation ticks' -Details $details -Failed ([ref]$failed)
        if ($secondSaveSequence -eq '17') {
            $details.Add('[PASS] Loaded runtime preserved last processed intent sequence = 17')
        } else {
            $details.Add("[FAIL] Loaded runtime did not preserve last processed intent sequence: $secondSaveSequence")
            $failed = $true
        }
    }

    if ($statusValues.Count -gt 0) {
        $loadedSchemaVersion = $statusValues['persistence_loaded_schema_version']
        $migratedFromVersion = $statusValues['persistence_migrated_from_version']
        $loadSucceeded = $statusValues['persistence_last_load_succeeded']
        $saveSucceeded = $statusValues['persistence_last_save_succeeded']
        if ($loadedSchemaVersion -eq '1') {
            $details.Add('[PASS] Host status reported loaded schema version = 1')
        } else {
            $details.Add("[FAIL] Host status did not report loaded schema version 1: $loadedSchemaVersion")
            $failed = $true
        }
        if ($migratedFromVersion -eq '1') {
            $details.Add('[PASS] Host status reported migration from schema version 1')
        } else {
            $details.Add("[FAIL] Host status did not report migration from schema version 1: $migratedFromVersion")
            $failed = $true
        }
        if ($loadSucceeded -eq 'yes') {
            $details.Add('[PASS] Host status reported persistence load success')
        } else {
            $details.Add('[FAIL] Host status did not report persistence load success')
            $failed = $true
        }
    }
} finally {
    Stop-WarProcesses
}

$reportLines = @(
    'WAR M37 Persistence Save/Load/Migration Report',
    "Demo root: $demoRoot",
    "Client executable: $clientExePath",
    "Host executable: $hostExePath",
    "Runtime root: $runtimeRoot",
    "Save path: $savePath",
    "Host status file: $statusFile",
    "Initial saved simulation ticks: $firstSaveTicks",
    "Republished simulation ticks: $secondSaveTicks",
    "Republished last processed intent sequence: $secondSaveSequence",
    "Host reported loaded schema version: $loadedSchemaVersion",
    "Host reported migrated-from version: $migratedFromVersion",
    "Host reported load succeeded: $loadSucceeded",
    "Host reported save succeeded: $saveSucceeded",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    ''
) + $details

Set-Content -LiteralPath $detailsPath -Value $details -Encoding ascii
Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    Write-Host '[M37] Persistence drill failed.'
    exit 1
}

Write-Host '[M37] Persistence drill passed.'
exit 0
