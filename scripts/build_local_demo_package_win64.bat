@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "PLATFORM=x64"

set "PROJECT_FILE=%REPO_ROOT%\WAR.vcxproj"
set "BUILD_OUTPUT_DIR=%REPO_ROOT%\bin\%CONFIG%"
set "EXE_PATH=%BUILD_OUTPUT_DIR%\WAR.exe"
set "PDB_PATH=%BUILD_OUTPUT_DIR%\WAR.pdb"
set "STAGE_ROOT=%REPO_ROOT%\out\local_demo\WAR_M34_%CONFIG%"
set "RUNTIME_STAGE=%STAGE_ROOT%\runtime"
set "HOST_STAGE=%RUNTIME_STAGE%\Host"
set "MANIFEST_PATH=%STAGE_ROOT%\demo_manifest.txt"

if not exist "%PROJECT_FILE%" (
    echo [M34] ERROR: WAR.vcxproj not found at "%PROJECT_FILE%".
    exit /b 1
)

where msbuild >nul 2>nul
if errorlevel 1 (
    echo [M34] ERROR: msbuild was not found on PATH.
    exit /b 1
)

echo [M34] Building WAR %CONFIG%|%PLATFORM%...
msbuild "%PROJECT_FILE%" /m /nologo /p:Configuration=%CONFIG%;Platform=%PLATFORM%
if errorlevel 1 (
    echo [M34] ERROR: build failed.
    exit /b 1
)

if not exist "%EXE_PATH%" (
    echo [M34] ERROR: expected executable missing at "%EXE_PATH%".
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

copy /y "%EXE_PATH%" "%STAGE_ROOT%\WAR.exe" >nul || exit /b 1
if exist "%PDB_PATH%" copy /y "%PDB_PATH%" "%STAGE_ROOT%\WAR.pdb" >nul
if exist "%REPO_ROOT%\assets" xcopy /y /i /e "%REPO_ROOT%\assets" "%STAGE_ROOT%\assets\" >nul
if exist "%REPO_ROOT%\Docs\Wrath and Ruin - Headless World Host Bootstrap.md" copy /y "%REPO_ROOT%\Docs\Wrath and Ruin - Headless World Host Bootstrap.md" "%STAGE_ROOT%\Docs\" >nul
if exist "%REPO_ROOT%\Milestones\M34_Headless_World_Host_Dedicated_Server_Bootstrap.md" copy /y "%REPO_ROOT%\Milestones\M34_Headless_World_Host_Dedicated_Server_Bootstrap.md" "%STAGE_ROOT%\Docs\" >nul
if exist "%REPO_ROOT%\scripts\launch_local_demo_win64.bat" copy /y "%REPO_ROOT%\scripts\launch_local_demo_win64.bat" "%STAGE_ROOT%\launch_local_demo_win64.bat" >nul
if exist "%REPO_ROOT%\scripts\launch_headless_host_win64.bat" copy /y "%REPO_ROOT%\scripts\launch_headless_host_win64.bat" "%STAGE_ROOT%\launch_headless_host_win64.bat" >nul
if exist "%REPO_ROOT%\scripts\launch_local_client_against_host_win64.bat" copy /y "%REPO_ROOT%\scripts\launch_local_client_against_host_win64.bat" "%STAGE_ROOT%\launch_local_client_against_host_win64.bat" >nul
if exist "%REPO_ROOT%\scripts\smoke_test_headless_host_win64.bat" copy /y "%REPO_ROOT%\scripts\smoke_test_headless_host_win64.bat" "%STAGE_ROOT%\smoke_test_headless_host_win64.bat" >nul
if exist "%REPO_ROOT%\scripts\smoke_test_local_demo_win64.bat" copy /y "%REPO_ROOT%\scripts\smoke_test_local_demo_win64.bat" "%STAGE_ROOT%\smoke_test_local_demo_win64.bat" >nul

(
    echo WAR Local Demo Manifest
    echo Milestone: M34 - Headless World Host / Dedicated Server Bootstrap
    echo Configuration: %CONFIG%
    echo Platform: %PLATFORM%
    echo Stage root: %STAGE_ROOT%
    echo Executable: %STAGE_ROOT%\WAR.exe
    echo Asset root: %STAGE_ROOT%\assets
    echo Runtime root: %RUNTIME_STAGE%
    echo Host runtime root: %HOST_STAGE%
    echo Launch script: %STAGE_ROOT%\launch_local_demo_win64.bat
    echo Host launch script: %STAGE_ROOT%\launch_headless_host_win64.bat
    echo Local client against host: %STAGE_ROOT%\launch_local_client_against_host_win64.bat
    echo Host smoke test script: %STAGE_ROOT%\smoke_test_headless_host_win64.bat
) > "%MANIFEST_PATH%"

echo [M34] Local demo package staged at "%STAGE_ROOT%".
exit /b 0
