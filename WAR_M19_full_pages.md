# WAR M19 — Full Pages

These are complete file contents for the M19 package.

## `CMakeLists.txt`

```text
cmake_minimum_required(VERSION 3.21)

project(WAR
    VERSION 0.3.2
    DESCRIPTION "Wrath and Ruin milestone 19"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WAR_SOURCES
    src/main.cpp
    src/engine/core/Application.cpp
    src/engine/core/Timer.cpp
    src/engine/core/Log.cpp
    src/engine/math/Vec2.cpp
    src/engine/render/Camera2D.cpp
    src/engine/render/WorldRenderer.cpp
    src/engine/render/BgfxWorldRenderer.cpp
    src/engine/render/BgfxRenderData.cpp
    src/engine/render/BgfxViewTransform.cpp
    src/engine/render/BgfxShaderProgram.cpp
    src/engine/render/RenderAssetPaths.cpp
    src/engine/render/DebugOverlayRenderer.cpp
    src/engine/render/GdiRenderDevice.cpp
    src/engine/render/BgfxRenderDevice.cpp
    src/engine/render/BgfxDebugFrameRenderer.cpp
    src/engine/world/WorldGrid.cpp
    src/engine/world/Pathfinding.cpp
    src/engine/world/WorldState.cpp
    src/engine/gameplay/EntityManager.cpp
    src/engine/gameplay/ActionSystem.cpp
    src/platform/win32/Win32Window.cpp
    src/game/GameLayer.cpp
)

add_executable(WAR WIN32
    ${WAR_SOURCES}
)

target_include_directories(WAR PRIVATE
    src
)

if(WIN32)
    target_compile_definitions(WAR PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

target_link_libraries(WAR PRIVATE
    user32
    gdi32
)

if(MSVC)
    target_compile_options(WAR PRIVATE /W4 /permissive-)
else()
    target_compile_options(WAR PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(WAR_BGFX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/bgfx/include")
set(WAR_BX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/bx/include")
set(WAR_BGFX_DEBUG_LIB_DIR "${CMAKE_SOURCE_DIR}/third_party/bgfx/lib/win64/debug")
set(WAR_BGFX_RELEASE_LIB_DIR "${CMAKE_SOURCE_DIR}/third_party/bgfx/lib/win64/release")
set(WAR_SHADER_BUILD_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/build_shaders_win64.bat")

if(EXISTS "${WAR_BGFX_INCLUDE_DIR}" AND EXISTS "${WAR_BX_INCLUDE_DIR}")
    target_include_directories(WAR PRIVATE
        "${WAR_BGFX_INCLUDE_DIR}"
        "${WAR_BX_INCLUDE_DIR}"
    )

    if(WIN32 AND EXISTS "${WAR_BGFX_DEBUG_LIB_DIR}" AND EXISTS "${WAR_BGFX_RELEASE_LIB_DIR}")
        target_link_directories(WAR PRIVATE
            $<$<CONFIG:Debug>:${WAR_BGFX_DEBUG_LIB_DIR}>
            $<$<CONFIG:Release>:${WAR_BGFX_RELEASE_LIB_DIR}>
            $<$<CONFIG:RelWithDebInfo>:${WAR_BGFX_RELEASE_LIB_DIR}>
            $<$<CONFIG:MinSizeRel>:${WAR_BGFX_RELEASE_LIB_DIR}>
        )

        target_link_libraries(WAR PRIVATE
            $<$<CONFIG:Debug>:bgfxDebug>
            $<$<CONFIG:Debug>:bxDebug>
            $<$<CONFIG:Debug>:bimgDebug>
            $<$<CONFIG:Release>:bgfxRelease>
            $<$<CONFIG:Release>:bxRelease>
            $<$<CONFIG:Release>:bimgRelease>
            $<$<CONFIG:RelWithDebInfo>:bgfxRelease>
            $<$<CONFIG:RelWithDebInfo>:bxRelease>
            $<$<CONFIG:RelWithDebInfo>:bimgRelease>
            $<$<CONFIG:MinSizeRel>:bgfxRelease>
            $<$<CONFIG:MinSizeRel>:bxRelease>
            $<$<CONFIG:MinSizeRel>:bimgRelease>
        )
    endif()
endif()

if(WIN32 AND EXISTS "${WAR_SHADER_BUILD_SCRIPT}")
    add_custom_target(war_build_shaders
        COMMAND cmd /c ""${WAR_SHADER_BUILD_SCRIPT}" "${CMAKE_SOURCE_DIR}" "$<CONFIG>""
        BYPRODUCTS
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/vs_color.bin"
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/fs_color.bin"
        VERBATIM
    )

    add_dependencies(WAR war_build_shaders)

    add_custom_command(TARGET WAR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WAR>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11"
            "$<TARGET_FILE_DIR:WAR>/assets/shaders/dx11"
        VERBATIM
    )
endif()

set_target_properties(WAR PROPERTIES
    OUTPUT_NAME "WAR"
)
```

