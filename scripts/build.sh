#!/bin/bash
BUILD="$1"
ARCH="$2"
BUILD_DIR="build-$2"

cmake -S . -B $BUILD_DIR -G Ninja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DCMAKE_BUILD_TYPE=$BUILD \
    -DTARGET_TRIPLET=$ARCH \
    -DCMAKE_TOOLCHAIN_FILE=scripts/cmake-$ARCH.cmake \
    -DCMAKE_INSTALL_PREFIX=$BUILD_DIR/bin

cmake --build $BUILD_DIR --parallel

chmod 777 $BUILD_DIR/bin/*

# update .clangd settings
if [ -f ".clangd" ]; then
    yq -y --in-place ".CompileFlags.CompilationDatabase = \"$BUILD_DIR\"" .clangd
    echo "Switched .CompileFlags.CompilationDatabase to $BUILD_DIR"
    echo "You may need to restart clangd server if this is a new value"
fi

# Update VS Code settings - we moved these settings to .clangd but leaving this here for future reference
# if [ -f ".vscode/settings.json" ]; then
#     jq --arg builddir "$BUILD_DIR" \
#        '(.["clangd.arguments"] // []) |= map(if startswith("-compile-commands-dir=") then "-compile-commands-dir=" + $builddir else . end)' \
#        .vscode/settings.json > .vscode/settings.json.tmp && \
#        mv .vscode/settings.json.tmp .vscode/settings.json  
# fi

# restart clangd server, vscode will restart it automatically
# if you not using vscode you may need to restart it manually
# if pgrep -f "^/usr/bin/clangd" > /dev/null; then
#     echo "Stopping clangd..."
#     pkill -f "^/usr/bin/clangd"
#     sleep 1
# fi  