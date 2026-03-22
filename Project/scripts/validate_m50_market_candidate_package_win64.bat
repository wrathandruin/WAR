@echo off
setlocal enableextensions enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
if not exist "%REPO_ROOT%\WAR.sln" (
    for %%I in ("%REPO_ROOT%\..") do set "REPO_ROOT=%%~fI"
)
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
set "STAGE_ROOT=%REPO_ROOT%\out\market_candidate\WAR_M50_%CONFIG%"
set "REPORT_PATH=%REPO_ROOT%\out\market_candidate\m50_validation_report_%CONFIG%.txt"

call "%SCRIPT_DIR%build_market_candidate_package_win64.bat" "%CONFIG%"
if errorlevel 1 goto :build_fail

pushd "%STAGE_ROOT%" >nul
call "%STAGE_ROOT%\smoke_test_headless_host_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\smoke_test_local_demo_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m45_hosted_bootstrap_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m46_environment_identity_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m46_missing_required_secrets_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m46_runtime_save_hygiene_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m47_ticket_issue_and_client_entry_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m47_ticket_denial_and_fail_states_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m47_reconnect_identity_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m48_failure_bundle_capture_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m48_operator_triage_artifacts_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m49_beta_content_scale_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m49_retention_expansion_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m49_release_candidate_discipline_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m50_launcher_productionization_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m50_installer_productionization_win64.bat"
if errorlevel 1 goto :stage_fail
call "%STAGE_ROOT%\validate_m50_update_productionization_win64.bat"
if errorlevel 1 goto :stage_fail
popd >nul

(
    echo WAR M50 Market Candidate Validation
    echo Result: PASS
    echo Stage root: %STAGE_ROOT%
    echo Scripts run:
    echo - smoke_test_headless_host_win64.bat
    echo - smoke_test_local_demo_win64.bat
    echo - validate_m45_hosted_bootstrap_win64.bat
    echo - validate_m46_environment_identity_win64.bat
    echo - validate_m46_missing_required_secrets_win64.bat
    echo - validate_m46_runtime_save_hygiene_win64.bat
    echo - validate_m47_ticket_issue_and_client_entry_win64.bat
    echo - validate_m47_ticket_denial_and_fail_states_win64.bat
    echo - validate_m47_reconnect_identity_win64.bat
    echo - validate_m48_failure_bundle_capture_win64.bat
    echo - validate_m48_operator_triage_artifacts_win64.bat
    echo - validate_m49_beta_content_scale_win64.bat
    echo - validate_m49_retention_expansion_win64.bat
    echo - validate_m49_release_candidate_discipline_win64.bat
    echo - validate_m50_launcher_productionization_win64.bat
    echo - validate_m50_installer_productionization_win64.bat
    echo - validate_m50_update_productionization_win64.bat
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 0

:build_fail
(
    echo WAR M50 Market Candidate Validation
    echo Result: FAIL
    echo Stage failed during build/package.
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 1

:stage_fail
popd >nul
(
    echo WAR M50 Market Candidate Validation
    echo Result: FAIL
    echo Stage root: %STAGE_ROOT%
) > "%REPORT_PATH%"
type "%REPORT_PATH%"
exit /b 1
