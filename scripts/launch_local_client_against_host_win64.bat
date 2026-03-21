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

set "LOCAL_RUNTIME_ROOT=%DEMO_ROOT%\runtime"
if not exist "%LOCAL_RUNTIME_ROOT%" set "LOCAL_RUNTIME_ROOT=%DEMO_ROOT%\Runtime"
set "TARGET_RUNTIME_ROOT=%~1"
if "%TARGET_RUNTIME_ROOT%"=="" set "TARGET_RUNTIME_ROOT=%LOCAL_RUNTIME_ROOT%"
set "TARGET_NAME=%~2"
if "%TARGET_NAME%"=="" set "TARGET_NAME=localhost-fallback"
set "LANE_MODE=%~3"
if "%LANE_MODE%"=="" (
    if /I "%TARGET_RUNTIME_ROOT%"=="%LOCAL_RUNTIME_ROOT%" (
        set "LANE_MODE=localhost-fallback"
    ) else (
        set "LANE_MODE=hosted-bootstrap"
    )
)

set "TRANSPORT_KIND=file-backed-localhost-fallback"
if /I "%LANE_MODE%"=="hosted-bootstrap" set "TRANSPORT_KIND=file-backed-hosted-bootstrap"

if not exist "%CLIENT_EXE_PATH%" (
    echo [M45] ERROR: WAR.exe not found next to the client/host launch script.
    exit /b 1
)

if not exist "%HOST_EXE_PATH%" (
    echo [M45] ERROR: host executable not found next to the client/host launch script.
    exit /b 1
)

set "WAR_RUNTIME_ROOT=%TARGET_RUNTIME_ROOT%"
set "WAR_CONNECT_TARGET_NAME=%TARGET_NAME%"
set "WAR_CONNECT_TRANSPORT=%TRANSPORT_KIND%"
set "WAR_CONNECT_LANE_MODE=%LANE_MODE%"
set "WAR_BUILD_CHANNEL=internal-alpha"

if /I "%LANE_MODE%"=="localhost-fallback" (
    start "WAR Headless Host" /min "%HOST_EXE_PATH%" %HOST_ARGS%
    ping 127.0.0.1 -n 2 >nul
) else (
    echo [M45] Client connecting to hosted bootstrap target "%TARGET_NAME%" via runtime root "%TARGET_RUNTIME_ROOT%".
)

start "WAR Client" "%CLIENT_EXE_PATH%"
exit /b 0
