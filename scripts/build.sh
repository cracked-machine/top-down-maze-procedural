#!/bin/bash
$BUILD=$1
cmake -S . -B build -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=scripts/cmake-toolchain-mingw64-x86_64.cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DCMAKE_BUILD_TYPE=$BUILD

cmake --build build --parallel

chmod 777 build/bin/*