## `WAR.vcxproj`

```xml
\
    <?xml version="1.0" encoding="utf-8"?>
    <Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <ItemGroup Label="ProjectConfigurations">
        <ProjectConfiguration Include="Debug|x64">
          <Configuration>Debug</Configuration>
          <Platform>x64</Platform>
        </ProjectConfiguration>
        <ProjectConfiguration Include="Release|x64">
          <Configuration>Release</Configuration>
          <Platform>x64</Platform>
        </ProjectConfiguration>
      </ItemGroup>
      <PropertyGroup Label="Globals">
        <VCProjectVersion>17.0</VCProjectVersion>
        <Keyword>Win32Proj</Keyword>
        <ProjectGuid>{5ECF3E8A-6D76-4BF2-B2B6-7E93F4C8A1D2}</ProjectGuid>
        <RootNamespace>WAR</RootNamespace>
        <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
      </PropertyGroup>
      <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
      <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
        <ConfigurationType>Application</ConfigurationType>
        <UseDebugLibraries>true</UseDebugLibraries>
        <PlatformToolset>v143</PlatformToolset>
        <CharacterSet>Unicode</CharacterSet>
      </PropertyGroup>
      <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
        <ConfigurationType>Application</ConfigurationType>
        <UseDebugLibraries>false</UseDebugLibraries>
        <PlatformToolset>v143</PlatformToolset>
        <WholeProgramOptimization>true</WholeProgramOptimization>
        <CharacterSet>Unicode</CharacterSet>
      </PropertyGroup>
      <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
      <ImportGroup Label="ExtensionSettings" />
      <ImportGroup Label="Shared" />
      <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
        <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
      </ImportGroup>
      <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
        <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
      </ImportGroup>
      <PropertyGroup Label="UserMacros" />
      <PropertyGroup>
        <OutDir>$(SolutionDir)bin\$(Configuration)\</OutDir>
        <IntDir>$(SolutionDir)obj\$(Configuration)\</IntDir>
        <TargetName>WAR</TargetName>
      </PropertyGroup>
      <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
        <ClCompile>
          <WarningLevel>Level4</WarningLevel>
          <SDLCheck>true</SDLCheck>
          <PreprocessorDefinitions>WIN32;_WINDOWS;BX_CONFIG_DEBUG=1;%(PreprocessorDefinitions)</PreprocessorDefinitions>
          <ConformanceMode>true</ConformanceMode>
          <LanguageStandard>stdcpp20</LanguageStandard>
          <AdditionalOptions>/Zc:__cplusplus /Zc:preprocessor %(AdditionalOptions)</AdditionalOptions>
          <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
          <AdditionalIncludeDirectories>$(ProjectDir)src;$(ProjectDir)third_party\bgfx\include;$(ProjectDir)third_party\bx\include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
        </ClCompile>
        <PreBuildEvent>
          <Command>call "$(ProjectDir)scripts\build_shaders_win64.bat" "$(ProjectDir)" "$(Configuration)"</Command>
        </PreBuildEvent>
        <Link>
          <SubSystem>Windows</SubSystem>
          <GenerateDebugInformation>true</GenerateDebugInformation>
          <AdditionalDependencies>user32.lib;gdi32.lib;bgfxDebug.lib;bxDebug.lib;bimgDebug.lib;%(AdditionalDependencies)</AdditionalDependencies>
          <AdditionalLibraryDirectories>$(ProjectDir)third_party\bgfx\lib\win64\debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
        </Link>
        <PostBuildEvent>
          <Command>if exist "$(ProjectDir)assets\shaders\dx11" xcopy /Y /I /E "$(ProjectDir)assets\shaders\dx11" "$(OutDir)assets\shaders\dx11\"</Command>
        </PostBuildEvent>
      </ItemDefinitionGroup>
      <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
        <ClCompile>
          <WarningLevel>Level4</WarningLevel>
          <FunctionLevelLinking>true</FunctionLevelLinking>
          <IntrinsicFunctions>true</IntrinsicFunctions>
          <SDLCheck>true</SDLCheck>
          <PreprocessorDefinitions>WIN32;_WINDOWS;NDEBUG;BX_CONFIG_DEBUG=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>
          <ConformanceMode>true</ConformanceMode>
          <LanguageStandard>stdcpp20</LanguageStandard>
          <AdditionalOptions>/Zc:__cplusplus /Zc:preprocessor %(AdditionalOptions)</AdditionalOptions>
          <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
          <AdditionalIncludeDirectories>$(ProjectDir)src;$(ProjectDir)third_party\bgfx\include;$(ProjectDir)third_party\bx\include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
        </ClCompile>
        <PreBuildEvent>
          <Command>call "$(ProjectDir)scripts\build_shaders_win64.bat" "$(ProjectDir)" "$(Configuration)"</Command>
        </PreBuildEvent>
        <Link>
          <SubSystem>Windows</SubSystem>
          <EnableCOMDATFolding>true</EnableCOMDATFolding>
          <OptimizeReferences>true</OptimizeReferences>
          <GenerateDebugInformation>true</GenerateDebugInformation>
          <AdditionalDependencies>user32.lib;gdi32.lib;bgfxRelease.lib;bxRelease.lib;bimgRelease.lib;%(AdditionalDependencies)</AdditionalDependencies>
          <AdditionalLibraryDirectories>$(ProjectDir)third_party\bgfx\lib\win64\release;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
        </Link>
        <PostBuildEvent>
          <Command>if exist "$(ProjectDir)assets\shaders\dx11" xcopy /Y /I /E "$(ProjectDir)assets\shaders\dx11" "$(OutDir)assets\shaders\dx11\"</Command>
        </PostBuildEvent>
      </ItemDefinitionGroup>
      <ItemGroup>
        <ClCompile Include="src\main.cpp" />
        <ClCompile Include="src\engine\core\Application.cpp" />
        <ClCompile Include="src\engine\core\Timer.cpp" />
        <ClCompile Include="src\engine\core\Log.cpp" />
        <ClCompile Include="src\engine\math\Vec2.cpp" />
        <ClCompile Include="src\engine\render\Camera2D.cpp" />
        <ClCompile Include="src\engine\render\WorldRenderer.cpp" />
        <ClCompile Include="src\engine\render\BgfxWorldRenderer.cpp" />
        <ClCompile Include="src\engine\render\BgfxRenderData.cpp" />
        <ClCompile Include="src\engine\render\BgfxViewTransform.cpp" />
        <ClCompile Include="src\engine\render\BgfxShaderProgram.cpp" />
        <ClCompile Include="src\engine\render\RenderAssetPaths.cpp" />
        <ClCompile Include="src\engine\render\DebugOverlayRenderer.cpp" />
        <ClCompile Include="src\engine\render\GdiRenderDevice.cpp" />
        <ClCompile Include="src\engine\render\BgfxRenderDevice.cpp" />
        <ClCompile Include="src\engine\render\BgfxDebugFrameRenderer.cpp" />
        <ClCompile Include="src\engine\world\WorldGrid.cpp" />
        <ClCompile Include="src\engine\world\Pathfinding.cpp" />
        <ClCompile Include="src\engine\world\WorldState.cpp" />
        <ClCompile Include="src\engine\gameplay\EntityManager.cpp" />
        <ClCompile Include="src\engine\gameplay\ActionSystem.cpp" />
        <ClCompile Include="src\platform\win32\Win32Window.cpp" />
        <ClCompile Include="src\game\GameLayer.cpp" />
      </ItemGroup>
      <ItemGroup>
        <ClInclude Include="src\engine\core\Application.h" />
        <ClInclude Include="src\engine\core\Timer.h" />
        <ClInclude Include="src\engine\core\Log.h" />
        <ClInclude Include="src\engine\math\Vec2.h" />
        <ClInclude Include="src\engine\render\Camera2D.h" />
        <ClInclude Include="src\engine\render\WorldRenderer.h" />
        <ClInclude Include="src\engine\render\BgfxWorldRenderer.h" />
        <ClInclude Include="src\engine\render\BgfxRenderData.h" />
        <ClInclude Include="src\engine\render\BgfxViewTransform.h" />
        <ClInclude Include="src\engine\render\BgfxShaderProgram.h" />
        <ClInclude Include="src\engine\render\RenderAssetPaths.h" />
        <ClInclude Include="src\engine\render\DebugOverlayRenderer.h" />
        <ClInclude Include="src\engine\render\RenderBackendType.h" />
        <ClInclude Include="src\engine\render\IRenderDevice.h" />
        <ClInclude Include="src\engine\render\GdiRenderDevice.h" />
        <ClInclude Include="src\engine\render\BgfxRenderDevice.h" />
        <ClInclude Include="src\engine\render\BgfxDebugFrameRenderer.h" />
        <ClInclude Include="src\engine\world\WorldGrid.h" />
        <ClInclude Include="src\engine\world\Pathfinding.h" />
        <ClInclude Include="src\engine\world\WorldState.h" />
        <ClInclude Include="src\engine\gameplay\Action.h" />
        <ClInclude Include="src\engine\gameplay\ActionQueue.h" />
        <ClInclude Include="src\engine\gameplay\Entity.h" />
        <ClInclude Include="src\engine\gameplay\EntityManager.h" />
        <ClInclude Include="src\engine\gameplay\ActionSystem.h" />
        <ClInclude Include="src\platform\IWindow.h" />
        <ClInclude Include="src\platform\win32\Win32Window.h" />
        <ClInclude Include="src\game\GameLayer.h" />
      </ItemGroup>
      <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
      <ImportGroup Label="ExtensionTargets" />
    </Project>
```

