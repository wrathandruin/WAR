@echo off
setlocal enableextensions

set "SCRIPT_DIR=%~dp0"
set "PACKAGE_ROOT=%SCRIPT_DIR%"
if not exist "%PACKAGE_ROOT%\WAR.exe" (
    for %%I in ("%SCRIPT_DIR%..") do set "PACKAGE_ROOT=%%~fI"
)

echo [M49] Opening beta release candidate package root:
echo %PACKAGE_ROOT%
start "" "%PACKAGE_ROOT%"
exit /b 0
