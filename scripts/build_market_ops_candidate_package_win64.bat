@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "PLATFORM=x64"

set "SOLUTION_FILE=%REPO_ROOT%\WAR.sln"
set "CLIENT_OUTPUT_DIR=%REPO_ROOT%\bin\%CONFIG%\split\desktop"
set "SERVER_OUTPUT_DIR=%REPO_ROOT%\bin\%CONFIG%\split\server"
set "CLIENT_EXE_PATH=%CLIENT_OUTPUT_DIR%\WAR.exe"
set "CLIENT_PDB_PATH=%CLIENT_OUTPUT_DIR%\WAR.pdb"
set "SERVER_EXE_PATH=%SERVER_OUTPUT_DIR%\WARServer.exe"
set "SERVER_PDB_PATH=%SERVER_OUTPUT_DIR%\WARServer.pdb"
set "STAGE_ROOT=%REPO_ROOT%\out\market_ops_candidate\WAR_M52_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "FAILURE_STAGE=%RUNTIME_STAGE%\CrashDumps\FailureBundles"
set "RETENTION_STAGE=%RUNTIME_STAGE%\Logs\Retention"
set "RC_STAGE=%RUNTIME_STAGE%\Logs\ReleaseCandidate"
set "LAUNCHER_STAGE=%RUNTIME_STAGE%\Logs\Launcher"
set "INSTALLER_LOG_STAGE=%RUNTIME_STAGE%\Logs\Installer"
set "UPDATER_LOG_STAGE=%RUNTIME_STAGE%\Logs\Updater"
set "ONBOARDING_LOG_STAGE=%RUNTIME_STAGE%\Logs\Onboarding"
set "LIVEOPS_LOG_STAGE=%RUNTIME_STAGE%\Logs\LiveOps"
set "RELEASE_MGMT_LOG_STAGE=%RUNTIME_STAGE%\Logs\ReleaseManagement"
set "MANIFEST_PATH=%STAGE_ROOT%\market_ops_candidate_manifest.txt"

if not exist "%SOLUTION_FILE%" (
    echo [M52] ERROR: WAR.sln not found at "%SOLUTION_FILE%".
    exit /b 1
)

set "VSWHERE_EXE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE_EXE%" (
    echo [M52] ERROR: vswhere.exe not found at "%VSWHERE_EXE%".
    exit /b 1
)

