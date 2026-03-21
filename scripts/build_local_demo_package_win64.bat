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
set "STAGE_ROOT=%REPO_ROOT%\out\local_demo\WAR_M44_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "MANIFEST_PATH=%STAGE_ROOT%\demo_manifest.txt"

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
if exist "%REPO_ROOT%\assets" xcopy /y /i /e "%REPO_ROOT%\assets" "%STAGE_ROOT%\assets\" >nul

for %%F in (
    "README.md"
    "Docs\Wrath and Ruin - M40-M44 Sign-Off Report.md"
    "Docs\Wrath and Ruin - M43 Validation and Acceptance Checklist.md"
    "Docs\Wrath and Ruin - M44 Validation and Acceptance Checklist.md"
    "Milestones\M43_Orbital_Space_Layer_Travel_State_Navigation_Rules.md"
    "Milestones\M44_Docking_Landing_Cross_Layer_Transition_Persistence_Return_Loop.md"
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
    if exist "%REPO_ROOT%\%%~F" copy /y "%REPO_ROOT%\%%~F" "%STAGE_ROOT%\%%~nxF" >nul
)

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
