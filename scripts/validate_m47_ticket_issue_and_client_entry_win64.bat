@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M47TicketIssueRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "HOST_DIR=%RUNTIME_ROOT%\Host"
set "CLIENT_STATUS_FILE=%LOG_DIR%\client_replication_status.txt"
set "HOST_STATUS_FILE=%HOST_DIR%\headless_host_status.txt"
set "RESUME_FILE=%CONFIG_DIR%\client_resume_identity.txt"
set "SECRETS_FILE=%CONFIG_DIR%\hosted_internal_alpha.secrets.cfg"
set "REPORT_PATH=%LOG_DIR%\m47_ticket_issue_and_client_entry_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m47_ticket_issue_and_client_entry_validation_details.txt"
set "CHECKLIST_PATH=%CONFIG_DIR%\m47_ticket_issue_checks.txt"

if exist "%RUNTIME_ROOT%" rmdir /s /q "%RUNTIME_ROOT%"
mkdir "%CONFIG_DIR%" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%HOST_DIR%" >nul 2>nul

(
    echo bootstrap_shared_key=M47_TICKET_ISSUE_PLACEHOLDER
    echo telemetry_hmac_salt=M47_TICKET_ISSUE_PLACEHOLDER
) > "%SECRETS_FILE%"

if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul
if exist "%CHECKLIST_PATH%" del /q "%CHECKLIST_PATH%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Host launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%HOST_STATUS_FILE%" "Host status file"
if "%FAILED%"=="1" goto :cleanup

call "%SCRIPT_DIR%launch_local_client_against_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Client launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%CLIENT_STATUS_FILE%" "Client replication status"
call :wait_for_file "%RESUME_FILE%" "Client resume identity file"
if "%FAILED%"=="1" goto :cleanup

(
    echo %CLIENT_STATUS_FILE%^|^^session_entry_state=ticket-issued$^|Client received issued ticket
    echo %CLIENT_STATUS_FILE%^|^^session_request_id=request-.+^|Session request id visible
    echo %CLIENT_STATUS_FILE%^|^^session_ticket_id=ticket-.+^|Session ticket id visible
    echo %CLIENT_STATUS_FILE%^|^^session_ticket_state=issued$^|Session ticket state visible
    echo %CLIENT_STATUS_FILE%^|^^session_denial_reason=none$^|No denial reason on success
    echo %CLIENT_STATUS_FILE%^|^^granted_session_id=session-.+^|Granted session id visible
    echo %CLIENT_STATUS_FILE%^|^^resume_session_id=session-.+^|Resume session id visible
    echo %CLIENT_STATUS_FILE%^|^^reconnect_requested=no$^|Fresh entry is not marked reconnect
    echo %CLIENT_STATUS_FILE%^|^^authority_mode=headless-host$^|Authority waits for granted ticket
    echo %CLIENT_STATUS_FILE%^|^^connect_state=connected-headless-host$^|Host lane is active after ticket issue
    echo %CLIENT_STATUS_FILE%^|^^account_id=.+^|Account id visible
    echo %CLIENT_STATUS_FILE%^|^^player_identity=.+^|Player identity visible
    echo %CLIENT_STATUS_FILE%^|^^host_session_id=session-.+^|Host session id mirrored to client
    echo %CLIENT_STATUS_FILE%^|^^host_instance_id=host-.+^|Host instance id mirrored to client
    echo %RESUME_FILE%^|^^granted_session_id=session-.+^|Resume identity persisted
    echo %RESUME_FILE%^|^^ticket_id=ticket-.+^|Issued ticket id persisted
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

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
if exist "%CHECKLIST_PATH%" del /q "%CHECKLIST_PATH%" >nul 2>nul

(
    echo WAR M47 Ticket Issue and Client Entry Validation
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
