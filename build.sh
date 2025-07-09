#!/bin/bash

# Build script for compiling C++ to WebAssembly using Emscripten

echo "Building WASM game..."

# Check if emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten compiler (emcc) not found!"
    echo "Please install Emscripten SDK first:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo "  source ./emsdk_env.sh"
    exit 1
fi

# Compile C++ to WebAssembly
emcc game.cpp \
    -o game.js \
    -s WASM=1 \
    -s EXPORTED_RUNTIME_METHODS='["cwrap", "ccall"]' \
    -s EXPORTED_FUNCTIONS='["_malloc", "_free", "_initGame", "_setKey", "_updateCamera", "_shoot", "_updateGame", "_getPlayerX", "_getPlayerY", "_getPlayerZ", "_getCarX", "_getCarY", "_getCarZ", "_getInCar", "_getScore", "_getCamAngleH", "_getCamAngleV", "_getCamDist", "_getEnemyCount", "_getEnemyX", "_getEnemyY", "_getEnemyZ", "_getBulletCount", "_getBulletX", "_getBulletY", "_getBulletZ"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=16777216 \
    -s MODULARIZE=0 \
    -s ENVIRONMENT=web \
    -s FORCE_FILESYSTEM=0 \
    -s TEXTDECODER=0 \
    -s ABORTING_MALLOC=0 \
    -s ALLOW_UNIMPLEMENTED_SYSCALLS=0 \
    -s ASSERTIONS=0 \
    -O2 \
    -std=c++17

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Generated files:"
    echo "  - game.js (JavaScript glue code)"
    echo "  - game.wasm (WebAssembly binary)"
    echo ""
    echo "To run the game:"
    echo "  1. Start a local web server (e.g., python -m http.server 8000)"
    echo "  2. Open http://localhost:8000/game.html in your browser"
else
    echo "Build failed!"
    exit 1
fi
