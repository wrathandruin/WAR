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
set "STAGE_ROOT=%REPO_ROOT%\out\local_demo\WAR_M44_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "MANIFEST_PATH=%STAGE_ROOT%\demo_manifest.txt"
set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\Project\scripts"
if not exist "%SOURCE_SCRIPTS_ROOT%" set "SOURCE_SCRIPTS_ROOT=%REPO_ROOT%\scripts"

if not exist "%SOLUTION_FILE%" (
    echo [M44] ERROR: WAR.sln not found at "%SOLUTION_FILE%".
    exit /b 1
)

set "VSWHERE_EXE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE_EXE%" (
    echo [M44] ERROR: vswhere.exe not found at "%VSWHERE_EXE%".
    exit /b 1
)

set "MSBUILD_EXE="
for /f "usebackq delims=" %%I in (`"%VSWHERE_EXE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    if not defined MSBUILD_EXE set "MSBUILD_EXE=%%I"
)

if not defined MSBUILD_EXE (
    echo [M44] ERROR: MSBuild.exe could not be resolved via vswhere.exe.
    exit /b 1
)

echo [M44] Building WAR %CONFIG%^|%PLATFORM%...
"%MSBUILD_EXE%" "%SOLUTION_FILE%" /m /nologo /t:WAR;WARServer /p:Configuration=%CONFIG%;Platform=%PLATFORM%
if errorlevel 1 (
    echo [M44] ERROR: build failed.
    exit /b 1
)

if not exist "%CLIENT_EXE_PATH%" (
    echo [M44] ERROR: expected client executable missing at "%CLIENT_EXE_PATH%".
    exit /b 1
)

if not exist "%SERVER_EXE_PATH%" (
    echo [M44] ERROR: expected host executable missing at "%SERVER_EXE_PATH%".
    exit /b 1
)

if exist "%STAGE_ROOT%" rmdir /s /q "%STAGE_ROOT%"
mkdir "%STAGE_ROOT%" || exit /b 1
mkdir "%STAGE_ROOT%\assets" || exit /b 1
mkdir "%RUNTIME_STAGE%\Config" || exit /b 1
mkdir "%RUNTIME_STAGE%\Logs" || exit /b 1
mkdir "%RUNTIME_STAGE%\Saves" || exit /b 1
mkdir "%RUNTIME_STAGE%\CrashDumps" || exit /b 1
mkdir "%HOST_STAGE%" || exit /b 1
mkdir "%STAGE_ROOT%\Docs" || exit /b 1

copy /y "%CLIENT_EXE_PATH%" "%STAGE_ROOT%\WAR.exe" >nul || exit /b 1
copy /y "%SERVER_EXE_PATH%" "%STAGE_ROOT%\WARServer.exe" >nul || exit /b 1
if exist "%CLIENT_PDB_PATH%" copy /y "%CLIENT_PDB_PATH%" "%STAGE_ROOT%\WAR.pdb" >nul
if exist "%SERVER_PDB_PATH%" copy /y "%SERVER_PDB_PATH%" "%STAGE_ROOT%\WARServer.pdb" >nul

if exist "%REPO_ROOT%\Assets" (
    robocopy "%REPO_ROOT%\Assets" "%STAGE_ROOT%\assets" /E /XD "%REPO_ROOT%\Assets\imported_assets" /R:2 /W:1 /NFL /NDL /NJH /NJS /NP >nul
    set "ROBOCOPY_EXIT=%ERRORLEVEL%"
    if errorlevel 8 (
        echo [M44] ERROR: failed to stage assets from "%REPO_ROOT%\Assets" to "%STAGE_ROOT%\assets" - robocopy exit !ROBOCOPY_EXIT!.
        exit /b 1
    )
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin" (
    echo [M44] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin" (
    echo [M44] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_color.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin" (
    echo [M44] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\vs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin" (
    echo [M44] ERROR: staged shader binary missing at "%STAGE_ROOT%\assets\shaders\dx11\fs_texture.bin".
    exit /b 1
)

if not exist "%STAGE_ROOT%\assets\textures\world_atlas.png" (
    echo [M44] ERROR: staged atlas texture missing at "%STAGE_ROOT%\assets\textures\world_atlas.png".
    exit /b 1
)

for %%F in (
    "README.md"
    "scripts\launch_local_demo_win64.bat"
    "scripts\launch_headless_host_win64.bat"
    "scripts\launch_local_client_against_host_win64.bat"
    "scripts\smoke_test_headless_host_win64.bat"
    "scripts\smoke_test_local_demo_win64.bat"
    "scripts\acceptance_m43_orbital_space_layer_win64.bat"
    "scripts\acceptance_m43_orbital_space_layer_win64.ps1"
    "scripts\acceptance_m44_return_loop_win64.bat"
    "scripts\acceptance_m44_return_loop_win64.ps1"
) do (
    call :copy_repo_relative "%%~F" "%STAGE_ROOT%\%%~nxF"
)

call :copy_repo_relative "Docs\Planning\WAR Alpha Handoff.md" "%STAGE_ROOT%\Docs\WAR Alpha Handoff.md"
call :copy_repo_relative "Docs\Planning\WAR Product Blueprint.md" "%STAGE_ROOT%\Docs\WAR Product Blueprint.md"
call :copy_repo_relative "Docs\Technical\WAR Technical Architecture.md" "%STAGE_ROOT%\Docs\WAR Technical Architecture.md"
call :copy_repo_relative "Docs\Art\WAR Art Direction and Asset Targets.md" "%STAGE_ROOT%\Docs\WAR Art Direction and Asset Targets.md"

(
    echo WAR Local Demo Manifest
    echo Milestone: M44 - Docking / Landing / Cross-Layer Transition Persistence / Return Loop
    echo Configuration: %CONFIG%
    echo Platform: %PLATFORM%
    echo Stage root: %STAGE_ROOT%
    echo Client executable: %STAGE_ROOT%\WAR.exe
    echo Host executable: %STAGE_ROOT%\WARServer.exe
    echo Runtime root: %RUNTIME_STAGE%
    echo Save path: %RUNTIME_STAGE%\Saves\authoritative_world_primary.txt
    echo Packaged authoritative integration smoke: %STAGE_ROOT%\smoke_test_local_demo_win64.bat
    echo M43 regression script: %STAGE_ROOT%\acceptance_m43_orbital_space_layer_win64.bat
    echo M44 acceptance script: %STAGE_ROOT%\acceptance_m44_return_loop_win64.bat
) > "%MANIFEST_PATH%"

echo [M44] Local demo package staged at "%STAGE_ROOT%".
exit /b 0

:copy_repo_relative
set "RELATIVE_PATH=%~1"
set "COPY_DEST=%~2"
set "COPY_SOURCE=%REPO_ROOT%\%RELATIVE_PATH%"
if /I "!RELATIVE_PATH:~0,8!"=="scripts\" set "COPY_SOURCE=%SOURCE_SCRIPTS_ROOT%\!RELATIVE_PATH:~8!"
if exist "!COPY_SOURCE!" copy /y "!COPY_SOURCE!" "%COPY_DEST%" >nul
exit /b 0
