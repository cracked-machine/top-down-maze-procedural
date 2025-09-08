set(TARGET ProceduralMaze)
add_executable(${TARGET} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Base.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Random.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/PlayableCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/Sprites/SpriteFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/BaseSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/RenderSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/RenderGameSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/RenderMenuSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/RenderOverlaySystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Debug/AssertHandler.cpp
)

target_link_libraries(${TARGET} PRIVATE 
    SFML::Graphics 
    SFML::Audio
    spdlog::spdlog
    EnTT
    $<$<PLATFORM_ID:Windows>:dbghelp>
    ImGui-SFML::ImGui-SFML
)
target_include_directories(${TARGET} SYSTEM PRIVATE 
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/inc/Systems
    ${CMAKE_SOURCE_DIR}/inc/Components
    ${CMAKE_SOURCE_DIR}/inc/Sprites
    ${CMAKE_SOURCE_DIR}/inc/Logging
    ${FETCHCONTENT_BASE_DIR}/imgui-src
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
        -Wa,-mbig-obj                   # support large object files (fixes 'file too big' error)
    >
)

# Already be disabled with release
# May add some performance benefit with debug but has obvious disadvantages
# target_compile_definitions(${TARGET} PUBLIC ENTT_DISABLE_ASSERT)

