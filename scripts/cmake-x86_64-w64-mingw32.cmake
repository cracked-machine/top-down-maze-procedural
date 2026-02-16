message(STATUS "TARGET_TRIPLET in toolchain: '${TARGET_TRIPLET}'")
message(STATUS "ARCH in toolchain: '${ARCH}'")

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   ${TARGET_TRIPLET}-gcc)
set(CMAKE_CXX_COMPILER ${TARGET_TRIPLET}-g++)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Add Windows system libraries path
set(CMAKE_SYSTEM_LIBRARY_PATH /usr/${TARGET_TRIPLET}/lib)
set(CMAKE_SYSTEM_INCLUDE_PATH /usr/${TARGET_TRIPLET}/include)

# Ensure dbghelp is available
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldbghelp -fno-lto -Wl,-Map=${CMAKE_BINARY_DIR}/ProceduralMaze.map")

# Use install() to copy MinGW runtime DLLs
file(COPY 
    "/usr/lib/gcc/x86_64-w64-mingw32/14-win32/libstdc++-6.dll"
    "/usr/lib/gcc/x86_64-w64-mingw32/14-win32/libgcc_s_seh-1.dll"
    DESTINATION ${CMAKE_BINARY_DIR}/bin
)