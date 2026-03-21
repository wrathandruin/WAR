@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "HOST_EXE_PATH=%DEMO_ROOT%\WARServer.exe"
if not exist "%HOST_EXE_PATH%" set "HOST_EXE_PATH=%DEMO_ROOT%\WAR.exe"
set "RUNTIME_ROOT=%DEMO_ROOT%\runtime"
if not exist "%RUNTIME_ROOT%" set "RUNTIME_ROOT=%DEMO_ROOT%\Runtime"
set "HOST_ROOT=%RUNTIME_ROOT%\Host"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "STATUS_FILE=%HOST_ROOT%\headless_host_status.txt"
set "HOST_LOG=%LOG_DIR%\headless_host_log.txt"
set "REPORT_PATH=%LOG_DIR%\headless_host_smoke_test.txt"
set "DETAILS_PATH=%LOG_DIR%\headless_host_smoke_test_details.txt"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul
if not exist "%HOST_ROOT%" mkdir "%HOST_ROOT%" >nul 2>nul
if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
if exist "%REPORT_PATH%" del /q "%REPORT_PATH%" >nul 2>nul
if exist "%STATUS_FILE%" del /q "%STATUS_FILE%" >nul 2>nul
if exist "%HOST_LOG%" del /q "%HOST_LOG%" >nul 2>nul

taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

set "FAILED=0"

call :check_file "%HOST_EXE_PATH%" "Host executable"
if "%FAILED%"=="1" goto :cleanup

call "%SCRIPT_DIR%launch_headless_host_win64.bat"
if errorlevel 1 (
    echo [FAIL] Packaged headless-host launch script returned failure>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :cleanup
)

call :wait_for_file "%STATUS_FILE%" "Headless host status file"
call :wait_for_file "%HOST_LOG%" "Headless host log"
if "%FAILED%"=="1" goto :cleanup

call :require_line "%STATUS_FILE%" "state=running" "Packaged headless host reached running state"
call :require_line "%STATUS_FILE%" "host_authority_active=yes" "Packaged headless host advertising authority"

:cleanup
taskkill /IM WAR.exe /F >nul 2>nul
taskkill /IM WARServer.exe /F >nul 2>nul

:report
(
    echo WAR Headless Host Smoke Test
    echo Demo root: %DEMO_ROOT%
    echo Host executable: %HOST_EXE_PATH%
    echo Runtime root: %RUNTIME_ROOT%
    echo Host root: %HOST_ROOT%
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
    echo [M44] Host smoke test passed.
    exit /b 0
)

echo [M44] Host smoke test failed.
exit /b 1

:wait_for_file
set "WAIT_PATH=%~1"
set "WAIT_LABEL=%~2"
set /a WAIT_ATTEMPT=0
:wait_for_file_loop
if exist "%WAIT_PATH%" (
    echo [PASS] %WAIT_LABEL%>> "%DETAILS_PATH%"
    exit /b 0
)
set /a WAIT_ATTEMPT+=1
if %WAIT_ATTEMPT% GEQ 16 (
    echo [FAIL] %WAIT_LABEL% missing after packaged host launch: %WAIT_PATH%>> "%DETAILS_PATH%"
    set "FAILED=1"
    exit /b 0
)
ping 127.0.0.1 -n 2 >nul
goto :wait_for_file_loop

:check_file
if exist "%~1" (
    echo [PASS] %~2>> "%DETAILS_PATH%"
) else (
    echo [FAIL] %~2 missing: %~1>> "%DETAILS_PATH%"
    set "FAILED=1"
)
exit /b 0

:require_line
findstr /b /c:"%~2" "%~1" >nul
if errorlevel 1 (
    echo [FAIL] %~3 ^(%~2^)>> "%DETAILS_PATH%"
    set "FAILED=1"
) else (
    echo [PASS] %~3 ^(%~2^)>> "%DETAILS_PATH%"
)
exit /b 0