set "MSBUILD_EXE="
for /f "usebackq delims=" %%I in (`"%VSWHERE_EXE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    if not defined MSBUILD_EXE set "MSBUILD_EXE=%%I"
)

if not defined MSBUILD_EXE (
    echo [M52] ERROR: MSBuild.exe could not be resolved via vswhere.exe.
    exit /b 1
)

echo [M52] Rebuilding WAR %CONFIG%^|%PLATFORM%...
"%MSBUILD_EXE%" "%SOLUTION_FILE%" /m /nologo /t:Rebuild /p:Configuration=%CONFIG%;Platform=%PLATFORM%
if errorlevel 1 (
    echo [M52] ERROR: build failed.
    exit /b 1
)

if not exist "%CLIENT_EXE_PATH%" (
    echo [M52] ERROR: expected client executable missing at "%CLIENT_EXE_PATH%".
    exit /b 1
)

if not exist "%SERVER_EXE_PATH%" (
    echo [M52] ERROR: expected host executable missing at "%SERVER_EXE_PATH%".
    exit /b 1
)

call :prepare_stage_root
if errorlevel 1 exit /b 1

mkdir "%STAGE_ROOT%" || exit /b 1
mkdir "%STAGE_ROOT%\assets" || exit /b 1
mkdir "%STAGE_ROOT%\Environment" || exit /b 1
mkdir "%STAGE_ROOT%\BetaContent" || exit /b 1
mkdir "%STAGE_ROOT%\Launcher" || exit /b 1
mkdir "%STAGE_ROOT%\Installer" || exit /b 1
mkdir "%STAGE_ROOT%\Onboarding" || exit /b 1
mkdir "%STAGE_ROOT%\LiveOps" || exit /b 1
mkdir "%STAGE_ROOT%\ReleaseManagement" || exit /b 1
mkdir "%RUNTIME_STAGE%\Config" || exit /b 1
mkdir "%RUNTIME_STAGE%\Logs" || exit /b 1
mkdir "%RETENTION_STAGE%" || exit /b 1
mkdir "%RC_STAGE%" || exit /b 1
mkdir "%LAUNCHER_STAGE%" || exit /b 1
mkdir "%INSTALLER_LOG_STAGE%" || exit /b 1
mkdir "%UPDATER_LOG_STAGE%" || exit /b 1
mkdir "%ONBOARDING_LOG_STAGE%" || exit /b 1
mkdir "%LIVEOPS_LOG_STAGE%" || exit /b 1
mkdir "%RELEASE_MGMT_LOG_STAGE%" || exit /b 1
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
if exist "%REPO_ROOT%\assets" xcopy /y /i /e "%REPO_ROOT%\assets" "%STAGE_ROOT%\assets\" >nul 2>nul
if exist "%REPO_ROOT%\Environment" xcopy /y /i /e "%REPO_ROOT%\Environment" "%STAGE_ROOT%\Environment\" >nul 2>nul
if exist "%REPO_ROOT%\BetaContent" xcopy /y /i /e "%REPO_ROOT%\BetaContent" "%STAGE_ROOT%\BetaContent\" >nul 2>nul
if exist "%REPO_ROOT%\Launcher" xcopy /y /i /e "%REPO_ROOT%\Launcher" "%STAGE_ROOT%\Launcher\" >nul 2>nul
if exist "%REPO_ROOT%\Installer" xcopy /y /i /e "%REPO_ROOT%\Installer" "%STAGE_ROOT%\Installer\" >nul 2>nul
if exist "%REPO_ROOT%\Onboarding" xcopy /y /i /e "%REPO_ROOT%\Onboarding" "%STAGE_ROOT%\Onboarding\" >nul 2>nul
if exist "%REPO_ROOT%\LiveOps" xcopy /y /i /e "%REPO_ROOT%\LiveOps" "%STAGE_ROOT%\LiveOps\" >nul 2>nul
if exist "%REPO_ROOT%\ReleaseManagement" xcopy /y /i /e "%REPO_ROOT%\ReleaseManagement" "%STAGE_ROOT%\ReleaseManagement\" >nul 2>nul

for %%F in (
    "README.md"
    "Docs/Validation/VALIDATION_EVIDENCE_M52.txt"
    "Milestones\M52_Live_Ops_Tooling_Analytics_Hardening_Release_Management.md"
    "scripts\launch_headless_host_win64.bat"
    "scripts\launch_local_client_against_host_win64.bat"
    "scripts\smoke_test_headless_host_win64.bat"
    "scripts\smoke_test_local_demo_win64.bat"
    "scripts\open_market_ops_candidate_folder_win64.bat"
) do (
    if exist "%REPO_ROOT%\%%~F" copy /y "%REPO_ROOT%\%%~F" "%STAGE_ROOT%\%%~nxF" >nul
)

(
    echo WAR Market Ops Candidate Manifest
    echo Milestone: M52 - Live-Ops Tooling / Analytics Hardening / Release Management
    echo Configuration: %CONFIG%
    echo Platform: %PLATFORM%
    echo Stage root: %STAGE_ROOT%
    echo Client executable: %STAGE_ROOT%\WAR.exe
    echo Host executable: %STAGE_ROOT%\WARServer.exe
    echo Deployable environment root: %STAGE_ROOT%\Environment
    echo LiveOps root: %STAGE_ROOT%\LiveOps
    echo ReleaseManagement root: %STAGE_ROOT%\ReleaseManagement
    echo Mutable runtime root: %RUNTIME_STAGE%
    echo LiveOps log root: %LIVEOPS_LOG_STAGE%
    echo ReleaseManagement log root: %RELEASE_MGMT_LOG_STAGE%
    echo Preserved aggregate validation is deferred to M59
) > "%MANIFEST_PATH%"

echo [M52] Market ops candidate package staged at "%STAGE_ROOT%".
exit /b 0

:prepare_stage_root
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
call :wait_for_unlock
if not exist "%STAGE_ROOT%" exit /b 0

set /a STAGE_ATTEMPT=0
:remove_stage_root
rmdir /s /q "%STAGE_ROOT%" >nul 2>nul
if not exist "%STAGE_ROOT%" exit /b 0
set /a STAGE_ATTEMPT+=1
if %STAGE_ATTEMPT% GEQ 10 (
    echo [M52] ERROR: unable to clear existing stage root "%STAGE_ROOT%".
    exit /b 1
)
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
call :wait_for_unlock
goto :remove_stage_root

:wait_for_unlock
ping 127.0.0.1 -n 3 >nul
exit /b 0
