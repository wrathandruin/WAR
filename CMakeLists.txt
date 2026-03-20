cmake_minimum_required(VERSION 3.21)

project(WAR
    VERSION 0.3.6
    DESCRIPTION "Wrath and Ruin milestone 23"
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
    src/engine/render/BgfxTextureAsset.cpp
    src/engine/render/BgfxSpriteMaterial.cpp
    src/engine/render/BgfxTileVisuals.cpp
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
        COMMAND cmd /c "\"${WAR_SHADER_BUILD_SCRIPT}\" \"${CMAKE_SOURCE_DIR}\" \"$<CONFIG>\""
        BYPRODUCTS
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/vs_color.bin"
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/fs_color.bin"
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/vs_texture.bin"
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11/fs_texture.bin"
        VERBATIM
    )

    add_dependencies(WAR war_build_shaders)

    add_custom_command(TARGET WAR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WAR>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WAR>/assets/textures"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/shaders/dx11"
            "$<TARGET_FILE_DIR:WAR>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/textures"
            "$<TARGET_FILE_DIR:WAR>/assets/textures"
        VERBATIM
    )
endif()

set_target_properties(WAR PROPERTIES
    OUTPUT_NAME "WAR"
)
