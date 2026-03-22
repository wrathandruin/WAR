@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M49RetentionRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "RETENTION_DIR=%LOG_DIR%\Retention"
set "SUMMARY_FILE=%RETENTION_DIR%\beta_retention_summary.txt"
set "JOURNAL_FILE=%RETENTION_DIR%\beta_launch_journal.txt"
set "REPORT_PATH=%LOG_DIR%\m49_retention_expansion_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m49_retention_expansion_validation_details.txt"

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

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "localhost-fallback" "localhost-fallback" "local" "local" "" "primary"
if errorlevel 1 (
    echo [FAIL] Host bootstrap launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%SUMMARY_FILE%" "Retention summary"
call :wait_for_file "%JOURNAL_FILE%" "Retention journal"
if "%FAILED%"=="1" goto :cleanup

findstr /b /c:"version=1" "%SUMMARY_FILE%" >nul || set "FAILED=1"
findstr /b /c:"total_launches=" "%SUMMARY_FILE%" >nul || set "FAILED=1"
findstr /b /c:"content_card_count=4" "%SUMMARY_FILE%" >nul || set "FAILED=1"
findstr /c:"component=host-bootstrap" "%JOURNAL_FILE%" >nul || set "FAILED=1"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M49 Retention Expansion Validation
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
