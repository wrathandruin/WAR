@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%DEMO_ROOT%\runtime"
if not exist "%RUNTIME_ROOT%" set "RUNTIME_ROOT=%DEMO_ROOT%\Runtime"
set "HOST_ROOT=%RUNTIME_ROOT%\Host"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "STATUS_FILE=%HOST_ROOT%\headless_host_status.txt"
set "REPORT_PATH=%LOG_DIR%\m45_headless_host_smoke_test.txt"
set "DETAILS_PATH=%LOG_DIR%\m45_headless_host_smoke_test_details.txt"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul
if not exist "%HOST_ROOT%" mkdir "%HOST_ROOT%" >nul 2>nul
if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul
if exist "%STATUS_FILE%" del /q "%STATUS_FILE%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "localhost-fallback" "localhost-fallback"
if errorlevel 1 (
    echo [FAIL] Host launch script returned failure>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%STATUS_FILE%" "Headless host status file"
if "%FAILED%"=="1" goto :cleanup

call :require_line "%STATUS_FILE%" "state=running" "Host reached running state"
call :require_line "%STATUS_FILE%" "host_authority_active=yes" "Host authority visible"
call :require_line "%STATUS_FILE%" "protocol_version=2" "Protocol version visible"
call :require_line "%STATUS_FILE%" "transport_kind=file-backed-localhost-fallback" "Localhost transport visible"
call :require_line "%STATUS_FILE%" "connect_lane_mode=localhost-fallback" "Lane mode visible"
call :require_prefix "%STATUS_FILE%" "host_instance_id=host-" "Host instance identity visible"
call :require_prefix "%STATUS_FILE%" "session_id=session-" "Session identity visible"
call :require_prefix "%STATUS_FILE%" "build_identity=" "Build identity visible"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M45 Headless Host Smoke Test
    if "%FAILED%"=="0" (echo Result: PASS) else (echo Result: FAIL)
    echo.
    if exist "%DETAILS_PATH%" type "%DETAILS_PATH%"
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
if "%FAILED%"=="0" exit /b 0
exit /b 1

:wait_for_file
set "WAIT_PATH=%~1"
set /a WAIT_ATTEMPT=0
:wait_loop
if exist "%WAIT_PATH%" exit /b 0
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 20 (
    echo [FAIL] %~2 missing: %WAIT_PATH%>> "%DETAILS_PATH%"
    set "FAILED=1"
    exit /b 0
)
ping 127.0.0.1 -n 2 >nul
goto :wait_loop

:require_line
findstr /b /c:"%~2" "%~1" >nul
if errorlevel 1 (
    echo [FAIL] %~3>> "%DETAILS_PATH%"
    set "FAILED=1"
) else (
    echo [PASS] %~3>> "%DETAILS_PATH%"
)
exit /b 0

:require_prefix
findstr /b /c:"%~2" "%~1" >nul
if errorlevel 1 (
    echo [FAIL] %~3>> "%DETAILS_PATH%"
    set "FAILED=1"
) else (
    echo [PASS] %~3>> "%DETAILS_PATH%"
)
exit /b 0
