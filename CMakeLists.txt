cmake_minimum_required(VERSION 3.21)

project(WAR
    VERSION 0.9.0
    DESCRIPTION "Wrath and Ruin milestone 44"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WAR_SOURCES
    WarEngine/src/main.cpp
    WarEngine/src/engine/core/Application.cpp
    WarEngine/src/core/Timer.cpp
    WarEngine/src/core/Log.cpp
    WarEngine/src/engine/core/LocalDemoDiagnostics.cpp
    WarEngine/src/runtime/RuntimePaths.cpp
    WarEngine/src/runtime/host/AuthoritativeHostProtocol.cpp
    WarEngine/src/engine/host/HeadlessHostBootstrap.cpp
    WarEngine/src/engine/host/HeadlessHostMode.cpp
    WarEngine/src/runtime/host/HeadlessHostPresence.cpp
    WarEngine/src/runtime/host/ReplicationHarness.cpp
    WarEngine/src/math/Vec2.cpp
    WarEngine/src/engine/render/Camera2D.cpp
    WarEngine/src/engine/render/WorldRenderer.cpp
    WarEngine/src/engine/render/BgfxWorldRenderer.cpp
    WarEngine/src/engine/render/BgfxRenderData.cpp
    WarEngine/src/engine/render/BgfxViewTransform.cpp
    WarEngine/src/engine/render/BgfxShaderProgram.cpp
    WarEngine/src/engine/render/BgfxTextureAsset.cpp
    WarEngine/src/engine/render/BgfxSpriteMaterial.cpp
    WarEngine/src/engine/render/BgfxWorldTheme.cpp
    WarEngine/src/engine/render/BgfxTileVisuals.cpp
    WarEngine/src/engine/render/RenderAssetPaths.cpp
    WarEngine/src/engine/render/DebugOverlayRenderer.cpp
    WarEngine/src/engine/render/GdiRenderDevice.cpp
    WarEngine/src/engine/render/BgfxRenderDevice.cpp
    WarEngine/src/engine/render/BgfxDebugFrameRenderer.cpp
    WarEngine/src/world/WorldGrid.cpp
    WarEngine/src/world/WorldRegionTag.cpp
    WarEngine/src/world/WorldSemanticDressing.cpp
    WarEngine/src/world/Pathfinding.cpp
    WarEngine/src/world/WorldState.cpp
    WarEngine/src/gameplay/EntityManager.cpp
    WarEngine/src/gameplay/ActionSystem.cpp
    WarEngine/src/simulation/SimulationRuntime.cpp
    WarEngine/src/platform/win32/Win32Window.cpp
    WarEngine/src/game/GameLayer.cpp
)

add_executable(WARLegacy WIN32
    ${WAR_SOURCES}
)

target_include_directories(WARLegacy PRIVATE
    WarEngine/src
    WarEngine/include
)

if(WIN32)
    target_compile_definitions(WARLegacy PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

target_link_libraries(WARLegacy PRIVATE
    user32
    gdi32
)

if(MSVC)
    target_compile_options(WARLegacy PRIVATE /W4 /permissive-)
else()
    target_compile_options(WARLegacy PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(WAR_BGFX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/bgfx/include")
set(WAR_BX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/bx/include")
set(WAR_BGFX_DEBUG_LIB_DIR "${CMAKE_SOURCE_DIR}/third_party/bgfx/lib/win64/debug")
set(WAR_BGFX_RELEASE_LIB_DIR "${CMAKE_SOURCE_DIR}/third_party/bgfx/lib/win64/release")
set(WAR_SHADER_BUILD_SCRIPT "${CMAKE_SOURCE_DIR}/Project/scripts/build_shaders_win64.bat")

if(EXISTS "${WAR_BGFX_INCLUDE_DIR}" AND EXISTS "${WAR_BX_INCLUDE_DIR}")
    target_include_directories(WARLegacy PRIVATE
        "${WAR_BGFX_INCLUDE_DIR}"
        "${WAR_BX_INCLUDE_DIR}"
    )

    if(WIN32 AND EXISTS "${WAR_BGFX_DEBUG_LIB_DIR}" AND EXISTS "${WAR_BGFX_RELEASE_LIB_DIR}")
        target_link_directories(WARLegacy PRIVATE
            $<$<CONFIG:Debug>:${WAR_BGFX_DEBUG_LIB_DIR}>
            $<$<CONFIG:Release>:${WAR_BGFX_RELEASE_LIB_DIR}>
            $<$<CONFIG:RelWithDebInfo>:${WAR_BGFX_RELEASE_LIB_DIR}>
            $<$<CONFIG:MinSizeRel>:${WAR_BGFX_RELEASE_LIB_DIR}>
        )

        target_link_libraries(WARLegacy PRIVATE
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
        COMMAND cmd /c call "${WAR_SHADER_BUILD_SCRIPT}" "${CMAKE_SOURCE_DIR}" "$<CONFIG>"
        BYPRODUCTS
            "${CMAKE_SOURCE_DIR}/Assets/shaders/dx11/vs_color.bin"
            "${CMAKE_SOURCE_DIR}/Assets/shaders/dx11/fs_color.bin"
            "${CMAKE_SOURCE_DIR}/Assets/shaders/dx11/vs_texture.bin"
            "${CMAKE_SOURCE_DIR}/Assets/shaders/dx11/fs_texture.bin"
        VERBATIM
    )

    add_dependencies(WARLegacy war_build_shaders)

    add_custom_command(TARGET WARLegacy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WARLegacy>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WARLegacy>/assets/textures"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Assets/shaders/dx11"
            "$<TARGET_FILE_DIR:WARLegacy>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Assets/textures"
            "$<TARGET_FILE_DIR:WARLegacy>/assets/textures"
        VERBATIM
    )
endif()

set_target_properties(WARLegacy PROPERTIES
    OUTPUT_NAME "WAR"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin64/$<CONFIG>"
)

option(WAR_ENABLE_SPLIT_TARGETS "Build transitional desktop/server/shared split targets." ON)

if(WAR_ENABLE_SPLIT_TARGETS)
    add_subdirectory(WarEngine)
    add_subdirectory(Desktop)
    add_subdirectory(Server)
endif()
