@echo off
setlocal
set "SCRIPT_DIR=%~dp0"
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%acceptance_m42_ship_runtime_boarding_win64.ps1" %*
exit /b %ERRORLEVEL%