## `WAR.vcxproj.filters`

```xml
\
    <?xml version="1.0" encoding="utf-8"?>
    <Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <ItemGroup>
        <Filter Include="Source Files">
          <UniqueIdentifier>{0E701D2D-3A50-4A17-BB5D-9453F2A77A10}</UniqueIdentifier>
          <Extensions>cpp;c;cc;cxx</Extensions>
        </Filter>
        <Filter Include="Header Files">
          <UniqueIdentifier>{A7F7C939-B61A-46EC-A255-7A671F4BE3C1}</UniqueIdentifier>
          <Extensions>h;hpp;hxx;inl</Extensions>
        </Filter>
      </ItemGroup>

      <ItemGroup>
        <ClCompile Include="src\main.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\core\Application.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\core\Timer.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\core\Log.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\math\Vec2.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\Camera2D.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\WorldRenderer.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\BgfxWorldRenderer.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\BgfxRenderData.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\BgfxViewTransform.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\BgfxShaderProgram.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\RenderAssetPaths.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\DebugOverlayRenderer.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\GdiRenderDevice.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\BgfxRenderDevice.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\render\BgfxDebugFrameRenderer.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\world\WorldGrid.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\world\Pathfinding.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\world\WorldState.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\gameplay\EntityManager.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\engine\gameplay\ActionSystem.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\platform\win32\Win32Window.cpp"><Filter>Source Files</Filter></ClCompile>
        <ClCompile Include="src\game\GameLayer.cpp"><Filter>Source Files</Filter></ClCompile>
      </ItemGroup>

      <ItemGroup>
        <ClInclude Include="src\engine\core\Application.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\core\Timer.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\core\Log.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\math\Vec2.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\Camera2D.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\WorldRenderer.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\BgfxWorldRenderer.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\BgfxRenderData.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\BgfxViewTransform.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\BgfxShaderProgram.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\RenderAssetPaths.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\DebugOverlayRenderer.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\RenderBackendType.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\IRenderDevice.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\GdiRenderDevice.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\BgfxRenderDevice.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\render\BgfxDebugFrameRenderer.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\world\WorldGrid.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\world\Pathfinding.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\world\WorldState.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\gameplay\Action.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\gameplay\ActionQueue.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\gameplay\Entity.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\gameplay\EntityManager.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\engine\gameplay\ActionSystem.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\platform\IWindow.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\platform\win32\Win32Window.h"><Filter>Header Files</Filter></ClInclude>
        <ClInclude Include="src\game\GameLayer.h"><Filter>Header Files</Filter></ClInclude>
      </ItemGroup>
    </Project>
```

