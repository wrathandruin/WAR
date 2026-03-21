@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M46MissingSecretsRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "HOST_DIR=%RUNTIME_ROOT%\Host"
set "STARTUP_REPORT=%LOG_DIR%\local_demo_startup_report.txt"
set "HOST_STATUS_FILE=%HOST_DIR%\headless_host_status.txt"
set "TRACE_FILE=%LOG_DIR%\headless_host_trace.txt"
set "SECRETS_FILE=%CONFIG_DIR%\hosted_internal_alpha.secrets.cfg"
set "REPORT_PATH=%LOG_DIR%\m46_missing_required_secrets_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m46_missing_required_secrets_validation_details.txt"

if exist "%RUNTIME_ROOT%" rmdir /s /q "%RUNTIME_ROOT%"
mkdir "%CONFIG_DIR%" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%HOST_DIR%" >nul 2>nul

if exist "%SECRETS_FILE%" del /q "%SECRETS_FILE%" >nul 2>nul
copy /y nul "%DETAILS_PATH%" >nul
copy /y nul "%REPORT_PATH%" >nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Missing-secrets host launch failed unexpectedly>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%STARTUP_REPORT%" "Startup report"
if "%FAILED%"=="1" goto :cleanup

call :require_line "%STARTUP_REPORT%" "Environment: hosted_internal_alpha" "Environment visible during fail-fast"
call :require_line "%STARTUP_REPORT%" "Environment profile: hosted_internal_alpha" "Environment profile visible during fail-fast"
call :require_line "%STARTUP_REPORT%" "Config identity: hosted-internal-alpha-baseline" "Config identity visible during fail-fast"
call :require_line "%STARTUP_REPORT%" "Secrets required: yes" "Secrets requirement visible during fail-fast"
call :require_line "%STARTUP_REPORT%" "Missing required secret count: 2" "Missing secret count visible"
call :require_line "%STARTUP_REPORT%" "Missing required secret keys: bootstrap_shared_key, telemetry_hmac_salt" "Missing secret key names visible"
call :require_line "%STARTUP_REPORT%" "Configuration valid: no" "Configuration failure visible"
call :wait_for_file "%TRACE_FILE%" "Fail-fast trace file"
if "%FAILED%"=="1" goto :cleanup

ping 127.0.0.1 -n 3 >nul
if exist "%HOST_STATUS_FILE%" (
    echo [FAIL] Host status should not exist after fail-fast missing secrets>> "%DETAILS_PATH%"
    set "FAILED=1"
) else (
    echo [PASS] Host status suppressed on fail-fast missing secrets>> "%DETAILS_PATH%"
)

if exist "%TRACE_FILE%" (
    call :require_line "%TRACE_FILE%" "fail-fast config" "Fail-fast trace visible"
) else (
    echo [FAIL] Fail-fast trace file missing>> "%DETAILS_PATH%"
    set "FAILED=1"
)

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M46 Missing Required Secrets Validation
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
findstr /c:"%~2" "%~1" >nul
if errorlevel 1 (
    echo [FAIL] %~3>> "%DETAILS_PATH%"
    set "FAILED=1"
) else (
    echo [PASS] %~3>> "%DETAILS_PATH%"
)
exit /b 0
