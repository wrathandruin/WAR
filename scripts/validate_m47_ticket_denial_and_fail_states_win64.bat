@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M47TicketDenialRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "HOST_DIR=%RUNTIME_ROOT%\Host"
set "REQUEST_DIR=%HOST_DIR%\SessionEntry\Requests"
set "DENIED_DIR=%HOST_DIR%\SessionEntry\DeniedTickets"
set "HOST_STATUS_FILE=%HOST_DIR%\headless_host_status.txt"
set "SECRETS_FILE=%CONFIG_DIR%\hosted_internal_alpha.secrets.cfg"
set "REQUEST_FILE=%REQUEST_DIR%\request_expired_request.txt"
set "REPORT_PATH=%LOG_DIR%\m47_ticket_denial_and_fail_states_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m47_ticket_denial_and_fail_states_validation_details.txt"
set "CHECKLIST_PATH=%CONFIG_DIR%\m47_ticket_denial_checks.txt"

if exist "%RUNTIME_ROOT%" rmdir /s /q "%RUNTIME_ROOT%"
mkdir "%CONFIG_DIR%" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%HOST_DIR%" >nul 2>nul

(
    echo bootstrap_shared_key=M47_DENIAL_PLACEHOLDER
    echo telemetry_hmac_salt=M47_DENIAL_PLACEHOLDER
) > "%SECRETS_FILE%"

if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul
if exist "%CHECKLIST_PATH%" del /q "%CHECKLIST_PATH%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"
set "DENIED_TICKET_FILE="

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Host launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

set /a WAIT_ATTEMPT=0
:wait_host_status_loop
if exist "%HOST_STATUS_FILE%" goto :host_status_ready
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 30 (
    echo [FAIL] Host status file missing: %HOST_STATUS_FILE%>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)
ping 127.0.0.1 -n 2 >nul
goto :wait_host_status_loop

:host_status_ready
if "%FAILED%"=="1" goto :cleanup

if not exist "%REQUEST_DIR%" mkdir "%REQUEST_DIR%" >nul 2>nul
(
    echo version=1
    echo request_id=expired-request
    echo account_id=bad-actor
    echo player_identity=bad-actor
    echo client_instance_id=client-expired
    echo build_identity=invalid-build
    echo environment_name=wrong-env
    echo connect_target_name=wrong-target
    echo requested_resume_session_id=stale-session
    echo requested_at_epoch_ms=1
    echo requested_ticket_ttl_seconds=1
    echo reconnect_requested=yes
) > "%REQUEST_FILE%"

set /a WAIT_ATTEMPT=0
:wait_denied_ticket_loop
for /f "delims=" %%F in ('dir /b /a-d "%DENIED_DIR%\ticket_*.txt" 2^>nul') do (
    set "DENIED_TICKET_FILE=%DENIED_DIR%\%%F"
    goto :denied_ticket_found
)
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 30 (
    echo [FAIL] Denied ticket file was not written>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)
ping 127.0.0.1 -n 2 >nul
goto :wait_denied_ticket_loop

:denied_ticket_found
echo [PASS] Denied ticket file written: %DENIED_TICKET_FILE%>> "%DETAILS_PATH%"

(
    echo %DENIED_TICKET_FILE%^|^^state=denied$^|Denied ticket state written
    echo %DENIED_TICKET_FILE%^|^^request_id=expired-request$^|Denied ticket preserves request id
    echo %DENIED_TICKET_FILE%^|^^ticket_id=deny-.+^|Denied ticket id visible
    echo %DENIED_TICKET_FILE%^|^^denial_reason=.+^|Denial reason written
) > "%CHECKLIST_PATH%"

for /f "usebackq tokens=1,2,* delims=|" %%A in ("%CHECKLIST_PATH%") do (
    powershell.exe -NoProfile -Command "if (Select-String -Path '%%~A' -Pattern '%%~B' -Quiet) { exit 0 } else { exit 1 }" >nul 2>nul
    if errorlevel 1 (
        echo [FAIL] %%C>> "%DETAILS_PATH%"
        set "FAILED=1"
    ) else (
        echo [PASS] %%C>> "%DETAILS_PATH%"
    )
)

powershell.exe -NoProfile -Command "if (Select-String -Path '%DENIED_TICKET_FILE%' -Pattern '^denial_reason=none$' -Quiet) { exit 0 } else { exit 1 }" >nul 2>nul
if errorlevel 1 (
    echo [PASS] Denied ticket has non-empty reason>> "%DETAILS_PATH%"
) else (
    echo [FAIL] Denied ticket has non-empty reason>> "%DETAILS_PATH%"
    set "FAILED=1"
)

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
if exist "%CHECKLIST_PATH%" del /q "%CHECKLIST_PATH%" >nul 2>nul

(
    echo WAR M47 Ticket Denial and Fail States Validation
    if "%FAILED%"=="0" (echo Result: PASS) else (echo Result: FAIL)
    echo.
    if exist "%DETAILS_PATH%" type "%DETAILS_PATH%"
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
if "%FAILED%"=="0" exit /b 0
exit /b 1