## `scripts/build_shaders_win64.bat`

```bat
\
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
    set "VARYING_DEF=%SHADER_SRC_DIR%\varying.def.sc"

    if /I "%CONFIG%"=="Debug" (
        set "SHADERC=%ROOT%\third_party\bgfx\.build\win64_vs2022\bin\shadercDebug.exe"
    ) else (
        set "SHADERC=%ROOT%\third_party\bgfx\.build\win64_vs2022\bin\shadercRelease.exe"
    )

    if not exist "%SHADER_OUT_DIR%" (
        mkdir "%SHADER_OUT_DIR%"
    )

    if not exist "%SHADERC%" (
        echo [WAR:M19] shaderc not found at "%SHADERC%". Skipping shader compilation.
        exit /b 0
    )

    if not exist "%SHADER_SRC_DIR%\vs_color.sc" (
        echo [WAR:M19] missing shader source "%SHADER_SRC_DIR%\vs_color.sc"
        exit /b 1
    )

    if not exist "%SHADER_SRC_DIR%\fs_color.sc" (
        echo [WAR:M19] missing shader source "%SHADER_SRC_DIR%\fs_color.sc"
        exit /b 1
    )

    if not exist "%VARYING_DEF%" (
        echo [WAR:M19] missing varying definition "%VARYING_DEF%"
        exit /b 1
    )

    echo [WAR:M19] compiling bgfx shaders for %CONFIG%...

    "%SHADERC%" -f "%SHADER_SRC_DIR%\vs_color.sc" -o "%SHADER_OUT_DIR%\vs_color.bin" --platform windows --type vertex -p s_5_0 -O 3 --varyingdef "%VARYING_DEF%" -i "%BGFX_SRC_INCLUDE%"
    if errorlevel 1 (
        echo [WAR:M19] vertex shader compilation failed
        exit /b 1
    )

    "%SHADERC%" -f "%SHADER_SRC_DIR%\fs_color.sc" -o "%SHADER_OUT_DIR%\fs_color.bin" --platform windows --type fragment -p s_5_0 -O 3 --varyingdef "%VARYING_DEF%" -i "%BGFX_SRC_INCLUDE%"
    if errorlevel 1 (
        echo [WAR:M19] fragment shader compilation failed
        exit /b 1
    )

    echo [WAR:M19] shader compilation complete
    exit /b 0
```

