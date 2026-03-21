@echo off
setlocal enableextensions

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "CLIENT_EXE_PATH=%DEMO_ROOT%\WAR.exe"
set "HOST_EXE_PATH=%DEMO_ROOT%\WARServer.exe"
set "HOST_ARGS="
if not exist "%HOST_EXE_PATH%" (
    set "HOST_EXE_PATH=%DEMO_ROOT%\WAR.exe"
    set "HOST_ARGS=--headless-host"
)

if not exist "%CLIENT_EXE_PATH%" (
    echo [M40] ERROR: WAR.exe not found next to the client/host launch script.
    exit /b 1
)

if not exist "%HOST_EXE_PATH%" (
    echo [M40] ERROR: host executable not found next to the client/host launch script.
    exit /b 1
)

start "WAR Headless Host" /min "%HOST_EXE_PATH%" %HOST_ARGS%
ping 127.0.0.1 -n 2 >nul
start "WAR Client" "%CLIENT_EXE_PATH%"
exit /b 0
