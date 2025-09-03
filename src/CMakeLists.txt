set(TARGET ProceduralMaze)
add_executable(${TARGET} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Base.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Random.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/PlayableCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/Sprites/SpriteFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/BaseSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Debug/AssertHandler.cpp
)
target_compile_features(${TARGET} PRIVATE cxx_std_20
)
target_link_libraries(${TARGET} PRIVATE 
    SFML::Graphics 
    SFML::Audio
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

# Already be disabled with release
# May add some performance benefit with debug but has obvious disadvantages
# target_compile_definitions(${TARGET} PUBLIC ENTT_DISABLE_ASSERT)

