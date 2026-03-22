@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\M50UpdateRuntime"
set "CONFIG_DIR=%RUNTIME_ROOT%\Config"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "UPDATER_DIR=%LOG_DIR%\Updater"
set "RUNTIME_MANIFEST=%UPDATER_DIR%\update_runtime_manifest.txt"
set "SOURCE_MANIFEST=%PACKAGE_ROOT%\Installer\m50_update_channel_manifest.txt"
set "REPORT_PATH=%LOG_DIR%\m50_update_validation.txt"
set "DETAILS_PATH=%LOG_DIR%\m50_update_validation_details.txt"

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

if not exist "%SOURCE_MANIFEST%" (
    echo [FAIL] Source update manifest missing>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call "%SCRIPT_DIR%launch_headless_host_win64.bat" "%RUNTIME_ROOT%" "localhost-fallback" "localhost-fallback" "local" "local" "" "primary"
if errorlevel 1 (
    echo [FAIL] Host bootstrap launch failed>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%RUNTIME_MANIFEST%" "Updater runtime manifest"
if "%FAILED%"=="1" goto :cleanup

findstr /b /c:"version=1" "%SOURCE_MANIFEST%" >nul || set "FAILED=1"
findstr /b /c:"update_channel=market-candidate" "%SOURCE_MANIFEST%" >nul || set "FAILED=1"
findstr /b /c:"version=1" "%RUNTIME_MANIFEST%" >nul || set "FAILED=1"
findstr /b /c:"update_manifest_path=" "%RUNTIME_MANIFEST%" >nul || set "FAILED=1"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

(
    echo WAR M50 Update Productionization Validation
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
