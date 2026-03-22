@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M48FailureBundleRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "FAILURE_ROOT=%RUNTIME_ROOT%\CrashDumps\FailureBundles"
set "BOOTSTRAP_BUNDLE_ROOT=%FAILURE_ROOT%\Bootstrap"
set "REPORT_PATH=%LOG_DIR%\m48_failure_bundle_capture_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m48_failure_bundle_capture_validation_details.txt"

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
set "LATEST_BUNDLE_DIR="

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "internal-alpha-lan" "hosted-bootstrap" "hosted_internal_alpha" "hosted_internal_alpha" "" "primary"
if errorlevel 1 (
    echo [FAIL] Bootstrap launch failed unexpectedly>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_bundle "%BOOTSTRAP_BUNDLE_ROOT%"
if "%FAILED%"=="1" goto :cleanup

call :require_file "%LATEST_BUNDLE_DIR%\bundle_manifest.txt" "Failure bundle manifest written"
call :require_file "%LATEST_BUNDLE_DIR%\attachments_manifest.txt" "Failure bundle attachment manifest written"
call :require_line "%LATEST_BUNDLE_DIR%\bundle_manifest.txt" "bundle_kind=bootstrap" "Bundle kind is bootstrap"
call :require_line "%LATEST_BUNDLE_DIR%\bundle_manifest.txt" "component=host-bootstrap" "Bootstrap component recorded"
call :require_prefix "%LATEST_BUNDLE_DIR%\bundle_manifest.txt" "failure_code=" "Failure code recorded"
call :require_prefix "%LATEST_BUNDLE_DIR%\bundle_manifest.txt" "failure_reason=" "Failure reason recorded"
call :require_prefix "%LATEST_BUNDLE_DIR%\attachments_manifest.txt" "copied_count=" "Copied attachment count recorded"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M48 Failure Bundle Capture Validation
    if "%FAILED%"=="0" (echo Result: PASS) else (echo Result: FAIL)
    echo.
    if exist "%DETAILS_PATH%" type "%DETAILS_PATH%"
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
if "%FAILED%"=="0" exit /b 0
exit /b 1

:wait_for_bundle
set "WAIT_ROOT=%~1"
set /a WAIT_ATTEMPT=0
:bundle_loop
for /f "delims=" %%F in ('dir /b /ad "%WAIT_ROOT%\*" 2^>nul') do (
    set "LATEST_BUNDLE_DIR=%WAIT_ROOT%\%%F"
    goto :bundle_found
)
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 30 (
    echo [FAIL] Failure bundle directory was not created under %WAIT_ROOT%>> "%DETAILS_PATH%"
    set "FAILED=1"
    exit /b 0
)
ping 127.0.0.1 -n 2 >nul
goto :bundle_loop

:bundle_found
echo [PASS] Failure bundle directory created: %LATEST_BUNDLE_DIR%>> "%DETAILS_PATH%"
exit /b 0

:require_file
if exist "%~1" (
    echo [PASS] %~2>> "%DETAILS_PATH%"
) else (
    echo [FAIL] %~2>> "%DETAILS_PATH%"
    set "FAILED=1"
)
exit /b 0

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
