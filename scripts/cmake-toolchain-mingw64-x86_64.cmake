# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_compile_options(
    -ftrivial-auto-var-init=zero    # initialize all automatic variables to zero
    -Werror                         # treat warnings as errors
    # -Wpedantic                      # enable all warnings demanded by ISO C++ standard
    # -Wall                           # enable all warnings
    # -Wextra                         # enable extra warnings
    # -Wno-dangling-pointer           # disable -Wdangling-pointer (bug in freetype library)

)
