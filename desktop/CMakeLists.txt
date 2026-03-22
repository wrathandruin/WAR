set(WAR_DESKTOP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/core/Application.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/core/LocalDemoDiagnostics.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxWorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxRenderData.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxViewTransform.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxShaderProgram.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxTextureAsset.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxSpriteMaterial.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxWorldTheme.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxTileVisuals.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/RenderAssetPaths.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/DebugOverlayRenderer.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/GdiRenderDevice.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxRenderDevice.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/engine/render/BgfxDebugFrameRenderer.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/platform/win32/Win32Window.cpp
    ${CMAKE_SOURCE_DIR}/WarEngine/src/game/GameLayer.cpp
)

add_executable(WAR WIN32
    ${WAR_DESKTOP_SOURCES}
)

target_include_directories(WAR
    PRIVATE
        ${CMAKE_SOURCE_DIR}/WarEngine/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/WarEngine/include
)

target_link_libraries(WAR
    PRIVATE
        WARShared
        user32
        gdi32
)

if(WIN32)
    target_compile_definitions(WAR PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(MSVC)
    target_compile_options(WAR PRIVATE /W4 /permissive-)
else()
    target_compile_options(WAR PRIVATE -Wall -Wextra -Wpedantic)
endif()

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

if(TARGET war_build_shaders)
    add_dependencies(WAR war_build_shaders)
endif()

if(WIN32)
    add_custom_command(TARGET WAR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WAR>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WAR>/assets/textures"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Assets/shaders/dx11"
            "$<TARGET_FILE_DIR:WAR>/assets/shaders/dx11"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Assets/textures"
            "$<TARGET_FILE_DIR:WAR>/assets/textures"
        VERBATIM
    )
endif()

set_target_properties(WAR PROPERTIES
    OUTPUT_NAME "WAR"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin64/$<CONFIG>/split/desktop"
)
