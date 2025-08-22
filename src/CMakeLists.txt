set(TARGET ProceduralMaze CACHE STRING "ProceduralMaze")
add_executable(${TARGET} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Base.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Random.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/BaseSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Debug/AssertHandler.cpp
)

target_link_libraries(${TARGET} PRIVATE 
    SFML::Graphics 
    spdlog::spdlog
    EnTT
    $<$<PLATFORM_ID:Windows>:dbghelp>
)
target_include_directories(${TARGET} SYSTEM PRIVATE 
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/inc/Systems
    ${CMAKE_SOURCE_DIR}/inc/Components
    ${CMAKE_SOURCE_DIR}/inc/Sprites
    ${CMAKE_SOURCE_DIR}/inc/Logging
    /usr/x86_64-w64-mingw32/include/GL/
)

target_compile_options(${TARGET} PRIVATE 
    $<$<COMPILE_LANGUAGE:CXX>:
        -std=c++23
        -Werror                         # treat warnings as errors
        -Wpedantic                      # enable all warnings demanded by ISO C++ standard
        -Wall                           # enable all warnings
        -Wextra                         # enable extra warnings
        -ftrivial-auto-var-init=zero    # initialize all automatic variables to zero
    >
)

# target_link_options(${TARGET} PRIVATE 
#     $<$<COMPILE_LANGUAGE:CXX>:
#         -lstdc++_libbacktrace
#     >)

# target_compile_definitions(${TARGET} PUBLIC ENTT_DISABLE_ASSERT)

