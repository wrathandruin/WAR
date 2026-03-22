@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
if not exist "%REPO_ROOT%\WAR.sln" (
    for %%I in ("%REPO_ROOT%\..") do set "REPO_ROOT=%%~fI"
)

set "CONFIG=%WAR_PACKAGE_CONFIG%"
if "%CONFIG%"=="" set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "PLATFORM=x64"

set "CLIENT_OUTPUT_DIR=%REPO_ROOT%\Bin64\%CONFIG%\split\desktop"
set "SERVER_OUTPUT_DIR=%REPO_ROOT%\Bin64\%CONFIG%\split\server"
set "CLIENT_EXE_PATH=%CLIENT_OUTPUT_DIR%\WAR.exe"
set "CLIENT_PDB_PATH=%CLIENT_OUTPUT_DIR%\WAR.pdb"
set "SERVER_EXE_PATH=%SERVER_OUTPUT_DIR%\WARServer.exe"
set "SERVER_PDB_PATH=%SERVER_OUTPUT_DIR%\WARServer.pdb"
set "STAGE_ROOT=%REPO_ROOT%\out\internal_alpha\WAR_M48_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "FAILURE_STAGE=%RUNTIME_STAGE%\CrashDumps\FailureBundles"
set "MANIFEST_PATH=%STAGE_ROOT%\internal_alpha_manifest.txt"
set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\Project\scripts"
if not exist "%SOURCE_SCRIPTS_ROOT%" set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\scripts"

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
ping 127.0.0.1 -n 3 >nul

call :reset_stage_root
if errorlevel 1 exit /b 1

call :ensure_dir "%STAGE_ROOT%"
if errorlevel 1 exit /b 1
call :ensure_dir "%STAGE_ROOT%\assets"
if errorlevel 1 exit /b 1
call :ensure_dir "%STAGE_ROOT%\Environment"
if errorlevel 1 exit /b 1
call :ensure_dir "%RUNTIME_STAGE%\Config"
if errorlevel 1 exit /b 1
call :ensure_dir "%RUNTIME_STAGE%\Logs"
if errorlevel 1 exit /b 1
call :ensure_dir "%RUNTIME_STAGE%\Saves"
if errorlevel 1 exit /b 1
call :ensure_dir "%RUNTIME_STAGE%\CrashDumps"
if errorlevel 1 exit /b 1
call :ensure_dir "%FAILURE_STAGE%\Startup"
if errorlevel 1 exit /b 1
call :ensure_dir "%FAILURE_STAGE%\Runtime"
if errorlevel 1 exit /b 1
call :ensure_dir "%FAILURE_STAGE%\Bootstrap"
if errorlevel 1 exit /b 1
call :ensure_dir "%FAILURE_STAGE%\OperatorTriage"
if errorlevel 1 exit /b 1
call :ensure_dir "%HOST_STAGE%"
if errorlevel 1 exit /b 1
call :ensure_dir "%STAGE_ROOT%\Docs"
if errorlevel 1 exit /b 1

copy /y "%CLIENT_EXE_PATH%" "%STAGE_ROOT%\WAR.exe" >nul || exit /b 1
copy /y "%SERVER_EXE_PATH%" "%STAGE_ROOT%\WARServer.exe" >nul || exit /b 1
if exist "%CLIENT_PDB_PATH%" copy /y "%CLIENT_PDB_PATH%" "%STAGE_ROOT%\WAR.pdb" >nul
if exist "%SERVER_PDB_PATH%" copy /y "%SERVER_PDB_PATH%" "%STAGE_ROOT%\WARServer.pdb" >nul

if exist "%REPO_ROOT%\Assets" (
    robocopy "%REPO_ROOT%\Assets" "%STAGE_ROOT%\assets" /E /XD "%REPO_ROOT%\Assets\imported_assets" /R:2 /W:1 /NFL /NDL /NJH /NJS /NP >nul
    set "ROBOCOPY_EXIT=%ERRORLEVEL%"
    if errorlevel 8 (
        echo [M48] ERROR: failed to stage assets from "%REPO_ROOT%\Assets" to "%STAGE_ROOT%\assets" - robocopy exit !ROBOCOPY_EXIT!.
        exit /b 1
    )
)

