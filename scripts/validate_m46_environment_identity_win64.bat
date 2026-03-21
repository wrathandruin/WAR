@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M46HostedEnvironmentRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "HOST_DIR=%RUNTIME_ROOT%\Host"
set "STARTUP_REPORT=%LOG_DIR%\local_demo_startup_report.txt"
set "HOST_STATUS_FILE=%HOST_DIR%\headless_host_status.txt"
set "TRACE_FILE=%LOG_DIR%\headless_host_trace.txt"
set "HOST_LOG_FILE=%LOG_DIR%\headless_host_log.txt"
set "SECRETS_FILE=%CONFIG_DIR%\hosted_internal_alpha.secrets.cfg"
set "REPORT_PATH=%LOG_DIR%\m46_environment_identity_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m46_environment_identity_validation_details.txt"
set "PRESENT_CHECKLIST=%CONFIG_DIR%\m46_environment_identity_present_checks.txt"
set "ABSENT_CHECKLIST=%CONFIG_DIR%\m46_environment_identity_absent_checks.txt"
set "SECRET_ALPHA=SECRET_ALPHA_SHOULD_NOT_APPEAR"
set "SECRET_BRAVO=SECRET_BRAVO_SHOULD_NOT_APPEAR"

if exist "%RUNTIME_ROOT%" rmdir /s /q "%RUNTIME_ROOT%"
mkdir "%CONFIG_DIR%" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%HOST_DIR%" >nul 2>nul

(
    echo bootstrap_shared_key=%SECRET_ALPHA%
    echo telemetry_hmac_salt=%SECRET_BRAVO%
) > "%SECRETS_FILE%"

copy /y nul "%DETAILS_PATH%" >nul
copy /y nul "%REPORT_PATH%" >nul
if exist "%PRESENT_CHECKLIST%" del /q "%PRESENT_CHECKLIST%" >nul 2>nul
if exist "%ABSENT_CHECKLIST%" del /q "%ABSENT_CHECKLIST%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "ops-alpha"
if errorlevel 1 (
    echo [FAIL] Hosted environment validation launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%STARTUP_REPORT%" "Startup report"
call :wait_for_file "%HOST_STATUS_FILE%" "Host status"
if "%FAILED%"=="1" goto :cleanup

(
    echo %STARTUP_REPORT%^|Environment: hosted_internal_alpha^|Environment visible in startup report
    echo %STARTUP_REPORT%^|Environment profile: hosted_internal_alpha^|Environment profile visible in startup report
    echo %STARTUP_REPORT%^|Config identity: hosted-internal-alpha-baseline^|Config identity visible in startup report
    echo %STARTUP_REPORT%^|Secrets required: yes^|Secrets requirement visible
    echo %STARTUP_REPORT%^|Loaded secret count: 2^|Loaded secret count visible
    echo %STARTUP_REPORT%^|Missing required secret count: 0^|No missing secrets visible
    echo %STARTUP_REPORT%^|Configuration valid: yes^|Configuration success visible
    echo %STARTUP_REPORT%^|Requested save slot: ops-alpha^|Requested save slot visible
    echo %STARTUP_REPORT%^|Sanitized primary save slot: ops-alpha^|Sanitized save slot visible
    echo %STARTUP_REPORT%^|Runtime ownership valid: yes^|Runtime ownership visible
    echo %HOST_STATUS_FILE%^|environment_name=hosted_internal_alpha^|Environment visible on host status
    echo %HOST_STATUS_FILE%^|environment_profile_name=hosted_internal_alpha^|Environment profile visible on host status
    echo %HOST_STATUS_FILE%^|runtime_ownership_valid=yes^|Runtime ownership visible on host status
    echo %HOST_STATUS_FILE%^|primary_save_path_owned=yes^|Primary save path ownership visible on host status
    echo %HOST_STATUS_FILE%^|persistence_slot=ops-alpha^|Persistence slot visible on host status
) > "%PRESENT_CHECKLIST%"

for /f "usebackq tokens=1,2,* delims=|" %%A in ("%PRESENT_CHECKLIST%") do (
    powershell.exe -NoProfile -Command "if (Select-String -Path '%%~A' -SimpleMatch -Pattern '%%~B' -Quiet) { exit 0 } else { exit 1 }" >nul 2>nul
    if errorlevel 1 (
        echo [FAIL] %%C>> "%DETAILS_PATH%"
        set "FAILED=1"
    ) else (
        echo [PASS] %%C>> "%DETAILS_PATH%"
    )
)

(
    echo %STARTUP_REPORT%^|%SECRET_ALPHA%^|Startup report redacts secret alpha
    echo %STARTUP_REPORT%^|%SECRET_BRAVO%^|Startup report redacts secret bravo
    echo %HOST_STATUS_FILE%^|%SECRET_ALPHA%^|Host status redacts secret alpha
    echo %HOST_STATUS_FILE%^|%SECRET_BRAVO%^|Host status redacts secret bravo
) > "%ABSENT_CHECKLIST%"
if exist "%TRACE_FILE%" >> "%ABSENT_CHECKLIST%" echo %TRACE_FILE%^|%SECRET_ALPHA%^|Trace redacts secret alpha
if exist "%TRACE_FILE%" >> "%ABSENT_CHECKLIST%" echo %TRACE_FILE%^|%SECRET_BRAVO%^|Trace redacts secret bravo
if exist "%HOST_LOG_FILE%" >> "%ABSENT_CHECKLIST%" echo %HOST_LOG_FILE%^|%SECRET_ALPHA%^|Host log redacts secret alpha
if exist "%HOST_LOG_FILE%" >> "%ABSENT_CHECKLIST%" echo %HOST_LOG_FILE%^|%SECRET_BRAVO%^|Host log redacts secret bravo

for /f "usebackq tokens=1,2,* delims=|" %%A in ("%ABSENT_CHECKLIST%") do (
    if not exist "%%~A" (
        echo [FAIL] %%C>> "%DETAILS_PATH%"
        set "FAILED=1"
    ) else (
        powershell.exe -NoProfile -Command "if (Select-String -Path '%%~A' -SimpleMatch -Pattern '%%~B' -Quiet) { exit 0 } else { exit 1 }" >nul 2>nul
        if errorlevel 1 (
            echo [PASS] %%C>> "%DETAILS_PATH%"
        ) else (
            echo [FAIL] %%C>> "%DETAILS_PATH%"
            set "FAILED=1"
        )
    )
)

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul
if exist "%PRESENT_CHECKLIST%" del /q "%PRESENT_CHECKLIST%" >nul 2>nul
if exist "%ABSENT_CHECKLIST%" del /q "%ABSENT_CHECKLIST%" >nul 2>nul
if exist "%DETAILS_PATH%" powershell.exe -NoProfile -Command "$p = '%DETAILS_PATH%'; $seen = [System.Collections.Generic.HashSet[string]]::new(); $out = foreach ($line in Get-Content -Path $p) { if ($seen.Add($line)) { $line } }; Set-Content -Path $p -Value $out" >nul 2>nul

(
    echo WAR M46 Environment Identity Validation
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