## `src/engine/render/RenderAssetPaths.h`

```cpp
#pragma once

#include <string>

namespace war::RenderAssetPaths
{
    [[nodiscard]] std::string shaderBackendFolder();
    [[nodiscard]] std::string colorVertexShaderPath();
    [[nodiscard]] std::string colorFragmentShaderPath();
}
```

## `src/engine/render/RenderAssetPaths.cpp`

```cpp
#include "engine/render/RenderAssetPaths.h"

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>)
#    define WAR_HAS_BGFX 1
#  else
#    define WAR_HAS_BGFX 0
#  endif
#else
#  define WAR_HAS_BGFX 0
#endif

#if WAR_HAS_BGFX
#  include <bgfx/bgfx.h>
#endif

namespace war::RenderAssetPaths
{
    std::string shaderBackendFolder()
    {
#if WAR_HAS_BGFX
        switch (bgfx::getRendererType())
        {
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            return "dx11";

        case bgfx::RendererType::OpenGL:
            return "glsl";

        case bgfx::RendererType::Vulkan:
            return "spirv";

        default:
            return "";
        }
#else
        return "";
#endif
    }

    std::string colorVertexShaderPath()
    {
        const std::string folder = shaderBackendFolder();
        if (folder.empty())
        {
            return "";
        }

        return "assets/shaders/" + folder + "/vs_color.bin";
    }

    std::string colorFragmentShaderPath()
    {
        const std::string folder = shaderBackendFolder();
        if (folder.empty())
        {
            return "";
        }

        return "assets/shaders/" + folder + "/fs_color.bin";
    }
}
```

## `src/engine/render/BgfxShaderProgram.h`

```cpp
#pragma once

#include <string>

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>)
#    define WAR_HAS_BGFX 1
#  else
#    define WAR_HAS_BGFX 0
#  endif
#else
#  define WAR_HAS_BGFX 0
#endif

#if WAR_HAS_BGFX
#  include <bgfx/bgfx.h>
#endif

namespace war
{
    class BgfxShaderProgram
    {
    public:
        ~BgfxShaderProgram();

        bool loadColorProgram(std::string& outStatus);
        void shutdown();

        [[nodiscard]] bool isReady() const;
        [[nodiscard]] const std::string& statusMessage() const;

#if WAR_HAS_BGFX
        [[nodiscard]] bgfx::ProgramHandle handle() const;
#endif

    private:
        bool m_attemptedLoad = false;
        bool m_ready = false;
        std::string m_lastStatus = "bgfx color program not loaded";

#if WAR_HAS_BGFX
        bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
#endif
    };
}
```

## `src/engine/render/BgfxShaderProgram.cpp`

