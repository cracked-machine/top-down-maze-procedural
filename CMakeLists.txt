cmake_minimum_required(VERSION 3.28)
project(ProceduralMazeProject LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 016bea9491ccafc3529019fe1d403885a8b3a6ae    # v3.0.1
    GIT_SHALLOW ON EXCLUDE_FROM_ALL SYSTEM)     

FetchContent_Declare(SPDLog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG 6fa36017cfd5731d617e1a934f0e5ea9c4445b13    # v1.15.3
    GIT_SHALLOW ON EXCLUDE_FROM_ALL SYSTEM)

FetchContent_Declare(EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG d4014c74dc3793aba95ae354d6e23a026c2796db    # v3.15.0
    EXCLUDE_FROM_ALL SYSTEM
)

FetchContent_MakeAvailable(
    SFML 
    SPDLog
    EnTT
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# add_compile_definitions(SHOW_BRICK_ENTITY_ID)
add_compile_definitions(SHOW_PLAYER_HIT_BOXES)

add_subdirectory( ${CMAKE_SOURCE_DIR}/src )

# runtime deps for mingw
file(   COPY        /usr/lib/gcc/x86_64-w64-mingw32/14-win32/libstdc++-6.dll build/bin/libstdc++-6.dll
                    /usr/lib/gcc/x86_64-w64-mingw32/14-win32/libgcc_s_seh-1.dll build/bin/libgcc_s_seh-1.dll
        DESTINATION ${CMAKE_BINARY_DIR}/bin/ )

file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR}/bin )