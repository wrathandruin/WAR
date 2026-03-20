cmake_minimum_required(VERSION 3.21)

project(WAR
    VERSION 0.3.1
    DESCRIPTION "Wrath and Ruin milestone 2.1"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(WAR WIN32
    src/main.cpp
    src/engine/core/Application.cpp
    src/engine/core/Timer.cpp
    src/engine/core/Log.cpp
    src/engine/math/Vec2.cpp
    src/engine/render/Camera2D.cpp
    src/engine/world/WorldGrid.cpp
    src/engine/world/Pathfinding.cpp
    src/platform/win32/Win32Window.cpp
    src/game/GameLayer.cpp
)

target_include_directories(WAR PRIVATE
    src
)

target_link_libraries(WAR PRIVATE
    user32
    gdi32
)

if(MSVC)
    target_compile_options(WAR PRIVATE /W4 /permissive-)
else()
    target_compile_options(WAR PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(WAR PROPERTIES
    OUTPUT_NAME "WAR"
)
