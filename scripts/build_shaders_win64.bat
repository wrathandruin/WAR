@echo off
setlocal EnableExtensions

set "ROOT=%~1"
if "%ROOT%"=="" (
    set "ROOT=%~dp0.."
)

set "CONFIG=%~2"
if "%CONFIG%"=="" (
    set "CONFIG=Debug"
)

set "SHADER_SRC_DIR=%ROOT%\assets\shaders\src"
set "SHADER_OUT_DIR=%ROOT%\assets\shaders\dx11"
set "BGFX_SRC_INCLUDE=%ROOT%\third_party\bgfx\src"
set "COLOR_VARYING_DEF=%SHADER_SRC_DIR%\varying.def.sc"
set "TEXTURE_VARYING_DEF=%SHADER_SRC_DIR%\varying_texture.def.sc"

if /I "%CONFIG%"=="Debug" (
    set "SHADERC=%ROOT%\third_party\bgfx\.build\win64_vs2022\bin\shadercDebug.exe"
) else (
    set "SHADERC=%ROOT%\third_party\bgfx\.build\win64_vs2022\bin\shadercRelease.exe"
)

if not exist "%SHADER_OUT_DIR%" (
    mkdir "%SHADER_OUT_DIR%"
)

if not exist "%SHADERC%" (
    echo [WAR:M20] shaderc not found at "%SHADERC%". Skipping shader compilation.
    exit /b 0
)

if not exist "%SHADER_SRC_DIR%\vs_color.sc" (
    echo [WAR:M20] missing shader source "%SHADER_SRC_DIR%\vs_color.sc"
    exit /b 1
)

if not exist "%SHADER_SRC_DIR%\fs_color.sc" (
    echo [WAR:M20] missing shader source "%SHADER_SRC_DIR%\fs_color.sc"
    exit /b 1
)

if not exist "%SHADER_SRC_DIR%\vs_texture.sc" (
    echo [WAR:M20] missing shader source "%SHADER_SRC_DIR%\vs_texture.sc"
    exit /b 1
)

if not exist "%SHADER_SRC_DIR%\fs_texture.sc" (
    echo [WAR:M20] missing shader source "%SHADER_SRC_DIR%\fs_texture.sc"
    exit /b 1
)

if not exist "%COLOR_VARYING_DEF%" (
    echo [WAR:M20] missing varying definition "%COLOR_VARYING_DEF%"
    exit /b 1
)

if not exist "%TEXTURE_VARYING_DEF%" (
    echo [WAR:M20] missing varying definition "%TEXTURE_VARYING_DEF%"
    exit /b 1
)

echo [WAR:M20] compiling bgfx shaders for %CONFIG%...

"%SHADERC%" -f "%SHADER_SRC_DIR%\vs_color.sc" -o "%SHADER_OUT_DIR%\vs_color.bin" --platform windows --type vertex -p s_5_0 -O 3 --varyingdef "%COLOR_VARYING_DEF%" -i "%BGFX_SRC_INCLUDE%"
if errorlevel 1 (
    echo [WAR:M20] color vertex shader compilation failed
    exit /b 1
)

"%SHADERC%" -f "%SHADER_SRC_DIR%\fs_color.sc" -o "%SHADER_OUT_DIR%\fs_color.bin" --platform windows --type fragment -p s_5_0 -O 3 --varyingdef "%COLOR_VARYING_DEF%" -i "%BGFX_SRC_INCLUDE%"
if errorlevel 1 (
    echo [WAR:M20] color fragment shader compilation failed
    exit /b 1
)

"%SHADERC%" -f "%SHADER_SRC_DIR%\vs_texture.sc" -o "%SHADER_OUT_DIR%\vs_texture.bin" --platform windows --type vertex -p s_5_0 -O 3 --varyingdef "%TEXTURE_VARYING_DEF%" -i "%BGFX_SRC_INCLUDE%"
if errorlevel 1 (
    echo [WAR:M20] texture vertex shader compilation failed
    exit /b 1
)

"%SHADERC%" -f "%SHADER_SRC_DIR%\fs_texture.sc" -o "%SHADER_OUT_DIR%\fs_texture.bin" --platform windows --type fragment -p s_5_0 -O 3 --varyingdef "%TEXTURE_VARYING_DEF%" -i "%BGFX_SRC_INCLUDE%"
if errorlevel 1 (
    echo [WAR:M20] texture fragment shader compilation failed
    exit /b 1
)

echo [WAR:M20] shader compilation complete
exit /b 0
