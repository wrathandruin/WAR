@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "DEMO_ROOT=%SCRIPT_DIR%"
if not exist "%DEMO_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "DEMO_ROOT=%%~fI"
)

set "EXE_PATH=%DEMO_ROOT%\WAR.exe"
set "ASSET_ROOT=%DEMO_ROOT%\assets"
set "RUNTIME_ROOT=%DEMO_ROOT%\runtime"
if not exist "%RUNTIME_ROOT%" set "RUNTIME_ROOT=%DEMO_ROOT%\Runtime"
set "LOG_DIR=%RUNTIME_ROOT%\Logs"
set "REPORT_PATH=%LOG_DIR%\local_demo_smoke_test.txt"
set "DETAILS_PATH=%LOG_DIR%\local_demo_smoke_test_details.txt"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul
if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul

set "FAILED=0"

call :check_file "%EXE_PATH%" "WAR executable"
call :check_dir "%ASSET_ROOT%\shaders\dx11" "DX11 shader directory"
call :check_dir "%ASSET_ROOT%\textures" "Texture directory"
call :check_dir "%RUNTIME_ROOT%\Config" "Runtime config directory"
call :check_dir "%RUNTIME_ROOT%\Logs" "Runtime logs directory"
call :check_dir "%RUNTIME_ROOT%\Saves" "Runtime saves directory"
call :check_dir "%RUNTIME_ROOT%\CrashDumps" "Runtime crash directory"

(
    echo WAR Local Demo Smoke Test
    echo Demo root: %DEMO_ROOT%
    echo Executable: %EXE_PATH%
    echo Asset root: %ASSET_ROOT%
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
    echo [M32] Smoke test passed.
    exit /b 0
)

echo [M32] Smoke test failed.
exit /b 1

:check_file
if exist "%~1" (
    echo [PASS] %~2>> "%DETAILS_PATH%"
) else (
    echo [FAIL] %~2 missing: %~1>> "%DETAILS_PATH%"
    set "FAILED=1"
)
exit /b 0

:check_dir
if exist "%~1" (
    echo [PASS] %~2>> "%DETAILS_PATH%"
) else (
    echo [FAIL] %~2 missing: %~1>> "%DETAILS_PATH%"
    set "FAILED=1"
)
exit /b 0
