@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)
if "%PACKAGE_ROOT:~-1%"=="\" set "PACKAGE_ROOT=%PACKAGE_ROOT:~0,-1%"

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M46RuntimeOwnershipRuntime"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "HOST_DIR=%RUNTIME_ROOT%\Host"
set "STARTUP_REPORT=%LOG_DIR%\local_demo_startup_report.txt"
set "HOST_STATUS_FILE=%HOST_DIR%\headless_host_status.txt"
set "REPORT_PATH=%LOG_DIR%\m46_runtime_save_hygiene_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m46_runtime_save_hygiene_validation_details.txt"
set "CHECKLIST_PATH=%RUNTIME_ROOT%\Config\m46_runtime_save_checks.txt"
set "RAW_SLOT=..\..\Ops Slot"
set "SANITIZED_SLOT=ops_slot"
set "EXPECTED_RUNTIME_ROOT=%RUNTIME_ROOT:\=/%"
set "EXPECTED_SAVE_PATH=%EXPECTED_RUNTIME_ROOT%/Saves/authoritative_world_%SANITIZED_SLOT%.txt"

if exist "%RUNTIME_ROOT%" rmdir /s /q "%RUNTIME_ROOT%"
mkdir "%RUNTIME_ROOT%\Config" >nul 2>nul
mkdir "%LOG_DIR%" >nul 2>nul
mkdir "%RUNTIME_ROOT%\Saves" >nul 2>nul
mkdir "%RUNTIME_ROOT%\CrashDumps" >nul 2>nul
mkdir "%HOST_DIR%" >nul 2>nul

copy /y nul "%DETAILS_PATH%" >nul
copy /y nul "%REPORT_PATH%" >nul
if exist "%CHECKLIST_PATH%" del /q "%CHECKLIST_PATH%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "localhost-fallback" "localhost-fallback" "local" "local" "" "%RAW_SLOT%"
if errorlevel 1 (
    echo [FAIL] Runtime/save hygiene launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%STARTUP_REPORT%" "Startup report"
call :wait_for_file "%HOST_STATUS_FILE%" "Host status"
if "%FAILED%"=="1" goto :cleanup

(
    echo %STARTUP_REPORT%^|Requested save slot: %RAW_SLOT%^|Raw requested save slot visible
    echo %STARTUP_REPORT%^|Sanitized primary save slot: %SANITIZED_SLOT%^|Sanitized save slot visible
    echo %STARTUP_REPORT%^|Primary save path: %EXPECTED_SAVE_PATH%^|Owned save path visible
    echo %STARTUP_REPORT%^|Runtime owned directories sane: yes^|Runtime-owned directories are sane
    echo %STARTUP_REPORT%^|Deployable environment separated: yes^|Deployable environment remains separate
    echo %STARTUP_REPORT%^|Primary save path owned: yes^|Primary save path ownership visible
    echo %STARTUP_REPORT%^|Runtime ownership valid: yes^|Runtime ownership validity visible
    echo %HOST_STATUS_FILE%^|runtime_ownership_valid=yes^|Runtime ownership visible on host status
    echo %HOST_STATUS_FILE%^|runtime_owned_directories_sane=yes^|Runtime-owned directories visible on host status
    echo %HOST_STATUS_FILE%^|deployable_environment_separated=yes^|Environment separation visible on host status
    echo %HOST_STATUS_FILE%^|primary_save_path_owned=yes^|Primary save path ownership visible on host status
    echo %HOST_STATUS_FILE%^|persistence_slot=%SANITIZED_SLOT%^|Sanitized persistence slot visible on host status
    echo %HOST_STATUS_FILE%^|persistence_save_path=%EXPECTED_SAVE_PATH%^|Owned save path visible on host status
) > "%CHECKLIST_PATH%"

for /f "usebackq tokens=1,2,* delims=|" %%A in ("%CHECKLIST_PATH%") do (
    powershell.exe -NoProfile -Command "if (Select-String -Path '%%~A' -SimpleMatch -Pattern '%%~B' -Quiet) { exit 0 } else { exit 1 }" >nul 2>nul
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
    echo WAR M46 Runtime Save Hygiene Validation
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
