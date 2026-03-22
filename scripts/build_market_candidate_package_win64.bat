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
set "STAGE_ROOT=%REPO_ROOT%\out\market_candidate\WAR_M50_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "FAILURE_STAGE=%RUNTIME_STAGE%\CrashDumps\FailureBundles"
set "RETENTION_STAGE=%RUNTIME_STAGE%\Logs\Retention"
set "RC_STAGE=%RUNTIME_STAGE%\Logs\ReleaseCandidate"
set "LAUNCHER_STAGE=%RUNTIME_STAGE%\Logs\Launcher"
set "INSTALLER_LOG_STAGE=%RUNTIME_STAGE%\Logs\Installer"
set "UPDATER_LOG_STAGE=%RUNTIME_STAGE%\Logs\Updater"
set "MANIFEST_PATH=%STAGE_ROOT%\market_candidate_manifest.txt"

if not exist "%SOLUTION_FILE%" (
    echo [M50] ERROR: WAR.sln not found at "%SOLUTION_FILE%".
    exit /b 1
)

set "VSWHERE_EXE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE_EXE%" (
    echo [M50] ERROR: vswhere.exe not found at "%VSWHERE_EXE%".
    exit /b 1
)

set "MSBUILD_EXE="
for /f "usebackq delims=" %%I in (`"%VSWHERE_EXE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    if not defined MSBUILD_EXE set "MSBUILD_EXE=%%I"
)

if not defined MSBUILD_EXE (
    echo [M50] ERROR: MSBuild.exe could not be resolved via vswhere.exe.
    exit /b 1
)

echo [M50] Rebuilding WAR %CONFIG%^|%PLATFORM%...
"%MSBUILD_EXE%" "%SOLUTION_FILE%" /m /nologo /t:Rebuild /p:Configuration=%CONFIG%;Platform=%PLATFORM%
if errorlevel 1 (
    echo [M50] ERROR: build failed.
    exit /b 1
)

if not exist "%CLIENT_EXE_PATH%" (
    echo [M50] ERROR: expected client executable missing at "%CLIENT_EXE_PATH%".
    exit /b 1
)

if not exist "%SERVER_EXE_PATH%" (
    echo [M50] ERROR: expected host executable missing at "%SERVER_EXE_PATH%".
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
mkdir "%RUNTIME_STAGE%\Config" || exit /b 1
mkdir "%RUNTIME_STAGE%\Logs" || exit /b 1
mkdir "%RETENTION_STAGE%" || exit /b 1
mkdir "%RC_STAGE%" || exit /b 1
mkdir "%LAUNCHER_STAGE%" || exit /b 1
mkdir "%INSTALLER_LOG_STAGE%" || exit /b 1
mkdir "%UPDATER_LOG_STAGE%" || exit /b 1
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
call :copy_directory "%REPO_ROOT%\assets" "%STAGE_ROOT%\assets" "assets" "%REPO_ROOT%\assets\imported_assets"
if errorlevel 1 exit /b 1
call :copy_directory "%REPO_ROOT%\Environment" "%STAGE_ROOT%\Environment" "Environment"
if errorlevel 1 exit /b 1
call :copy_directory "%REPO_ROOT%\BetaContent" "%STAGE_ROOT%\BetaContent" "BetaContent"
if errorlevel 1 exit /b 1
call :copy_directory "%REPO_ROOT%\Launcher" "%STAGE_ROOT%\Launcher" "Launcher"
if errorlevel 1 exit /b 1
call :copy_directory "%REPO_ROOT%\Installer" "%STAGE_ROOT%\Installer" "Installer"
if errorlevel 1 exit /b 1

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin" (
    echo [M50] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin" (
    echo [M50] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin" (
    echo [M50] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin" (
    echo [M50] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\textures\world_atlas.bmp" (
    echo [M50] ERROR: staged atlas texture missing at "%STAGE_ROOT%\assets\textures\world_atlas.bmp".
    exit /b 1
)

if not exist "%STAGE_ROOT%\BetaContent\m49_beta_content_manifest.txt" (
    echo [M50] ERROR: staged beta content manifest missing at "%STAGE_ROOT%\BetaContent\m49_beta_content_manifest.txt".
    exit /b 1
)

if not exist "%STAGE_ROOT%\Launcher\m50_launcher_manifest.txt" (
    echo [M50] ERROR: staged launcher manifest missing at "%STAGE_ROOT%\Launcher\m50_launcher_manifest.txt".
    exit /b 1
)

if not exist "%STAGE_ROOT%\Installer\m50_installer_manifest.txt" (
    echo [M50] ERROR: staged installer manifest missing at "%STAGE_ROOT%\Installer\m50_installer_manifest.txt".
    exit /b 1
)

if not exist "%STAGE_ROOT%\Installer\m50_update_channel_manifest.txt" (
    echo [M50] ERROR: staged update channel manifest missing at "%STAGE_ROOT%\Installer\m50_update_channel_manifest.txt".
    exit /b 1
)

for %%F in (
    "README.md"
    "VALIDATION_EVIDENCE_M50.txt"
    "Milestones\M50_Launcher_Installer_Update_Productionization.md"
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
    "scripts\validate_m49_beta_content_scale_win64.bat"
    "scripts\validate_m49_retention_expansion_win64.bat"
    "scripts\validate_m49_release_candidate_discipline_win64.bat"
    "scripts\validate_m50_launcher_productionization_win64.bat"
    "scripts\validate_m50_installer_productionization_win64.bat"
    "scripts\validate_m50_update_productionization_win64.bat"
    "scripts\validate_m50_market_candidate_package_win64.bat"
    "scripts\open_market_candidate_folder_win64.bat"
) do (
    if exist "%REPO_ROOT%\%%~F" copy /y "%REPO_ROOT%\%%~F" "%STAGE_ROOT%\%%~nxF" >nul
)

(
    echo WAR Market Candidate Manifest
    echo Milestone: M50 - Launcher / Installer / Update Productionization
    echo Configuration: %CONFIG%
    echo Platform: %PLATFORM%
    echo Stage root: %STAGE_ROOT%
    echo Client executable: %STAGE_ROOT%\WAR.exe
    echo Host executable: %STAGE_ROOT%\WARServer.exe
    echo Deployable environment root: %STAGE_ROOT%\Environment
    echo Beta content root: %STAGE_ROOT%\BetaContent
    echo Launcher root: %STAGE_ROOT%\Launcher
    echo Installer root: %STAGE_ROOT%\Installer
    echo Mutable runtime root: %RUNTIME_STAGE%
    echo Retention log root: %RETENTION_STAGE%
    echo Release candidate log root: %RC_STAGE%
    echo Launcher log root: %LAUNCHER_STAGE%
    echo Installer log root: %INSTALLER_LOG_STAGE%
    echo Updater log root: %UPDATER_LOG_STAGE%
    echo Failure bundle root: %FAILURE_STAGE%
    echo Preserved validation lanes:
    echo - M45 hosted bootstrap
    echo - M46 environment identity / secrets / runtime hygiene
    echo - M47 ticket issue / denial / reconnect
    echo - M48 failure bundle capture / operator triage artifacts
    echo - M49 beta content / retention / RC discipline
    echo New M50 validation lanes:
    echo - validate_m50_launcher_productionization_win64.bat
    echo - validate_m50_installer_productionization_win64.bat
    echo - validate_m50_update_productionization_win64.bat
    echo - validate_m50_market_candidate_package_win64.bat
) > "%MANIFEST_PATH%"

echo [M50] Market candidate package staged at "%STAGE_ROOT%".
exit /b 0

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
    echo [M50] ERROR: failed to stage !COPY_LABEL! from "!COPY_SOURCE!" to "!COPY_DEST!" - robocopy exit !ROBOCOPY_EXIT!.
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
    echo [M50] ERROR: unable to clear existing stage root "%STAGE_ROOT%".
    exit /b 1
)
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
call :wait_for_unlock
goto :remove_stage_root

:wait_for_unlock
ping 127.0.0.1 -n 3 >nul
exit /b 0
