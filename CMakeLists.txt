cmake_minimum_required(VERSION 3.21)

project(WAR
    VERSION 0.1.0
    DESCRIPTION "Wrath and Ruin starter project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WAR_BUILD_TESTS "Build WAR tests" ON)

add_executable(WAR
    src/main.cpp
    src/engine/core/Application.cpp
    src/engine/core/Log.cpp
    src/game/Game.cpp
)

target_include_directories(WAR PRIVATE
    src
)

if(MSVC)
    target_compile_options(WAR PRIVATE /W4 /permissive-)
else()
    target_compile_options(WAR PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(WAR PROPERTIES
    OUTPUT_NAME "WAR"
)

if(WAR_BUILD_TESTS)
    enable_testing()
endif()