```cpp
#include "engine/render/BgfxShaderProgram.h"

#include <fstream>

#include "engine/render/RenderAssetPaths.h"

namespace war
{
#if WAR_HAS_BGFX
    namespace
    {
        const bgfx::Memory* loadMemoryFromFile(const char* path)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file)
            {
                return nullptr;
            }

            const std::streamsize size = file.tellg();
            if (size <= 0)
            {
                return nullptr;
            }

            file.seekg(0, std::ios::beg);

            const bgfx::Memory* memory = bgfx::alloc(static_cast<uint32_t>(size + 1));
            if (!file.read(reinterpret_cast<char*>(memory->data), size))
            {
                return nullptr;
            }

            memory->data[size] = 0;
            return memory;
        }
    }
#endif

    BgfxShaderProgram::~BgfxShaderProgram()
    {
        shutdown();
    }

    bool BgfxShaderProgram::loadColorProgram(std::string& outStatus)
    {
#if WAR_HAS_BGFX
        if (m_attemptedLoad)
        {
            outStatus = m_lastStatus;
            return m_ready;
        }

        m_attemptedLoad = true;

        const std::string vsPath = RenderAssetPaths::colorVertexShaderPath();
        const std::string fsPath = RenderAssetPaths::colorFragmentShaderPath();

        if (vsPath.empty() || fsPath.empty())
        {
            m_lastStatus = "unsupported bgfx renderer for shader asset folder";
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::Memory* vsMemory = loadMemoryFromFile(vsPath.c_str());
        if (vsMemory == nullptr)
        {
            m_lastStatus = "missing shader binary: " + vsPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::Memory* fsMemory = loadMemoryFromFile(fsPath.c_str());
        if (fsMemory == nullptr)
        {
            m_lastStatus = "missing shader binary: " + fsPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::ShaderHandle vs = bgfx::createShader(vsMemory);
        if (!bgfx::isValid(vs))
        {
            m_lastStatus = "failed to create bgfx vertex shader: " + vsPath;
            outStatus = m_lastStatus;
            return false;
        }

        const bgfx::ShaderHandle fs = bgfx::createShader(fsMemory);
        if (!bgfx::isValid(fs))
        {
            bgfx::destroy(vs);
            m_lastStatus = "failed to create bgfx fragment shader: " + fsPath;
            outStatus = m_lastStatus;
            return false;
        }

        m_program = bgfx::createProgram(vs, fs, true);
        if (!bgfx::isValid(m_program))
        {
            m_lastStatus = "failed to create bgfx shader program";
            outStatus = m_lastStatus;
            return false;
        }

        m_ready = true;
        m_lastStatus = "bgfx color program ready";
        outStatus = m_lastStatus;
        return true;
#else
        m_lastStatus = "bgfx headers not available at compile time";
        outStatus = m_lastStatus;
        return false;
#endif
    }

    void BgfxShaderProgram::shutdown()
    {
#if WAR_HAS_BGFX
        if (bgfx::isValid(m_program))
        {
            bgfx::destroy(m_program);
            m_program = BGFX_INVALID_HANDLE;
        }
#endif

        m_ready = false;
        m_attemptedLoad = false;
        m_lastStatus = "bgfx color program not loaded";
    }

    bool BgfxShaderProgram::isReady() const
    {
        return m_ready;
    }

    const std::string& BgfxShaderProgram::statusMessage() const
    {
        return m_lastStatus;
    }

#if WAR_HAS_BGFX
    bgfx::ProgramHandle BgfxShaderProgram::handle() const
    {
        return m_program;
    }
#endif
}
```

## `src/engine/render/BgfxWorldRenderer.h`

```cpp
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "engine/math/Vec2.h"
#include "engine/render/BgfxRenderData.h"
#include "engine/render/BgfxShaderProgram.h"
#include "engine/render/Camera2D.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldState.h"

namespace war
{
    class BgfxWorldRenderer
    {
    public:
        [[nodiscard]] bool render(
            const WorldState& worldState,
            const Camera2D& camera,
            const Vec2& playerPosition,
            const std::vector<TileCoord>& currentPath,
            size_t pathIndex,
            bool hasHoveredTile,
            TileCoord hoveredTile);

        void shutdown();

        [[nodiscard]] const std::string& statusMessage() const;

    private:
        [[nodiscard]] bool submitLayer(const BgfxRenderLayer& layer) const;

        BgfxShaderProgram m_colorProgram{};
        std::string m_statusMessage = "BgfxWorldRenderer not used yet";
    };
}
```

## `src/engine/render/BgfxWorldRenderer.cpp`

