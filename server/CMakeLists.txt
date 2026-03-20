set(WAR_SERVER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/host/HeadlessHostBootstrap.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/core/LocalDemoDiagnostics.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/host/HeadlessHostMode.cpp
)

add_executable(WARServer
    ${WAR_SERVER_SOURCES}
)

target_include_directories(WARServer
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/shared/include
)

target_link_libraries(WARServer
    PRIVATE
        WARShared
)

if(WIN32)
    target_compile_definitions(WARServer PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(MSVC)
    target_compile_options(WARServer PRIVATE /W4 /permissive-)
else()
    target_compile_options(WARServer PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(WARServer PROPERTIES
    OUTPUT_NAME "WARServer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/split/server/$<CONFIG>"
)
