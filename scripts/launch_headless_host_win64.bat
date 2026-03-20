@echo off
setlocal enableextensions

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "EXE_PATH=%DEMO_ROOT%\WAR.exe"
set "RUNTIME_ROOT=%DEMO_ROOT%\runtime"
if not exist "%RUNTIME_ROOT%" set "RUNTIME_ROOT=%DEMO_ROOT%\Runtime"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"

if not exist "%EXE_PATH%" (
    echo [M34] ERROR: WAR.exe not found next to the host launch script.
    exit /b 1
)

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul

echo [%DATE% %TIME%] launching headless host %EXE_PATH% >> "%LOG_DIR%\headless_host_launch.txt"
start "WAR Headless Host" /min "%EXE_PATH%" --headless-host
exit /b 0
