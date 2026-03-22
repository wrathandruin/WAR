@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
if exist "%SCRIPT_DIR%smoke_test_local_demo_win64.bat" (
    pushd "%SCRIPT_DIR%" >nul
    set "STAGED_ROOT=%CD%"
    popd >nul
    for %%I in ("%STAGED_ROOT%\..") do set "REPORT_ROOT=%%~fI"
) else (
    for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
if not exist "%REPO_ROOT%\WAR.sln" (
    for %%I in ("%REPO_ROOT%\..") do set "REPO_ROOT=%%~fI"
)
    set "STAGED_ROOT=%REPO_ROOT%\out\session_mvp_candidate\WAR_M59_%CONFIG%"
    set "REPORT_ROOT=%REPO_ROOT%\out\session_mvp_candidate"
)
set "LOG_DIR=%STAGED_ROOT%\runtime\Logs"
set "STATUS_FILE=%LOG_DIR%\client_replication_status.txt"
set "SMOKE_SCRIPT=%STAGED_ROOT%\smoke_test_local_demo_win64.bat"
set "SMOKE_FILE=%LOG_DIR%\m59_session_mvp_smoke_test.txt"
set "REPORT_PATH=%REPORT_ROOT%\m59_stage_validation_report_%CONFIG%.txt"
set "DETAILS_PATH=%REPORT_ROOT%\m59_stage_validation_details_%CONFIG%.txt"

if not exist "%REPORT_ROOT%" mkdir "%REPORT_ROOT%" >nul 2>nul
if exist "%DETAILS_PATH%" del /q "%DETAILS_PATH%" >nul 2>nul
set "FAILED=0"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>nul

if not exist "%STAGED_ROOT%" (
    echo [FAIL] Stage root missing: %STAGED_ROOT%>> "%DETAILS_PATH%"
    set "FAILED=1"
    goto :report
)

if not exist "%STATUS_FILE%" (
    if not exist "%SMOKE_SCRIPT%" (
        echo [FAIL] Runtime status missing and smoke script unavailable: %SMOKE_SCRIPT%>> "%DETAILS_PATH%"
        set "FAILED=1"
        goto :report
    )
    call "%SMOKE_SCRIPT%" >nul
)

call :require_prefix "%STATUS_FILE%" "room_title=" "Room title is present"
call :require_prefix "%STATUS_FILE%" "room_description=" "Room description is present"
call :require_prefix "%STATUS_FILE%" "prompt_line=" "Prompt line is present"
call :require_prefix "%STATUS_FILE%" "command_bar=" "Command bar is present"
call :require_prefix "%STATUS_FILE%" "command_discoverability=" "Command discoverability is present"
call :require_line "%STATUS_FILE%" "social_visibility_rule=same-location-key" "M54 social rule is present"
call :require_line "%STATUS_FILE%" "presence_interest_rule=same-location-key+fresh-heartbeat" "M55 presence rule is present"
call :require_prefix "%STATUS_FILE%" "continuity_summary=" "M56 continuity summary is present"
call :require_prefix "%STATUS_FILE%" "authoring_content_loaded=" "M57 authoring load state is present"
call :require_prefix "%STATUS_FILE%" "mission_journal=" "M57 mission journal is present"
call :require_prefix "%STATUS_FILE%" "environment_identity=" "M58 environment identity is present"
call :require_prefix "%STATUS_FILE%" "text_presentation_profile=" "M58 presentation profile is present"

:report
(
    echo WAR M59 Session MVP Stage Validation
    if "%FAILED%"=="0" (echo Result: PASS) else (echo Result: FAIL)
    echo Stage root: %STAGED_ROOT%
    echo Runtime status: %STATUS_FILE%
    echo.
    if exist "%DETAILS_PATH%" type "%DETAILS_PATH%"
) > "%REPORT_PATH%"

if "%FAILED%"=="0" (
    findstr /i /c:"validated shared location key" "%SMOKE_FILE%" >nul 2>nul
    if errorlevel 1 (
        (
            echo WAR M59 Session-MVP Validation
            echo Status: AUTOMATED CARRY-FORWARD CHECKS PASSED ^- LIVE TWO-PARTICIPANT SCENARIO STILL REQUIRED
            echo.
            echo Stage root: %STAGED_ROOT%
            echo Runtime status field presence checks passed for the official M51-M58 carry-forward set.
            echo Complete the live M59 cooperation beat defined by:
            echo Content\SessionMvp\session_mvp_authoring.txt
            echo Docs\WAR Alpha Handoff.md
            echo.
            echo Replace this file with honest staged evidence after the two-participant scenario run.
        ) > "%SMOKE_FILE%"
    )
)

type "%REPORT_PATH%"
if "%FAILED%"=="0" exit /b 0
exit /b 1

:require_line
findstr /b /c:"%~2" "%~1" >nul 2>nul
if errorlevel 1 (
    echo [FAIL] %~3>> "%DETAILS_PATH%"
    set "FAILED=1"
) else (
    echo [PASS] %~3>> "%DETAILS_PATH%"
)
exit /b 0

:require_prefix
findstr /b /c:"%~2" "%~1" >nul 2>nul
if errorlevel 1 (
    echo [FAIL] %~3>> "%DETAILS_PATH%"
    set "FAILED=1"
) else (
    echo [PASS] %~3>> "%DETAILS_PATH%"
)
exit /b 0