```cpp
#include "engine/render/BgfxWorldRenderer.h"

#include <cstring>
#include <vector>

#include "engine/render/BgfxViewTransform.h"

#if WAR_HAS_BGFX
namespace
{
    struct PosColorVertex
    {
        float x;
        float y;
        float z;
        unsigned int abgr;

        static bgfx::VertexLayout layout;

        static void init()
        {
            layout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
        }
    };

    bgfx::VertexLayout PosColorVertex::layout{};
    bool s_layoutInitialized = false;

    void ensureLayoutInitialized()
    {
        if (!s_layoutInitialized)
        {
            PosColorVertex::init();
            s_layoutInitialized = true;
        }
    }
}
#endif

namespace war
{
    bool BgfxWorldRenderer::render(
        const WorldState& worldState,
        const Camera2D& camera,
        const Vec2& playerPosition,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile)
    {
#if WAR_HAS_BGFX
        ensureLayoutInitialized();

        if (!m_colorProgram.loadColorProgram(m_statusMessage))
        {
            return false;
        }

        const bgfx::Stats* stats = bgfx::getStats();
        if (stats == nullptr)
        {
            m_statusMessage = "bgfx stats unavailable";
            return false;
        }

        const int viewWidth = static_cast<int>(stats->width);
        const int viewHeight = static_cast<int>(stats->height);

        if (viewWidth <= 0 || viewHeight <= 0)
        {
            m_statusMessage = "invalid bgfx view size";
            return false;
        }

        float view[16]{};
        float proj[16]{};
        BgfxViewTransform::buildMatrices(camera, viewWidth, viewHeight, view, proj);

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(viewWidth), static_cast<uint16_t>(viewHeight));
        bgfx::setViewTransform(0, view, proj);

        const BgfxWorldRenderData renderData = BgfxRenderDataBuilder::build(
            worldState,
            camera,
            playerPosition,
            currentPath,
            pathIndex,
            hasHoveredTile,
            hoveredTile);

        submitLayer(renderData.tiles);
        submitLayer(renderData.path);
        submitLayer(renderData.hoveredTile);
        submitLayer(renderData.entities);
        submitLayer(renderData.player);

        m_statusMessage = "bgfx world rendered";
        return true;
#else
        (void)worldState;
        (void)camera;
        (void)playerPosition;
        (void)currentPath;
        (void)pathIndex;
        (void)hasHoveredTile;
        (void)hoveredTile;
        m_statusMessage = "bgfx headers not available at compile time";
        return false;
#endif
    }

    void BgfxWorldRenderer::shutdown()
    {
        m_colorProgram.shutdown();
        m_statusMessage = "bgfx world renderer shutdown";
    }

    const std::string& BgfxWorldRenderer::statusMessage() const
    {
        return m_statusMessage;
    }

    bool BgfxWorldRenderer::submitLayer(const BgfxRenderLayer& layer) const
    {
#if WAR_HAS_BGFX
        if (layer.quads.empty() || !m_colorProgram.isReady())
        {
            return false;
        }

        std::vector<PosColorVertex> vertices;
        std::vector<unsigned short> indices;

        vertices.reserve(layer.quads.size() * 4);
        indices.reserve(layer.quads.size() * 6);

        for (const BgfxQuad& quad : layer.quads)
        {
            const unsigned short base = static_cast<unsigned short>(vertices.size());

            vertices.push_back({ quad.left,  quad.top,    0.0f, quad.color });
            vertices.push_back({ quad.right, quad.top,    0.0f, quad.color });
            vertices.push_back({ quad.right, quad.bottom, 0.0f, quad.color });
            vertices.push_back({ quad.left,  quad.bottom, 0.0f, quad.color });

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        bgfx::TransientVertexBuffer tvb{};
        bgfx::TransientIndexBuffer tib{};

        if (!bgfx::allocTransientBuffers(
            &tvb,
            PosColorVertex::layout,
            static_cast<uint32_t>(vertices.size()),
            &tib,
            static_cast<uint32_t>(indices.size())))
        {
            return false;
        }

        std::memcpy(tvb.data, vertices.data(), vertices.size() * sizeof(PosColorVertex));
        std::memcpy(tib.data, indices.data(), indices.size() * sizeof(unsigned short));

        bgfx::setVertexBuffer(0, &tvb);
        bgfx::setIndexBuffer(&tib);
        bgfx::setState(
            BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA);
        bgfx::submit(0, m_colorProgram.handle());
        return true;
#else
        (void)layer;
        return false;
#endif
    }
}
```

## `src/game/GameLayer.cpp`

