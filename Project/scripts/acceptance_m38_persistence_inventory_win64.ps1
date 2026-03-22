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

function Stop-WarProcesses {
    foreach ($processName in @('WAR', 'WARServer')) {
        Get-Process -Name $processName -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    }
}

function Wait-ForFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [int]$TimeoutSeconds
    )

    for ($index = 0; $index -lt $TimeoutSeconds; $index++) {
        if (Test-Path -LiteralPath $Path) {
            return $true
        }

        Start-Sleep -Seconds 1
    }

    return $false
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$demoRoot = $scriptDir
if (-not (Test-Path -LiteralPath (Join-Path $demoRoot 'WAR.exe'))) {
    $demoRoot = Split-Path -Parent $scriptDir
}

$clientExePath = Join-Path $demoRoot 'WAR.exe'
$hostExePath = Join-Path $demoRoot 'WARServer.exe'
$hostArgs = @()
if (-not (Test-Path -LiteralPath $hostExePath)) {
    $hostExePath = $clientExePath
    $hostArgs += '--headless-host'
}

$runtimeRoot = Join-Path $demoRoot 'runtime'
if (-not (Test-Path -LiteralPath $runtimeRoot)) {
    $runtimeRoot = Join-Path $demoRoot 'Runtime'
}

$savePath = Join-Path $runtimeRoot 'Saves\authoritative_world_primary.txt'
$logDir = Join-Path $runtimeRoot 'Logs'
$reportPath = Join-Path $logDir 'm38_persistence_report.txt'
$detailsPath = Join-Path $logDir 'm38_persistence_details.txt'

New-Item -ItemType Directory -Force -Path $logDir, (Split-Path -Parent $savePath) | Out-Null

$details = [System.Collections.Generic.List[string]]::new()
$failed = $false

try {
    Stop-WarProcesses
    foreach ($path in @($savePath, $reportPath, $detailsPath)) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force -ErrorAction SilentlyContinue
        }
    }

    $legacySeed = @(
        'version=1',
        'valid=yes',
        'simulation_ticks=120',
        'last_processed_intent_sequence=14',
        'published_epoch_ms=0',
        'player_x=120',
        'player_y=120',
        'movement_target_active=no',
        'movement_target_x=0',
        'movement_target_y=0',
        'path_index=0',
        'path_count=0',
        'entity_count=0',
        'event_count=1',
        'event_0=Legacy persistence seed'
    )
    Set-Content -LiteralPath $savePath -Value $legacySeed -Encoding ascii
    $details.Add('[PASS] Seeded legacy v1 authoritative save')

    $hostProcess = Start-Process -FilePath $hostExePath -ArgumentList ($hostArgs + '--host-run-seconds=4') -WindowStyle Minimized -PassThru
    Start-Sleep -Seconds 6

    if (-not (Wait-ForFile -Path $savePath -TimeoutSeconds 5)) {
        $details.Add("[FAIL] Save file missing after first host boot: $savePath")
        $failed = $true
    }

    $first = Get-KeyValueMap -Path $savePath

    if ($first['schema_version'] -eq '3') {
        $details.Add('[PASS] Save migrated to schema 3')
    } else {
        $details.Add("[FAIL] Save schema_version expected 3 but found '$($first['schema_version'])'")
        $failed = $true
    }

    if ($first['migrated_from_schema_version'] -eq '1') {
        $details.Add('[PASS] Save records migration from schema 1')
    } else {
        $details.Add("[FAIL] migrated_from_schema_version expected 1 but found '$($first['migrated_from_schema_version'])'")
        $failed = $true
    }

    $firstTicks = 0
    if ([int]::TryParse($first['simulation_ticks'], [ref]$firstTicks) -and $firstTicks -gt 120) {
        $details.Add("[PASS] simulation_ticks restored and advanced: $firstTicks")
    } else {
        $details.Add("[FAIL] simulation_ticks did not restore/advance correctly: '$($first['simulation_ticks'])'")
        $failed = $true
    }

    if ($first['last_processed_intent_sequence'] -eq '14') {
        $details.Add('[PASS] last_processed_intent_sequence restored from persisted state')
    } else {
        $details.Add("[FAIL] last_processed_intent_sequence expected 14 but found '$($first['last_processed_intent_sequence'])'")
        $failed = $true
    }

    $nextSequence = 0
    if ([int]::TryParse($first['next_intent_sequence'], [ref]$nextSequence) -and $nextSequence -ge 15) {
        $details.Add("[PASS] next_intent_sequence restored safely: $nextSequence")
    } else {
        $details.Add("[FAIL] next_intent_sequence expected >= 15 but found '$($first['next_intent_sequence'])'")
        $failed = $true
    }

    foreach ($requiredKey in @('player_health_current','player_health_max','inventory_count','equipment_weapon','equipment_suit','equipment_tool')) {
        if ($first.ContainsKey($requiredKey)) {
            $details.Add("[PASS] Persisted actor field present: $requiredKey")
        } else {
            $details.Add("[FAIL] Persisted actor field missing: $requiredKey")
            $failed = $true
        }
    }

    Stop-WarProcesses
    Start-Sleep -Seconds 1

    $hostProcess2 = Start-Process -FilePath $hostExePath -ArgumentList ($hostArgs + '--host-run-seconds=3') -WindowStyle Minimized -PassThru
    Start-Sleep -Seconds 5

    $second = Get-KeyValueMap -Path $savePath
    $secondTicks = 0
    if ([int]::TryParse($second['simulation_ticks'], [ref]$secondTicks) -and $secondTicks -gt $firstTicks) {
        $details.Add("[PASS] Second boot resumed persisted simulation tick state: $firstTicks -> $secondTicks")
    } else {
        $details.Add("[FAIL] Second boot did not resume persisted simulation ticks: first=$firstTicks second='$($second['simulation_ticks'])'")
        $failed = $true
    }

    if ($second.ContainsKey('persistence_schema_version')) {
        $details.Add('[INFO] Save contains explicit persistence schema mirror')
    }

    if (Test-Path -LiteralPath (Join-Path $demoRoot 'acceptance_m36_localhost_authority_win64.bat')) {
        $details.Add('[PASS] M36 regression acceptance lane staged alongside M38 package')
    } else {
        $details.Add('[FAIL] M36 regression acceptance lane missing from staged package')
        $failed = $true
    }
}
finally {
    Stop-WarProcesses
}

$reportLines = @(
    'WAR M38 Persistence / Inventory Acceptance Report',
    "Demo root: $demoRoot",
    "Client executable: $clientExePath",
    "Host executable: $hostExePath",
    "Runtime root: $runtimeRoot",
    "Save path: $savePath",
    $(if ($failed) { 'Result: FAIL' } else { 'Result: PASS' }),
    ''
) + $details

Set-Content -LiteralPath $detailsPath -Value $details -Encoding ascii
Set-Content -LiteralPath $reportPath -Value $reportLines -Encoding ascii
Get-Content -LiteralPath $reportPath

if ($failed) {
    Write-Host '[M38] Persistence acceptance failed.'
    exit 1
}

Write-Host '[M38] Persistence acceptance passed.'
exit 0
