@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "CLIENT_EXE_PATH=%DEMO_ROOT%\WAR.exe"
set "RUNTIME_ROOT=%DEMO_ROOT%\runtime"
if not exist "%RUNTIME_ROOT%" set "RUNTIME_ROOT=%DEMO_ROOT%\Runtime"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "STARTUP_REPORT=%LOG_DIR%\local_demo_startup_report.txt"
set "STATUS_FILE=%LOG_DIR%\client_replication_status.txt"
set "REPORT_PATH=%LOG_DIR%\local_demo_smoke_test.txt"
set "DETAILS_PATH=%LOG_DIR%\local_demo_smoke_test_details.txt"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul
if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul
if exist "%STARTUP_REPORT%" del /q "%STARTUP_REPORT%" >nul 2>nul
if exist "%STATUS_FILE%" del /q "%STATUS_FILE%" >nul 2>nul

set "FAILED=0"

call :check_file "%CLIENT_EXE_PATH%" "Client executable"
if "%FAILED%"=="1" goto :report

call "%SCRIPT_DIR%launch_local_demo_win64.bat"
if errorlevel 1 (
    echo [FAIL] Launch script returned failure>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :report
)

ping 127.0.0.1 -n 11 >nul

call :check_file "%STARTUP_REPORT%" "Local demo startup report"
call :check_file "%STATUS_FILE%" "Client replication status file"

taskkill /IM WAR.exe /F >nul 2>nul

:report
(
    echo WAR Local Demo Smoke Test
    echo Demo root: %DEMO_ROOT%
    echo Client executable: %CLIENT_EXE_PATH%
    echo Runtime root: %RUNTIME_ROOT%
    if "%FAILED%"=="0" (
        echo Result: PASS
    ) else (
        echo Result: FAIL
    )
    echo.
    if exist "%DETAILS_PATH%" type "%DETAILS_PATH%"
) > "%REPORT_PATH%"

type "%REPORT_PATH%"
if "%FAILED%"=="0" (
    echo [M40] Local demo smoke test passed.
    exit /b 0
)

echo [M40] Local demo smoke test failed.
exit /b 1

:check_file
if exist "%~1" (
    echo [PASS] %~2>> "%DETAILS_PATH%"
) else (
    echo [FAIL] %~2 missing: %~1>> "%DETAILS_PATH%"
    set "FAILED=1"
)
exit /b 0
