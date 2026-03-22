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
set "STAGE_ROOT=%REPO_ROOT%\out\session_mvp_candidate\WAR_M59_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "MANIFEST_PATH=%STAGE_ROOT%\session_mvp_candidate_manifest.txt"
set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\Project\scripts"
if not exist "%SOURCE_SCRIPTS_ROOT%" set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\scripts"

if not exist "%SOLUTION_FILE%" (
    echo [M59] ERROR: WAR.sln not found at "%SOLUTION_FILE%".
    exit /b 1
)

set "VSWHERE_EXE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE_EXE%" (
    echo [M59] ERROR: vswhere.exe not found at "%VSWHERE_EXE%".
    exit /b 1
)

set "MSBUILD_EXE="
for /f "usebackq delims=" %%I in (`"%VSWHERE_EXE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    if not defined MSBUILD_EXE set "MSBUILD_EXE=%%I"
)

if not defined MSBUILD_EXE (
    echo [M59] ERROR: MSBuild.exe could not be resolved via vswhere.exe.
    exit /b 1
)

echo [M59] Rebuilding WAR %CONFIG%^|%PLATFORM%...
"%MSBUILD_EXE%" "%SOLUTION_FILE%" /m /nologo /t:Rebuild /p:Configuration=%CONFIG%;Platform=%PLATFORM%
if errorlevel 1 (
    echo [M59] ERROR: build failed.
    exit /b 1
)

if not exist "%CLIENT_EXE_PATH%" (
    echo [M59] ERROR: expected client executable missing at "%CLIENT_EXE_PATH%".
    exit /b 1
)

if not exist "%SERVER_EXE_PATH%" (
    echo [M59] ERROR: expected host executable missing at "%SERVER_EXE_PATH%".
    exit /b 1
)

call :prepare_stage_root
if errorlevel 1 exit /b 1

mkdir "%STAGE_ROOT%" || exit /b 1
mkdir "%STAGE_ROOT%\assets" || exit /b 1
mkdir "%STAGE_ROOT%\Content" || exit /b 1
mkdir "%STAGE_ROOT%\Environment" || exit /b 1
mkdir "%STAGE_ROOT%\Docs" || exit /b 1
mkdir "%RUNTIME_STAGE%\Config" || exit /b 1
mkdir "%RUNTIME_STAGE%\Logs" || exit /b 1
mkdir "%RUNTIME_STAGE%\Saves" || exit /b 1
mkdir "%RUNTIME_STAGE%\CrashDumps" || exit /b 1
mkdir "%HOST_STAGE%" || exit /b 1

copy /y "%CLIENT_EXE_PATH%" "%STAGE_ROOT%\WAR.exe" >nul || exit /b 1
copy /y "%SERVER_EXE_PATH%" "%STAGE_ROOT%\WARServer.exe" >nul || exit /b 1
if exist "%CLIENT_PDB_PATH%" copy /y "%CLIENT_PDB_PATH%" "%STAGE_ROOT%\WAR.pdb" >nul
if exist "%SERVER_PDB_PATH%" copy /y "%SERVER_PDB_PATH%" "%STAGE_ROOT%\WARServer.pdb" >nul

call :copy_directory "%REPO_ROOT%\Assets" "%STAGE_ROOT%\assets" "assets" "%REPO_ROOT%\Assets\imported_assets"
if errorlevel 1 exit /b 1
call :copy_directory "%REPO_ROOT%\Content" "%STAGE_ROOT%\Content" "Content"
if errorlevel 1 exit /b 1
set "SOURCE_ENVIRONMENT_ROOT=%REPO_ROOT%\Project\Environment"
if not exist "%SOURCE_ENVIRONMENT_ROOT%" set "SOURCE_ENVIRONMENT_ROOT=%REPO_ROOT%\Environment"
call :copy_directory "%SOURCE_ENVIRONMENT_ROOT%" "%STAGE_ROOT%\Environment" "Environment"
if errorlevel 1 exit /b 1

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin" (
    echo [M59] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin" (
    echo [M59] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin" (
    echo [M59] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin" (
    echo [M59] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\textures\world_atlas.png" (
    echo [M59] ERROR: staged atlas texture missing at "%STAGE_ROOT%\assets\textures\world_atlas.png".
    exit /b 1
)

if not exist "%STAGE_ROOT%\Content\SessionMvp\session_mvp_authoring.txt" (
    echo [M59] ERROR: staged authoring content missing at "%STAGE_ROOT%\Content\SessionMvp\session_mvp_authoring.txt".
    exit /b 1
)

if not exist "%STAGE_ROOT%\Content\SessionMvp\session_mvp_art_manifest.txt" (
    echo [M59] ERROR: staged art manifest missing at "%STAGE_ROOT%\Content\SessionMvp\session_mvp_art_manifest.txt".
    exit /b 1
)

for %%F in (
    "README.md"
    "scripts\launch_headless_host_win64.bat"
    "scripts\launch_local_demo_win64.bat"
    "scripts\launch_local_client_against_host_win64.bat"
    "scripts\smoke_test_headless_host_win64.bat"
    "scripts\smoke_test_local_demo_win64.bat"
    "scripts\validate_m54_session_mvp_win64.bat"
    "scripts\validate_m55_session_mvp_win64.bat"
    "scripts\validate_m56_session_mvp_win64.bat"
    "scripts\validate_m57_session_mvp_win64.bat"
    "scripts\validate_m58_session_mvp_win64.bat"
    "scripts\validate_m59_session_mvp_win64.bat"
    "scripts\open_session_mvp_candidate_folder_win64.bat"
) do (
    call :copy_repo_relative "%%~F" "%STAGE_ROOT%\%%~nxF"
)

call :copy_repo_relative "Docs\Planning\WAR Alpha Handoff.md" "%STAGE_ROOT%\Docs\WAR Alpha Handoff.md"
call :copy_repo_relative "Docs\Planning\WAR Product Blueprint.md" "%STAGE_ROOT%\Docs\WAR Product Blueprint.md"
call :copy_repo_relative "Docs\Technical\WAR Technical Architecture.md" "%STAGE_ROOT%\Docs\WAR Technical Architecture.md"
call :copy_repo_relative "Docs\Art\WAR Art Direction and Asset Targets.md" "%STAGE_ROOT%\Docs\WAR Art Direction and Asset Targets.md"

(
    echo WAR Session MVP Candidate Manifest
    echo Milestone: M59 - Multi-User Scenario Beat / Cooperation And Communication Validation
    echo Configuration: %CONFIG%
    echo Platform: %PLATFORM%
    echo Stage root: %STAGE_ROOT%
    echo Client executable: %STAGE_ROOT%\WAR.exe
    echo Host executable: %STAGE_ROOT%\WARServer.exe
    echo Runtime root: %RUNTIME_STAGE%
    echo Content root: %STAGE_ROOT%\Content
    echo Docs root: %STAGE_ROOT%\Docs
    echo Automated aggregate validation wrapper: %SOURCE_SCRIPTS_ROOT%\validate_m59_session_mvp_package_win64.bat
    echo Manual cooperation reference: %STAGE_ROOT%\Content\SessionMvp\session_mvp_authoring.txt
    echo Product handoff reference: %STAGE_ROOT%\Docs\WAR Alpha Handoff.md
) > "%MANIFEST_PATH%"

echo [M59] Session MVP candidate package staged at "%STAGE_ROOT%".
exit /b 0

:copy_repo_relative
set "RELATIVE_PATH=%~1"
set "COPY_DEST=%~2"
set "COPY_SOURCE=%REPO_ROOT%\%RELATIVE_PATH%"
if /I "!RELATIVE_PATH:~0,8!"=="scripts\" set "COPY_SOURCE=%SOURCE_SCRIPTS_ROOT%\!RELATIVE_PATH:~8!"
if exist "!COPY_SOURCE!" copy /y "!COPY_SOURCE!" "%COPY_DEST%" >nul
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
    echo [M59] ERROR: failed to stage !COPY_LABEL! from "!COPY_SOURCE!" to "!COPY_DEST!" - robocopy exit !ROBOCOPY_EXIT!.
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
    echo [M59] ERROR: unable to clear existing stage root "%STAGE_ROOT%".
    exit /b 1
)
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
call :wait_for_unlock
goto :remove_stage_root

:wait_for_unlock
ping 127.0.0.1 -n 3 >nul
exit /b 0
