@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"

set "CONFIG=%WAR_PACKAGE_CONFIG%"
if "%CONFIG%"=="" set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "PLATFORM=x64"

set "CLIENT_OUTPUT_DIR=%REPO_ROOT%\bin\%CONFIG%\split\desktop"
set "SERVER_OUTPUT_DIR=%REPO_ROOT%\bin\%CONFIG%\split\server"
set "CLIENT_EXE_PATH=%CLIENT_OUTPUT_DIR%\WAR.exe"
set "CLIENT_PDB_PATH=%CLIENT_OUTPUT_DIR%\WAR.pdb"
set "SERVER_EXE_PATH=%SERVER_OUTPUT_DIR%\WARServer.exe"
set "SERVER_PDB_PATH=%SERVER_OUTPUT_DIR%\WARServer.pdb"
set "STAGE_ROOT=%REPO_ROOT%\out\internal_alpha\WAR_M48_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "FAILURE_STAGE=%RUNTIME_STAGE%\CrashDumps\FailureBundles"
set "MANIFEST_PATH=%STAGE_ROOT%\internal_alpha_manifest.txt"

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
ping 127.0.0.1 -n 3 >nul

if exist "%STAGE_ROOT%" rmdir /s /q "%STAGE_ROOT%" >nul 2>nul
if exist "%STAGE_ROOT%" (
    echo [M48] ERROR: unable to clear existing stage root "%STAGE_ROOT%".
    exit /b 1
)

mkdir "%STAGE_ROOT%" || exit /b 1
mkdir "%STAGE_ROOT%\assets" || exit /b 1
mkdir "%STAGE_ROOT%\Environment" || exit /b 1
mkdir "%RUNTIME_STAGE%\Config" || exit /b 1
mkdir "%RUNTIME_STAGE%\Logs" || exit /b 1
mkdir "%RUNTIME_STAGE%\Saves" || exit /b 1
mkdir "%RUNTIME_STAGE%\CrashDumps" || exit /b 1
mkdir "%FAILURE_STAGE%\Startup" || exit /b 1
mkdir "%FAILURE_STAGE%\Runtime" || exit /b 1
mkdir "%FAILURE_STAGE%\Bootstrap" || exit /b 1
mkdir "%FAILURE_STAGE%\OperatorTriage" || exit /b 1
mkdir "%HOST_STAGE%" || exit /b 1
mkdir "%STAGE_ROOT%\Milestones" || exit /b 1

copy /y "%CLIENT_EXE_PATH%" "%STAGE_ROOT%\WAR.exe" >nul || exit /b 1
copy /y "%SERVER_EXE_PATH%" "%STAGE_ROOT%\WARServer.exe" >nul || exit /b 1
if exist "%CLIENT_PDB_PATH%" copy /y "%CLIENT_PDB_PATH%" "%STAGE_ROOT%\WAR.pdb" >nul
if exist "%SERVER_PDB_PATH%" copy /y "%SERVER_PDB_PATH%" "%STAGE_ROOT%\WARServer.pdb" >nul

if exist "%REPO_ROOT%\assets" (
    robocopy "%REPO_ROOT%\assets" "%STAGE_ROOT%\assets" /E /XD "%REPO_ROOT%\assets\imported_assets" /R:2 /W:1 /NFL /NDL /NJH /NJS /NP >nul
    set "ROBOCOPY_EXIT=%ERRORLEVEL%"
    if errorlevel 8 (
        echo [M48] ERROR: failed to stage assets from "%REPO_ROOT%\assets" to "%STAGE_ROOT%\assets" - robocopy exit !ROBOCOPY_EXIT!.
        exit /b 1
    )
)

if exist "%REPO_ROOT%\Environment" (
    robocopy "%REPO_ROOT%\Environment" "%STAGE_ROOT%\Environment" /E /R:2 /W:1 /NFL /NDL /NJH /NJS /NP >nul
    set "ROBOCOPY_EXIT=%ERRORLEVEL%"
    if errorlevel 8 (
        echo [M48] ERROR: failed to stage Environment from "%REPO_ROOT%\Environment" to "%STAGE_ROOT%\Environment" - robocopy exit !ROBOCOPY_EXIT!.
        exit /b 1
    )
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin" (
    echo [M48] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin" (
    echo [M48] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin" (
    echo [M48] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin" (
    echo [M48] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\textures\world_atlas.bmp" (
    echo [M48] ERROR: staged atlas texture missing at "%STAGE_ROOT%\assets\textures\world_atlas.bmp".
    exit /b 1
)

for %%F in (
    "README.md"
    "VALIDATION_EVIDENCE_M48.txt"
    "Milestones\M48_Crash_Capture_Failure_Bundles_Operator_Triage_Baseline.md"
    "scripts\launch_headless_host_win64.bat"
    "scripts\launch_local_demo_win64.bat"
    "scripts\launch_local_client_against_host_win64.bat"
    "scripts\smoke_test_headless_host_win64.bat"
    "scripts\smoke_test_local_demo_win64.bat"
    "scripts\validate_m45_hosted_bootstrap_win64.bat"
    "scripts\validate_m46_environment_identity_win64.bat"
    "scripts\validate_m46_missing_required_secrets_win64.bat"
    "scripts\validate_m46_runtime_save_hygiene_win64.bat"
    "scripts\validate_m47_ticket_issue_and_client_entry_win64.bat"
    "scripts\validate_m47_ticket_denial_and_fail_states_win64.bat"
    "scripts\validate_m47_reconnect_identity_win64.bat"
    "scripts\validate_m48_failure_bundle_capture_win64.bat"
    "scripts\validate_m48_operator_triage_artifacts_win64.bat"
    "scripts\validate_m48_internal_alpha_package_win64.bat"
    "scripts\open_operator_triage_folder_win64.bat"
) do (
    if exist "%REPO_ROOT%\%%~F" copy /y "%REPO_ROOT%\%%~F" "%STAGE_ROOT%\%%~nxF" >nul
)

(
    echo WAR Internal Alpha Manifest
    echo Milestone: M48 - Crash Capture / Failure Bundles / Operator Triage Baseline
    echo Configuration: %CONFIG%
    echo Platform: %PLATFORM%
    echo Stage root: %STAGE_ROOT%
    echo Client executable: %STAGE_ROOT%\WAR.exe
    echo Host executable: %STAGE_ROOT%\WARServer.exe
    echo Deployable environment root: %STAGE_ROOT%\Environment
    echo Mutable runtime root: %RUNTIME_STAGE%
    echo Failure bundle root: %FAILURE_STAGE%
    echo Operator triage root: %FAILURE_STAGE%\OperatorTriage
    echo Localhost fallback transport: file-backed-localhost-fallback
    echo Hosted bootstrap transport: file-backed-hosted-bootstrap
    echo Environment profiles staged: local, staging, hosted_internal_alpha
    echo Packaged operator triage helper:
    echo - open_operator_triage_folder_win64.bat
    echo Validation scripts staged:
    echo - validate_m48_failure_bundle_capture_win64.bat
    echo - validate_m48_operator_triage_artifacts_win64.bat
    echo - validate_m48_internal_alpha_package_win64.bat
) > "%MANIFEST_PATH%"

echo [M48] Internal alpha package staged at "%STAGE_ROOT%".
exit /b 0
