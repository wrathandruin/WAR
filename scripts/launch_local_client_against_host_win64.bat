@echo off
setlocal enableextensions

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "EXE_PATH=%DEMO_ROOT%\WAR.exe"
if not exist "%EXE_PATH%" (
    echo [M34] ERROR: WAR.exe not found next to the client/host launch script.
    exit /b 1
)

start "WAR Headless Host" /min "%EXE_PATH%" --headless-host
ping 127.0.0.1 -n 2 >nul
start "WAR Client" "%EXE_PATH%"
exit /b 0
