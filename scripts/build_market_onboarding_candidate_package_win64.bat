@echo off
setlocal enableextensions enabledelayedexpansion

echo [M51] Market onboarding package build wrapper
set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"

echo [M51] This milestone defers the aggregate M51-M59 validation lane until M59.
echo [M51] Stage target: %REPO_ROOT%\out\market_onboarding\WAR_M51_%CONFIG%
exit /b 0
