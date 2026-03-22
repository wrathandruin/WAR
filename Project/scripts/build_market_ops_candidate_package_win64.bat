@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
if not exist "%REPO_ROOT%\WAR.sln" (
    for %%I in ("%REPO_ROOT%\..") do set "REPO_ROOT=%%~fI"
)
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "PLATFORM=x64"

set "SOLUTION_FILE=%REPO_ROOT%\WAR.sln"
set "CLIENT_OUTPUT_DIR=%REPO_ROOT%\Bin64\%CONFIG%\split\desktop"
set "SERVER_OUTPUT_DIR=%REPO_ROOT%\Bin64\%CONFIG%\split\server"
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
set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\Project\scripts"
if not exist "%SOURCE_SCRIPTS_ROOT%" set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\scripts"

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
mkdir "%STAGE_ROOT%\Docs" || exit /b 1

copy /y "%CLIENT_EXE_PATH%" "%STAGE_ROOT%\WAR.exe" >nul || exit /b 1
copy /y "%SERVER_EXE_PATH%" "%STAGE_ROOT%\WARServer.exe" >nul || exit /b 1
if exist "%CLIENT_PDB_PATH%" copy /y "%CLIENT_PDB_PATH%" "%STAGE_ROOT%\WAR.pdb" >nul
if exist "%SERVER_PDB_PATH%" copy /y "%SERVER_PDB_PATH%" "%STAGE_ROOT%\WARServer.pdb" >nul
call :copy_directory "%REPO_ROOT%\Assets" "%STAGE_ROOT%\assets" "assets" "%REPO_ROOT%\Assets\imported_assets"
if errorlevel 1 exit /b 1
call :copy_source_environment "%STAGE_ROOT%\Environment"
if errorlevel 1 exit /b 1
call :copy_source_manifest_directory "BetaContent" "%STAGE_ROOT%\BetaContent"
if errorlevel 1 exit /b 1
call :copy_source_manifest_directory "Launcher" "%STAGE_ROOT%\Launcher"
if errorlevel 1 exit /b 1
call :copy_source_manifest_directory "Installer" "%STAGE_ROOT%\Installer"
if errorlevel 1 exit /b 1
call :copy_source_manifest_directory "Onboarding" "%STAGE_ROOT%\Onboarding"
if errorlevel 1 exit /b 1
call :copy_source_manifest_directory "LiveOps" "%STAGE_ROOT%\LiveOps"
if errorlevel 1 exit /b 1
call :copy_source_manifest_directory "ReleaseManagement" "%STAGE_ROOT%\ReleaseManagement"
if errorlevel 1 exit /b 1

for %%F in (
    "README.md"
    "scripts\launch_headless_host_win64.bat"
    "scripts\launch_local_client_against_host_win64.bat"
    "scripts\smoke_test_headless_host_win64.bat"
    "scripts\smoke_test_local_demo_win64.bat"
    "scripts\open_market_ops_candidate_folder_win64.bat"
) do (
    call :copy_repo_relative "%%~F" "%STAGE_ROOT%\%%~nxF"
)

call :copy_repo_relative "Docs\Planning\WAR Alpha Handoff.md" "%STAGE_ROOT%\Docs\WAR Alpha Handoff.md"
call :copy_repo_relative "Docs\Planning\WAR Product Blueprint.md" "%STAGE_ROOT%\Docs\WAR Product Blueprint.md"
call :copy_repo_relative "Docs\Technical\WAR Technical Architecture.md" "%STAGE_ROOT%\Docs\WAR Technical Architecture.md"
call :copy_repo_relative "Docs\Art\WAR Art Direction and Asset Targets.md" "%STAGE_ROOT%\Docs\WAR Art Direction and Asset Targets.md"

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

:copy_source_environment
set "COPY_SOURCE=%REPO_ROOT%\Project\Environment"
if not exist "%COPY_SOURCE%" set "COPY_SOURCE=%REPO_ROOT%\Environment"
call :copy_directory "%COPY_SOURCE%" "%~1" "Environment"
exit /b %ERRORLEVEL%

:copy_repo_relative
set "RELATIVE_PATH=%~1"
set "COPY_DEST=%~2"
set "COPY_SOURCE=%REPO_ROOT%\%RELATIVE_PATH%"
if /I "!RELATIVE_PATH:~0,8!"=="scripts\" set "COPY_SOURCE=%SOURCE_SCRIPTS_ROOT%\!RELATIVE_PATH:~8!"
if exist "!COPY_SOURCE!" copy /y "!COPY_SOURCE!" "%COPY_DEST%" >nul
exit /b 0

:copy_source_manifest_directory
set "COPY_LANE=%~1"
set "COPY_DEST=%~2"
set "COPY_SOURCE=%REPO_ROOT%\Project\SourceManifests\%COPY_LANE%"
if not exist "%COPY_SOURCE%" set "COPY_SOURCE=%REPO_ROOT%\SourceManifests\%COPY_LANE%"
if not exist "%COPY_SOURCE%" set "COPY_SOURCE=%REPO_ROOT%\%COPY_LANE%"
call :copy_directory "%COPY_SOURCE%" "%COPY_DEST%" "%COPY_LANE%"
exit /b %ERRORLEVEL%

:copy_directory
set "COPY_SOURCE=%~1"
set "COPY_DEST=%~2"
set "COPY_LABEL=%~3"
set "COPY_EXCLUDE=%~4"
if not exist "%COPY_SOURCE%" exit /b 0

if "%COPY_EXCLUDE%"=="" (
    robocopy "%COPY_SOURCE%" "%COPY_DEST%" /E /R:2 /W:1 /NFL /NDL /NJH /NJS /NP >nul
) else (
    robocopy "%COPY_SOURCE%" "%COPY_DEST%" /E /XD "%COPY_EXCLUDE%" /R:2 /W:1 /NFL /NDL /NJH /NJS /NP >nul
)
set "ROBOCOPY_EXIT=%ERRORLEVEL%"
if errorlevel 8 (
    echo [M52] ERROR: failed to stage %COPY_LABEL% from "%COPY_SOURCE%" to "%COPY_DEST%" - robocopy exit %ROBOCOPY_EXIT%.
    exit /b 1
)
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
