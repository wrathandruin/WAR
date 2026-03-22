set(WAR_SHARED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/RuntimePaths.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/host/AuthoritativeHostProtocol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/host/HeadlessHostPresence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/host/ReplicationHarness.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math/Vec2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/WorldGrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/WorldRegionTag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/WorldSemanticDressing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/Pathfinding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/WorldState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gameplay/EntityManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gameplay/ActionSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simulation/SimulationRuntime.cpp
)

add_library(WARShared STATIC
    ${WAR_SHARED_SOURCES}
)

target_include_directories(WARShared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
    target_compile_definitions(WARShared PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(MSVC)
    target_compile_options(WARShared PRIVATE /W4 /permissive-)
else()
    target_compile_options(WARShared PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(WARShared PROPERTIES
    OUTPUT_NAME "WARShared"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin64/$<CONFIG>/split/WarEngine"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin64/$<CONFIG>/split/WarEngine"
)
