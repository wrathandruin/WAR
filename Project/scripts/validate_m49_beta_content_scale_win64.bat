@echo off
setlocal enableextensions

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "CONTENT_MANIFEST=%PACKAGE_ROOT%\BetaContent\m49_beta_content_manifest.txt"
set "JOURNEY_CARDS=%PACKAGE_ROOT%\BetaContent\m49_retention_journey_cards.txt"
set "REPORT_PATH=%PACKAGE_ROOT%\runtime\Logs\m49_beta_content_scale_validation.txt"

set "FAILED=0"

if not exist "%CONTENT_MANIFEST%" (
    echo [FAIL] Beta content manifest missing>"%REPORT_PATH%"
    exit /b 1
)

if not exist "%JOURNEY_CARDS%" (
    echo [FAIL] Retention journey cards missing>"%REPORT_PATH%"
    exit /b 1
)

findstr /b /c:"content_card_count=4" "%CONTENT_MANIFEST%" >nul || set "FAILED=1"
findstr /b /c:"card_04_id=frontier_pad_reentry" "%CONTENT_MANIFEST%" >nul || set "FAILED=1"
findstr /b /c:"step_04=repeat_session_resume" "%JOURNEY_CARDS%" >nul || set "FAILED=1"

(
    echo WAR M49 Beta Content Scale Validation
    if "%FAILED%"=="0" (echo Result: PASS) else (echo Result: FAIL)
    echo Content manifest: %CONTENT_MANIFEST%
    echo Journey cards: %JOURNEY_CARDS%
) > "%REPORT_PATH%"

type "%REPORT_PATH%"
if "%FAILED%"=="0" exit /b 0
exit /b 1
