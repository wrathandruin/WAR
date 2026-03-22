@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "HOSTED_RUNTIME_ROOT=%PACKAGE_ROOT%\HostedBootstrapRuntime"
set "CONFIG_DIR=%HOSTED_RUNTIME_ROOT%\Config"
set "LOG_DIR=%HOSTED_RUNTIME_ROOT%\Logs"
set "HOST_STATUS_FILE=%HOSTED_RUNTIME_ROOT%\Host\headless_host_status.txt"
set "CLIENT_STATUS_FILE=%HOSTED_RUNTIME_ROOT%\Logs\client_replication_status.txt"
set "SECRETS_FILE=%CONFIG_DIR%\hosted_internal_alpha.secrets.cfg"
set "REPORT_PATH=%LOG_DIR%\m45_hosted_bootstrap_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m45_hosted_bootstrap_validation_details.txt"

if exist "%HOSTED_RUNTIME_ROOT%" rmdir /s /q "%HOSTED_RUNTIME_ROOT%"
mkdir "%CONFIG_DIR%" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%HOSTED_RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%HOSTED_RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%HOSTED_RUNTIME_ROOT%\Host" >nul 2>nul

(
    echo bootstrap_shared_key=M45_HOSTED_BOOTSTRAP_PLACEHOLDER
    echo telemetry_hmac_salt=M45_HOSTED_TELEMETRY_PLACEHOLDER
) > "%SECRETS_FILE%"

if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%HOSTED_RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Hosted bootstrap host launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%HOST_STATUS_FILE%" "Hosted bootstrap host status"
if "%FAILED%"=="1" goto :cleanup

call "%SCRIPT_DIR%launch_local_client_against_host_win64.bat" "%HOSTED_RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Hosted bootstrap client launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%CLIENT_STATUS_FILE%" "Hosted bootstrap client status"
if "%FAILED%"=="1" goto :cleanup

call :require_line "%HOST_STATUS_FILE%" "transport_kind=file-backed-hosted-bootstrap" "Hosted transport visible on host"
call :require_line "%HOST_STATUS_FILE%" "connect_target_name=internal-alpha-lan" "Hosted target visible on host"
call :require_line "%HOST_STATUS_FILE%" "connect_lane_mode=hosted-bootstrap" "Hosted lane visible on host"
call :require_line "%HOST_STATUS_FILE%" "environment_name=hosted_internal_alpha" "Hosted environment visible on host"
call :require_line "%HOST_STATUS_FILE%" "environment_profile_name=hosted_internal_alpha" "Hosted environment profile visible on host"
call :require_prefix "%HOST_STATUS_FILE%" "host_instance_id=host-" "Host instance identity visible"
call :require_prefix "%HOST_STATUS_FILE%" "session_id=session-" "Session identity visible"

call :require_line "%CLIENT_STATUS_FILE%" "connect_state=connected-headless-host" "Hosted client connect success visible"
call :require_line "%CLIENT_STATUS_FILE%" "connect_transport=file-backed-hosted-bootstrap" "Hosted client transport visible"
call :require_line "%CLIENT_STATUS_FILE%" "connect_target_name=internal-alpha-lan" "Hosted client target visible"
call :require_line "%CLIENT_STATUS_FILE%" "connect_lane_mode=hosted-bootstrap" "Hosted client lane visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "client_instance_id=client-" "Hosted client identity visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "client_session_id=client-session-" "Hosted client session visible"
call :require_prefix "%CLIENT_STATUS_FILE%" "host_instance_id=host-" "Hosted host identity mirrored to client"
call :require_prefix "%CLIENT_STATUS_FILE%" "host_session_id=session-" "Hosted host session mirrored to client"
call :require_prefix "%CLIENT_STATUS_FILE%" "room_title=" "Room title visible in hosted bootstrap"
call :require_prefix "%CLIENT_STATUS_FILE%" "room_description=" "Room description visible in hosted bootstrap"
call :require_prefix "%CLIENT_STATUS_FILE%" "prompt_line=" "Prompt visible in hosted bootstrap"
call :require_prefix "%CLIENT_STATUS_FILE%" "command_bar=> " "Command shell visible in hosted bootstrap"
call :require_prefix "%CLIENT_STATUS_FILE%" "command_echo=" "Command reply surface visible in hosted bootstrap"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M45 Hosted Bootstrap Validation
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