```cpp
#include "game/GameLayer.h"

#include <memory>
#include <windows.h>

#include "engine/gameplay/Action.h"
#include "engine/gameplay/ActionSystem.h"
#include "engine/render/BgfxRenderDevice.h"
#include "engine/render/GdiRenderDevice.h"

namespace war
{
    void GameLayer::initialize(IWindow& window)
    {
        m_window = &window;
        m_camera.setViewportSize(window.getWidth(), window.getHeight());
        m_camera.setPosition({ 0.0f, 0.0f });

        m_worldState.initializeTestWorld();

        const TileCoord spawnTile{ 2, 2 };
        m_playerPosition = m_worldState.world().tileToWorldCenter(spawnTile);

        {
            auto preferred = std::make_unique<BgfxRenderDevice>();
            if (preferred->initialize(m_window->getHandle()))
            {
                m_renderDevice = std::move(preferred);
                pushEvent("Milestone 19 initialized");
                pushEvent("bgfx shader/asset pipeline cleanup active");
                pushEvent(std::string("Active backend: ") + m_renderDevice->name());
            }
            else
            {
                auto fallback = std::make_unique<GdiRenderDevice>();
                const bool fallbackReady = fallback->initialize(m_window->getHandle());
                m_renderDevice = std::move(fallback);

                pushEvent("Milestone 19 initialized");
                pushEvent("bgfx unavailable, falling back to GDI");
                pushEvent(std::string("Active backend: ") + m_renderDevice->name());
                if (!fallbackReady)
                {
                    pushEvent("Warning: fallback backend failed to initialize");
                }
            }
        }
    }

    void GameLayer::update(float dt)
    {
        m_lastDeltaTime = dt;
        m_camera.setViewportSize(m_window->getWidth(), m_window->getHeight());

        updateInput();
        ActionSystem::processPending(
            m_worldState,
            m_actions,
            m_playerPosition,
            m_currentPath,
            m_pathIndex,
            m_eventLog);
        updatePlayer(dt);
    }

    void GameLayer::render()
    {
        if (!m_renderDevice)
        {
            return;
        }

        const RECT clientRect = getClientRect();
        if (!m_renderDevice->beginFrame(m_window->getHandle(), clientRect))
        {
            return;
        }

        HDC dc = m_renderDevice->getDrawContext();
        if (dc != nullptr)
        {
            m_worldRenderer.render(
                dc,
                clientRect,
                m_worldState,
                m_camera,
                m_playerPosition,
                m_currentPath,
                m_pathIndex,
                m_hasHoveredTile,
                m_hoveredTile);

            m_debugOverlayRenderer.render(
                dc,
                m_worldState,
                m_camera,
                m_playerPosition,
                m_currentPath,
                m_pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_eventLog,
                m_lastDeltaTime,
                m_window->getMousePosition());
        }
        else
        {
            m_bgfxWorldRenderer.render(
                m_worldState,
                m_camera,
                m_playerPosition,
                m_currentPath,
                m_pathIndex,
                m_hasHoveredTile,
                m_hoveredTile);

            m_bgfxDebugFrameRenderer.render(
                m_worldState,
                m_playerPosition,
                m_eventLog,
                m_lastDeltaTime,
                m_bgfxWorldRenderer.statusMessage());
        }

        m_renderDevice->endFrame(m_window->getHandle());
    }

    void GameLayer::shutdown()
    {
        m_bgfxWorldRenderer.shutdown();

        if (m_renderDevice)
        {
            m_renderDevice->shutdown();
        }
    }

    void GameLayer::updateInput()
    {
        const POINT mouse = m_window->getMousePosition();
        const Vec2 mouseWorld = m_camera.screenToWorld(mouse.x, mouse.y);
        const TileCoord hovered = m_worldState.world().worldToTile(mouseWorld);

        m_hasHoveredTile = m_worldState.world().isInBounds(hovered);
        m_hoveredTile = hovered;

        POINT click{};
        if (m_window->consumeLeftClick(click))
        {
            const Vec2 world = m_camera.screenToWorld(click.x, click.y);
            const TileCoord targetTile = m_worldState.world().worldToTile(world);
            m_actions.push({ ActionType::Move, targetTile });
        }

        POINT rightClick{};
        if (m_window->consumeRightClick(rightClick))
        {
            const Vec2 world = m_camera.screenToWorld(rightClick.x, rightClick.y);
            const TileCoord targetTile = m_worldState.world().worldToTile(world);

            const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            if (shiftDown)
            {
                m_actions.push({ ActionType::Inspect, targetTile });
            }
            else
            {
                m_actions.push({ ActionType::Interact, targetTile });
            }
        }

        const int wheel = m_window->consumeMouseWheelDelta();
        if (wheel != 0)
        {
            m_camera.zoomBy(wheel > 0 ? 0.10f : -0.10f);
        }

        if (m_window->isMiddleMouseDown())
        {
            const POINT delta = m_window->consumeMouseDelta();
            const float zoom = m_camera.getZoom();
            m_camera.pan({
                -static_cast<float>(delta.x) / zoom,
                -static_cast<float>(delta.y) / zoom
            });
        }
        else
        {
            (void)m_window->consumeMouseDelta();
        }
    }

    void GameLayer::updatePlayer(float dt)
    {
        if (m_currentPath.empty() || m_pathIndex >= m_currentPath.size())
        {
            return;
        }

        const Vec2 waypoint = m_worldState.world().tileToWorldCenter(m_currentPath[m_pathIndex]);
        const Vec2 toTarget = waypoint - m_playerPosition;
        const float distance = length(toTarget);

        if (distance < 1.0f)
        {
            m_playerPosition = waypoint;
            ++m_pathIndex;

            if (m_pathIndex >= m_currentPath.size())
            {
                pushEvent("Path complete");
            }
            return;
        }

        const Vec2 direction = normalize(toTarget);
        const float step = m_playerSpeed * dt;

        if (step >= distance)
        {
            m_playerPosition = waypoint;
            ++m_pathIndex;

            if (m_pathIndex >= m_currentPath.size())
            {
                pushEvent("Path complete");
            }
            return;
        }

        m_playerPosition += direction * step;
    }

    void GameLayer::pushEvent(const std::string& message)
    {
        m_eventLog.push_back(message);
        constexpr size_t kMaxEvents = 10;
        if (m_eventLog.size() > kMaxEvents)
        {
            m_eventLog.erase(
                m_eventLog.begin(),
                m_eventLog.begin() + static_cast<std::ptrdiff_t>(m_eventLog.size() - kMaxEvents));
        }
    }

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }
}
```
