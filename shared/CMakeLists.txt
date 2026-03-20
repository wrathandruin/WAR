set(WAR_SHARED_SOURCES
    ${CMAKE_SOURCE_DIR}/shared/src/core/Timer.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/core/Log.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/runtime/RuntimePaths.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/runtime/host/AuthoritativeHostProtocol.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/runtime/host/HeadlessHostPresence.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/runtime/host/ReplicationHarness.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/math/Vec2.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/world/WorldGrid.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/world/WorldRegionTag.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/world/WorldSemanticDressing.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/world/Pathfinding.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/world/WorldState.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/gameplay/EntityManager.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/gameplay/ActionSystem.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/simulation/SimulationRuntime.cpp
)

add_library(WARShared STATIC
    ${WAR_SHARED_SOURCES}
)

target_include_directories(WARShared
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/shared/include
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
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/split/shared/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/split/shared/$<CONFIG>"
)
