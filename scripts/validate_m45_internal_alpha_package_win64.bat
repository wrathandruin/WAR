@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "STAGE_ROOT=%REPO_ROOT%\out\internal_alpha\WAR_M45_%CONFIG%"
set "REPORT_PATH=%REPO_ROOT%\out\internal_alpha\m45_validation_report_%CONFIG%.txt"

call "%SCRIPT_DIR%build_internal_alpha_package_win64.bat" "%CONFIG%"
if errorlevel 1 (
    echo WAR M45 Internal Alpha Validation > "%REPORT_PATH%"
    echo Result: FAIL >> "%REPORT_PATH%"
    echo Stage failed during build/package. >> "%REPORT_PATH%"
    type "%REPORT_PATH%"
    exit /b 1
)

pushd "%STAGE_ROOT%" >nul
call "%STAGE_ROOT%\smoke_test_headless_host_win64.bat"
if errorlevel 1 goto :fail
call "%STAGE_ROOT%\smoke_test_local_demo_win64.bat"
if errorlevel 1 goto :fail
call "%STAGE_ROOT%\validate_m45_hosted_bootstrap_win64.bat"
if errorlevel 1 goto :fail
popd >nul

(
    echo WAR M45 Internal Alpha Validation
    echo Result: PASS
    echo Stage root: %STAGE_ROOT%
    echo Scripts run:
    echo - smoke_test_headless_host_win64.bat
    echo - smoke_test_local_demo_win64.bat
    echo - validate_m45_hosted_bootstrap_win64.bat
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 0

:fail
popd >nul
(
    echo WAR M45 Internal Alpha Validation
    echo Result: FAIL
    echo Stage root: %STAGE_ROOT%
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 1
