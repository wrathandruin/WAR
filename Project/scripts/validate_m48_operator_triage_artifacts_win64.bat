@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M48OperatorTriageRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "TRIAGE_DIR=%RUNTIME_ROOT%\CrashDumps\FailureBundles\OperatorTriage"
set "REPORT_PATH=%LOG_DIR%\m48_operator_triage_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m48_operator_triage_validation_details.txt"

if exist "%RUNTIME_ROOT%" rmdir /s /q "%RUNTIME_ROOT%"
mkdir "%CONFIG_DIR%" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Host" >nul 2>nul

if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Bootstrap launch failed unexpectedly>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%TRIAGE_DIR%\latest_failure_bundle.txt" "Latest failure pointer"
call :wait_for_file "%TRIAGE_DIR%\operator_triage_summary.txt" "Operator triage summary"
call :wait_for_file "%TRIAGE_DIR%\operator_triage_checklist.txt" "Operator triage checklist"
call :wait_for_file "%TRIAGE_DIR%\latest_bundle_manifest.txt" "Promoted latest bundle manifest"
call :wait_for_file "%TRIAGE_DIR%\latest_attachments_manifest.txt" "Promoted latest attachments manifest"
if "%FAILED%"=="1" goto :cleanup

call :require_text_line "%TRIAGE_DIR%\latest_failure_bundle.txt" "bundle_kind=bootstrap" "Latest pointer reports bootstrap bundle"
call :require_prefix "%TRIAGE_DIR%\latest_failure_bundle.txt" "bundle_directory=" "Latest pointer exposes bundle path"
call :require_text_line "%TRIAGE_DIR%\operator_triage_summary.txt" "WAR Operator Triage Summary" "Operator summary header present"
call :require_text_line "%TRIAGE_DIR%\operator_triage_checklist.txt" "WAR Operator Triage Checklist" "Operator checklist header present"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M48 Operator Triage Artifact Validation
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

:require_text_line
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
