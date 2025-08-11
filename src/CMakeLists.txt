set(TARGET RENAME_THIS_CMAKE_TARGET)
add_executable(${TARGET} 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/components/base.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/base_system.cpp
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
    ${CMAKE_SOURCE_DIR}/inc/systems
    ${CMAKE_SOURCE_DIR}/inc/components
    ${CMAKE_SOURCE_DIR}/inc/logging
    /usr/x86_64-w64-mingw32/include/GL/
)

