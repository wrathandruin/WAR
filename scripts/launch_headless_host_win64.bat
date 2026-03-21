@echo off
setlocal enableextensions

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "HOST_EXE_PATH=%DEMO_ROOT%\WARServer.exe"
set "HOST_ARGS="
if not exist "%HOST_EXE_PATH%" (
    set "HOST_EXE_PATH=%DEMO_ROOT%\WAR.exe"
    set "HOST_ARGS=--headless-host"
)
set "RUNTIME_ROOT=%DEMO_ROOT%\runtime"
if not exist "%RUNTIME_ROOT%" set "RUNTIME_ROOT=%DEMO_ROOT%\Runtime"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"

if not exist "%HOST_EXE_PATH%" (
    echo [M40] ERROR: no host executable found next to the host launch script.
    exit /b 1
)

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul

echo [%DATE% %TIME%] launching headless host %HOST_EXE_PATH% %HOST_ARGS% >> "%LOG_DIR%\headless_host_launch.txt"
start "WAR Headless Host" /min "%HOST_EXE_PATH%" %HOST_ARGS%
exit /b 0
