@echo off
setlocal enableextensions

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

set "RUNTIME_ROOT=%PACKAGE_ROOT%\runtime"
if not exist "%RUNTIME_ROOT%" set "RUNTIME_ROOT=%PACKAGE_ROOT%\Runtime"

set "TRIAGE_DIR=%RUNTIME_ROOT%\CrashDumps\FailureBundles\OperatorTriage"

if not exist "%TRIAGE_DIR%" (
    echo [M48] ERROR: operator triage directory not found:
    echo %TRIAGE_DIR%
    exit /b 1
)

echo [M48] Opening operator triage directory:
echo %TRIAGE_DIR%
start "" "%TRIAGE_DIR%"
exit /b 0
