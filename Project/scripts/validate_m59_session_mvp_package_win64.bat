@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
if not exist "%REPO_ROOT%\WAR.sln" (
    for %%I in ("%REPO_ROOT%\..") do set "REPO_ROOT=%%~fI"
)
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "STAGE_ROOT=%REPO_ROOT%\out\session_mvp_candidate\WAR_M59_%CONFIG%"
set "REPORT_PATH=%REPO_ROOT%\out\session_mvp_candidate\m59_validation_report_%CONFIG%.txt"

if not exist "%REPO_ROOT%\out\session_mvp_candidate" mkdir "%REPO_ROOT%\out\session_mvp_candidate" >nul 2>nul
call "%SCRIPT_DIR%build_session_mvp_candidate_package_win64.bat" "%CONFIG%"
if errorlevel 1 goto :build_fail

pushd "%STAGE_ROOT%" >nul
call "%STAGE_ROOT%\smoke_test_headless_host_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\smoke_test_local_demo_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m54_session_mvp_win64.bat" "%CONFIG%"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m55_session_mvp_win64.bat" "%CONFIG%"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m56_session_mvp_win64.bat" "%CONFIG%"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m57_session_mvp_win64.bat" "%CONFIG%"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m58_session_mvp_win64.bat" "%CONFIG%"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m59_session_mvp_win64.bat" "%CONFIG%"
if errorlevel 1 goto :stage_fail
popd >nul

(
    echo WAR M59 Session MVP Aggregate Validation
    echo Result: PASS
    echo Stage root: %STAGE_ROOT%
    echo Scripts run:
    echo - smoke_test_headless_host_win64.bat
    echo - smoke_test_local_demo_win64.bat
    echo - validate_m54_session_mvp_win64.bat
    echo - validate_m55_session_mvp_win64.bat
    echo - validate_m56_session_mvp_win64.bat
    echo - validate_m57_session_mvp_win64.bat
    echo - validate_m58_session_mvp_win64.bat
    echo - validate_m59_session_mvp_win64.bat
    echo.
    echo Automated carry-forward validation is complete.
    echo Final M59 sign-off still requires the live two-participant scenario defined by:
    echo - %STAGE_ROOT%\Content\SessionMvp\session_mvp_authoring.txt
    echo - %STAGE_ROOT%\Docs\WAR Alpha Handoff.md
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 0

:build_fail
(
    echo WAR M59 Session MVP Aggregate Validation
    echo Result: FAIL
    echo Stage failed during build/package.
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 1

:stage_fail
popd >nul
(
    echo WAR M59 Session MVP Aggregate Validation
    echo Result: FAIL
    echo Stage root: %STAGE_ROOT%
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 1