set "SOURCE_ENVIRONMENT_ROOT=%REPO_ROOT%\Project\Environment"
if not exist "%SOURCE_ENVIRONMENT_ROOT%" set "SOURCE_ENVIRONMENT_ROOT=%REPO_ROOT%\Environment"

if exist "%SOURCE_ENVIRONMENT_ROOT%" (
    robocopy "%SOURCE_ENVIRONMENT_ROOT%" "%STAGE_ROOT%\Environment" /E /R:2 /W:1 /NFL /NDL /NJH /NJS /NP >nul
    set "ROBOCOPY_EXIT=%ERRORLEVEL%"
    if errorlevel 8 (
        echo [M48] ERROR: failed to stage Environment from "%SOURCE_ENVIRONMENT_ROOT%" to "%STAGE_ROOT%\Environment" - robocopy exit !ROBOCOPY_EXIT!.
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

if not exist "%STAGE_ROOT%\assets\textures\world_atlas.png" (
    echo [M48] ERROR: staged atlas texture missing at "%STAGE_ROOT%\assets\textures\world_atlas.png".
    exit /b 1
)

for %%F in (
    "README.md"
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
    call :copy_repo_relative "%%~F" "%STAGE_ROOT%\%%~nxF"
)

call :copy_repo_relative "Docs\Planning\WAR Alpha Handoff.md" "%STAGE_ROOT%\Docs\WAR Alpha Handoff.md"
call :copy_repo_relative "Docs\Planning\WAR Product Blueprint.md" "%STAGE_ROOT%\Docs\WAR Product Blueprint.md"
call :copy_repo_relative "Docs\Technical\WAR Technical Architecture.md" "%STAGE_ROOT%\Docs\WAR Technical Architecture.md"
call :copy_repo_relative "Docs\Art\WAR Art Direction and Asset Targets.md" "%STAGE_ROOT%\Docs\WAR Art Direction and Asset Targets.md"

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

:reset_stage_root
if not exist "%STAGE_ROOT%" exit /b 0

attrib -r -s -h "%STAGE_ROOT%\*" /s /d >nul 2>nul
rmdir /s /q "%STAGE_ROOT%" >nul 2>nul
if not exist "%STAGE_ROOT%" exit /b 0

echo [M48] WARN: stage root busy, falling back to in-place cleanup for "%STAGE_ROOT%".
call :clear_stage_root_contents "%STAGE_ROOT%"
if errorlevel 1 exit /b 1
exit /b 0

:clear_stage_root_contents
set "TARGET_ROOT=%~1"
if not exist "%TARGET_ROOT%" exit /b 0

attrib -r -s -h "%TARGET_ROOT%\*" /s /d >nul 2>nul
for /f "delims=" %%I in ('dir /b /a "%TARGET_ROOT%" 2^>nul') do (
    if exist "%TARGET_ROOT%\%%~I\NUL" (
        rmdir /s /q "%TARGET_ROOT%\%%~I" >nul 2>nul
    ) else (
        del /f /q "%TARGET_ROOT%\%%~I" >nul 2>nul
    )
)

for /f "delims=" %%I in ('dir /b /a "%TARGET_ROOT%" 2^>nul') do (
    echo [M48] ERROR: stage root cleanup blocked by "%TARGET_ROOT%\%%~I".
    exit /b 1
)

exit /b 0

:ensure_dir
if exist "%~1" exit /b 0
mkdir "%~1" >nul 2>nul
if exist "%~1" exit /b 0
echo [M48] ERROR: unable to create directory "%~1".
exit /b 1

:copy_repo_relative
set "RELATIVE_PATH=%~1"
set "COPY_DEST=%~2"
set "COPY_SOURCE=%REPO_ROOT%\%RELATIVE_PATH%"
if /I "!RELATIVE_PATH:~0,8!"=="scripts\" set "COPY_SOURCE=%SOURCE_SCRIPTS_ROOT%\!RELATIVE_PATH:~8!"
if exist "!COPY_SOURCE!" copy /y "!COPY_SOURCE!" "%COPY_DEST%" >nul
exit /b 0
