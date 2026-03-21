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
set "CLIENT_STATUS_FILE=%LOG_DIR%\client_replication_status.txt"
set "HOST_STATUS_FILE=%HOST_ROOT%\headless_host_status.txt"
set "REPORT_PATH=%LOG_DIR%\m45_local_demo_smoke_test.txt"
set "DETAILS_PATH=%LOG_DIR%\m45_local_demo_smoke_test_details.txt"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul
if not exist "%HOST_ROOT%" mkdir "%HOST_ROOT%" >nul 2>nul
if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul
if exist "%CLIENT_STATUS_FILE%" del /q "%CLIENT_STATUS_FILE%" >nul 2>nul
if exist "%HOST_STATUS_FILE%" del /q "%HOST_STATUS_FILE%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_local_client_against_host_win64.bat" "%RUNTIME_ROOT%" "localhost-fallback" "localhost-fallback"
if errorlevel 1 (
    echo [FAIL] Local demo launch returned failure>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%CLIENT_STATUS_FILE%" "Client status file"
call :wait_for_file "%HOST_STATUS_FILE%" "Host status file"
if "%FAILED%"=="1" goto :cleanup

call :require_line "%CLIENT_STATUS_FILE%" "connect_state=connected-headless-host" "Client connect success visible"
call :require_line "%CLIENT_STATUS_FILE%" "connect_transport=file-backed-localhost-fallback" "Client transport visible"
call :require_line "%CLIENT_STATUS_FILE%" "connect_lane_mode=localhost-fallback" "Client lane mode visible"
call :require_line "%CLIENT_STATUS_FILE%" "expected_protocol_version=2" "Client protocol version visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "client_instance_id=client-" "Client instance identity visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "client_session_id=client-session-" "Client session identity visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "host_instance_id=host-" "Host instance mirrored to client status"
call :require_prefix "%CLIENT_STATUS_FILE%" "host_session_id=session-" "Host session mirrored to client status"
call :require_prefix "%CLIENT_STATUS_FILE%" "room_title=" "Room title visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "room_description=" "Room description visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "prompt_line=" "Prompt visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "command_bar=> " "Command shell visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "command_echo=" "Command reply surface visible"
call :require_line "%HOST_STATUS_FILE%" "transport_kind=file-backed-localhost-fallback" "Host localhost transport visible"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M45 Local Demo Smoke Test
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
if %WAIT_ATTEMPT% GEQ 30 (
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
