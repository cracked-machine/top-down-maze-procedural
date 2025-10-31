set(TARGET ProceduralMaze)

add_executable(${TARGET} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine.cpp
    ${CMAKE_SOURCE_DIR}/src/EventHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Debug/AssertHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Sprites/BaseFragmentShader.cpp
    ${CMAKE_SOURCE_DIR}/src/Sprites/MultiSprite.cpp
    ${CMAKE_SOURCE_DIR}/src/Sprites/SpriteFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Sprites/TileMap.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/ProcGen/RandomLevelGenerator.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/ProcGen/CellAutomataSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Render/RenderSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Render/RenderGameSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Render/RenderMenuSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Render/RenderOverlaySystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/AnimSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/BaseSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/DiggingSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/ExitSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/FootstepSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/MusicSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/PathFindSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/PersistentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/PlayerSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/BombSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/FloodSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/NpcSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/HazardFieldSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/WormholeSystem.cpp
)

target_link_libraries(${TARGET} PRIVATE 
    SFML::Graphics 
    SFML::Audio
    spdlog::spdlog
    EnTT
    $<$<PLATFORM_ID:Windows>:dbghelp>
    ImGui-SFML::ImGui-SFML
    nlohmann_json::nlohmann_json
)
target_include_directories(${TARGET} SYSTEM PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Logging
    ${CMAKE_SOURCE_DIR}/src/Debug
    ${CMAKE_SOURCE_DIR}/src/Logging

    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/inc/Systems
    ${CMAKE_SOURCE_DIR}/inc/Components
    ${CMAKE_SOURCE_DIR}/inc/Sprites
    ${CMAKE_SOURCE_DIR}/inc/Logging
    ${FETCHCONTENT_BASE_DIR}/imgui-src
    $<$<STREQUAL:${TARGET_TRIPLET},x86_64-w64-mingw32>:/usr/x86_64-w64-mingw32/include/GL/>
)

target_compile_options(${TARGET} PRIVATE 
    $<$<COMPILE_LANGUAGE:CXX>:
        -std=c++23
        -Werror                         # treat warnings as errors
        -Wpedantic                      # enable all warnings demanded by ISO C++ standard
        -Wall                           # enable all warnings
        -Wextra                         # enable extra warnings
        -ftrivial-auto-var-init=zero    # initialize all automatic variables to zero
        # mingw limits object data sizes so enable large object files (fixes 'file too big' error)
        $<$<STREQUAL:${TARGET_TRIPLET},x86_64-w64-mingw32>:-Wa,-mbig-obj>
    >
)

# Already be disabled with release
# May add some performance benefit with debug but has obvious disadvantages
# target_compile_definitions(${TARGET} PUBLIC ENTT_DISABLE_ASSERT)

