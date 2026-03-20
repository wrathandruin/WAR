cmake_minimum_required(VERSION 3.21)

project(WAR
    VERSION 0.9.0
    DESCRIPTION "Wrath and Ruin milestone 36"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WAR_SOURCES
    src/main.cpp
    src/engine/core/Application.cpp
    shared/src/core/Timer.cpp
    shared/src/core/Log.cpp
    src/engine/core/LocalDemoDiagnostics.cpp
    shared/src/runtime/RuntimePaths.cpp
    shared/src/runtime/host/AuthoritativeHostProtocol.cpp
    src/engine/host/HeadlessHostBootstrap.cpp
    src/engine/host/HeadlessHostMode.cpp
    shared/src/runtime/host/HeadlessHostPresence.cpp
    shared/src/runtime/host/ReplicationHarness.cpp
    shared/src/math/Vec2.cpp
    src/engine/render/Camera2D.cpp
    src/engine/render/WorldRenderer.cpp
    src/engine/render/BgfxWorldRenderer.cpp
    src/engine/render/BgfxRenderData.cpp
    src/engine/render/BgfxViewTransform.cpp
    src/engine/render/BgfxShaderProgram.cpp
    src/engine/render/BgfxTextureAsset.cpp
    src/engine/render/BgfxSpriteMaterial.cpp
    src/engine/render/BgfxWorldTheme.cpp
    src/engine/render/BgfxTileVisuals.cpp
    src/engine/render/RenderAssetPaths.cpp
    src/engine/render/DebugOverlayRenderer.cpp
    src/engine/render/GdiRenderDevice.cpp
    src/engine/render/BgfxRenderDevice.cpp
    src/engine/render/BgfxDebugFrameRenderer.cpp
    shared/src/world/WorldGrid.cpp
    shared/src/world/WorldRegionTag.cpp
    shared/src/world/WorldSemanticDressing.cpp
    shared/src/world/Pathfinding.cpp
    shared/src/world/WorldState.cpp
    shared/src/gameplay/EntityManager.cpp
    shared/src/gameplay/ActionSystem.cpp
    shared/src/simulation/SimulationRuntime.cpp
    src/platform/win32/Win32Window.cpp
    src/game/GameLayer.cpp
)

add_executable(WARLegacy WIN32
    ${WAR_SOURCES}
)

target_include_directories(WARLegacy PRIVATE
    src
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
set(WAR_SHADER_BUILD_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/build_shaders_win64.bat")

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
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/vs_color.bin"
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/fs_color.bin"
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/vs_texture.bin"
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/fs_texture.bin"
        VERBATIM
    )

    add_dependencies(WARLegacy war_build_shaders)

    add_custom_command(TARGET WARLegacy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WARLegacy>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WARLegacy>/assets/textures"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11"
            "$<TARGET_FILE_DIR:WARLegacy>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/textures"
            "$<TARGET_FILE_DIR:WARLegacy>/assets/textures"
        VERBATIM
    )
endif()

set_target_properties(WARLegacy PROPERTIES
    OUTPUT_NAME "WAR"
)

option(WAR_ENABLE_SPLIT_TARGETS "Build transitional desktop/server/shared split targets." ON)

if(WAR_ENABLE_SPLIT_TARGETS)
    add_subdirectory(shared)
    add_subdirectory(desktop)
    add_subdirectory(server)
endif()
