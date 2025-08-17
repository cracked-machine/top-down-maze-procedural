set(TARGET ProceduralMaze)
add_executable(${TARGET} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Base.cpp
    ${CMAKE_SOURCE_DIR}/src/Components/Random.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems/BaseSystem.cpp
)
target_compile_features(${TARGET} PRIVATE cxx_std_20
)
target_link_libraries(${TARGET} PRIVATE 
    SFML::Graphics 
    spdlog::spdlog
    EnTT
)
target_include_directories(${TARGET} SYSTEM PRIVATE 
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/inc/Systems
    ${CMAKE_SOURCE_DIR}/inc/Components
    ${CMAKE_SOURCE_DIR}/inc/Sprites
    ${CMAKE_SOURCE_DIR}/inc/Logging
    /usr/x86_64-w64-mingw32/include/GL/
)
target_compile_definitions(${TARGET} PUBLIC ENTT_DISABLE_ASSERT)

