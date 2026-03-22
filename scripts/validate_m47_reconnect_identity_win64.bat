@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M47ReconnectRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "HOST_DIR=%RUNTIME_ROOT%\Host"
set "CLIENT_STATUS_FILE=%LOG_DIR%\client_replication_status.txt"
set "HOST_STATUS_FILE=%HOST_DIR%\headless_host_status.txt"
set "REQUEST_DIR=%HOST_DIR%\SessionEntry\Requests"
set "DENIED_DIR=%HOST_DIR%\SessionEntry\DeniedTickets"
set "RESUME_FILE=%CONFIG_DIR%\client_resume_identity.txt"
set "SECRETS_FILE=%CONFIG_DIR%\hosted_internal_alpha.secrets.cfg"
set "REPORT_PATH=%LOG_DIR%\m47_reconnect_identity_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m47_reconnect_identity_validation_details.txt"
set "CHECKLIST_PATH=%CONFIG_DIR%\m47_reconnect_checks.txt"

if exist "%RUNTIME_ROOT%" rmdir /s /q "%RUNTIME_ROOT%"
mkdir "%CONFIG_DIR%" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%HOST_DIR%" >nul 2>nul

(
    echo bootstrap_shared_key=M47_RECONNECT_PLACEHOLDER
    echo telemetry_hmac_salt=M47_RECONNECT_PLACEHOLDER
) > "%SECRETS_FILE%"

if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul
if exist "%CHECKLIST_PATH%" del /q "%CHECKLIST_PATH%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"
set "GRANTED_SESSION_ID="
set "DENIED_TICKET_FILE="

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Host launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

set /a WAIT_ATTEMPT=0
:wait_reconnect_host_status_loop
if exist "%HOST_STATUS_FILE%" goto :reconnect_host_status_ready
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 30 (
    echo [FAIL] Host status file missing: %HOST_STATUS_FILE%>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)
ping 127.0.0.1 -n 2 >nul
goto :wait_reconnect_host_status_loop

:reconnect_host_status_ready
if "%FAILED%"=="1" goto :cleanup

call "%SCRIPT_DIR%launch_local_client_against_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Client launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

set /a WAIT_ATTEMPT=0
:wait_reconnect_client_status_loop
if exist "%CLIENT_STATUS_FILE%" goto :reconnect_client_status_ready
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 30 (
    echo [FAIL] Client replication status missing: %CLIENT_STATUS_FILE%>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)
ping 127.0.0.1 -n 2 >nul
goto :wait_reconnect_client_status_loop

:reconnect_client_status_ready
set /a WAIT_ATTEMPT=0
:wait_reconnect_resume_loop
if exist "%RESUME_FILE%" goto :reconnect_resume_ready
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 30 (
    echo [FAIL] Client resume identity file missing: %RESUME_FILE%>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)
ping 127.0.0.1 -n 2 >nul
goto :wait_reconnect_resume_loop

:reconnect_resume_ready
if "%FAILED%"=="1" goto :cleanup

for /f "tokens=1,* delims==" %%A in (%RESUME_FILE%) do (
    if /I "%%A"=="granted_session_id" set "GRANTED_SESSION_ID=%%B"
)

if "%GRANTED_SESSION_ID%"=="" (
    echo [FAIL] Granted session id not found in resume identity file>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
) else (
    echo [PASS] Granted session id captured: %GRANTED_SESSION_ID%>> "%DETAILS_PATH%"
)

if not exist "%REQUEST_DIR%" mkdir "%REQUEST_DIR%" >nul 2>nul
(
    echo version=1
    echo request_id=reconnect-request
    echo account_id=internal-alpha-player
    echo player_identity=operator-alpha
    echo client_instance_id=client-reconnect
    echo build_identity=reconnect-build
    echo environment_name=hosted_internal_alpha
    echo connect_target_name=internal-alpha-lan
    echo requested_resume_session_id=%GRANTED_SESSION_ID%
    echo requested_at_epoch_ms=9999999999999
    echo requested_ticket_ttl_seconds=120
    echo reconnect_requested=yes
) > "%REQUEST_DIR%\request_reconnect_request.txt"

set /a WAIT_ATTEMPT=0
:wait_reconnect_denied_ticket_loop
for /f "delims=" %%F in ('dir /b /a-d "%DENIED_DIR%\ticket_*.txt" 2^>nul') do (
    set "DENIED_TICKET_FILE=%DENIED_DIR%\%%F"
    findstr /c:"request_id=reconnect-request" "!DENIED_TICKET_FILE!" >nul
    if not errorlevel 1 goto :reconnect_denied_ticket_found
)
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 30 (
    echo [FAIL] Reconnect denial ticket file was not written>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)
ping 127.0.0.1 -n 2 >nul
goto :wait_reconnect_denied_ticket_loop

:reconnect_denied_ticket_found
echo [PASS] Reconnect denial ticket written: %DENIED_TICKET_FILE%>> "%DETAILS_PATH%"

(
    echo %DENIED_TICKET_FILE%^|^^request_id=reconnect-request$^|Reconnect request id preserved
    echo %DENIED_TICKET_FILE%^|^^state=denied$^|Reconnect request handled coherently
    echo %DENIED_TICKET_FILE%^|^^resume_session_id=.+^|Reconnect ticket carries resume identity field
    echo %DENIED_TICKET_FILE%^|^^denial_reason=.+^|Reconnect denial reason visible
    echo %RESUME_FILE%^|^^granted_session_id=%GRANTED_SESSION_ID%$^|Resume identity persisted for reconnect direction
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
    echo WAR M47 Reconnect Identity Validation
    if "%FAILED%"=="0" (echo Result: PASS) else (echo Result: FAIL)
    echo.
    if exist "%DETAILS_PATH%" type "%DETAILS_PATH%"
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
if "%FAILED%"=="0" exit /b 0
exit /b 1
