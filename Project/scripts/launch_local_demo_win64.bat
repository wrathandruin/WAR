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
    echo [M44] ERROR: WAR.exe not found next to the launch script.
    exit /b 1
)

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul

call :resolve_build_channel "%DEMO_ROOT%"
if defined RESOLVED_BUILD_CHANNEL set "WAR_BUILD_CHANNEL=%RESOLVED_BUILD_CHANNEL%"

echo [%DATE% %TIME%] launching %EXE_PATH% >> "%LOG_DIR%\local_demo_launch.txt"
start "WAR Local Demo" "%EXE_PATH%"
exit /b 0

:resolve_build_channel
set "RESOLVED_BUILD_CHANNEL=internal-alpha"
set "CHANNEL_ROOT=%~1"
if exist "%CHANNEL_ROOT%\session_mvp_candidate_manifest.txt" set "RESOLVED_BUILD_CHANNEL=session-mvp-candidate"
if exist "%CHANNEL_ROOT%\market_candidate_manifest.txt" set "RESOLVED_BUILD_CHANNEL=market-candidate"
if exist "%CHANNEL_ROOT%\beta_release_candidate_manifest.txt" set "RESOLVED_BUILD_CHANNEL=beta-candidate"
if exist "%CHANNEL_ROOT%\demo_manifest.txt" set "RESOLVED_BUILD_CHANNEL=local-demo"
exit /b 0
