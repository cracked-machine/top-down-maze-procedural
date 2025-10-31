cmake_minimum_required(VERSION 3.28)
project(ProceduralMazeProject LANGUAGES CXX)

message(STATUS "Building for architecture: ${TARGET_TRIPLET}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
# message(STATUS "Loading toolchain file: ${CMAKE_SOURCE_DIR}/scripts/cmake-${TARGET_TRIPLET}.cmake")
# include(${CMAKE_SOURCE_DIR}/scripts/cmake-${TARGET_TRIPLET}.cmake)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found - compilation caching disabled")
endif()

include(FetchContent)

set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "Use system dependencies" FORCE)
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

FetchContent_Declare(ImGUI
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1
    GIT_SHALLOW ON EXCLUDE_FROM_ALL SYSTEM
)

FetchContent_Declare(imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG        v3.0
)

FetchContent_Declare(json-nlohmann
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
  GIT_SHALLOW ON EXCLUDE_FROM_ALL SYSTEM
)

set(IMGUI_DIR ${FETCHCONTENT_BASE_DIR}/imgui-src)
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)
FetchContent_MakeAvailable(
    SFML 
    SPDLog
    EnTT
    ImGUI
    imgui-sfml
    json-nlohmann
)

# ThirdParty Lib warning Supression:
# 1. spdlog-src/include/spdlog/logger.h:317:10: error: 
# variable 'args' might be clobbered by 'longjmp' or 'vfork' 
target_compile_options(spdlog INTERFACE -Wno-clobbered)
# 2. flac-src/src/share/win_utf8_io/win_utf8_io.c:132:29: warning: 
# "cast between incompatible function types from 'FARPROC' to int" 
target_compile_options(FLAC PRIVATE -Wno-cast-function-type)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory( ${CMAKE_SOURCE_DIR}/src )



# Copy resources to binary dir
file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR}/bin )