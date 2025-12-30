set(TARGET ProceduralMaze)

add_executable(${TARGET} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Engine.cpp
    ${CMAKE_SOURCE_DIR}/src/Audio/EffectsItem.cpp
    ${CMAKE_SOURCE_DIR}/src/Audio/MusicItem.cpp
    ${CMAKE_SOURCE_DIR}/src/Audio/SoundBank.cpp
    ${CMAKE_SOURCE_DIR}/src/Debug/AssertHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Shaders/BaseFragmentShader.cpp
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
    ${CMAKE_SOURCE_DIR}/src/Systems/DiggingSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/ExitSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/FootstepSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/PersistSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/PlayerSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/BombSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/NpcSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/HazardFieldSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/Threats/WormholeSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/GraveSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/AltarSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/LootSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/CryptSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/PassageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/SystemStore.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/SceneInputRouter.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/SceneManager.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/Scenes/GraveyardScene.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/Scenes/SettingsMenuScene.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/Scenes/PausedMenuScene.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/Scenes/GameOverScene.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/Scenes/TitleScene.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/Scenes/LevelCompleteScene.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/Scenes/CryptScene.cpp
    ${CMAKE_SOURCE_DIR}/src/SceneControl/RegistryTransfer.cpp
    ${CMAKE_SOURCE_DIR}/src/Factory/PlayerFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Factory/NpcFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Factory/LootFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Factory/ObstacleFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Factory/BombFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/Factory/CryptFactory.cpp
)

target_link_libraries(${TARGET} PRIVATE 
    SFML::Graphics 
    SFML::Audio
    spdlog::spdlog
    EnTT
    $<$<PLATFORM_ID:Windows>:dbghelp>
    ImGui-SFML::ImGui-SFML
    nlohmann_json::nlohmann_json
    stdc++exp
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

