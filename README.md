# GTA7 v0.5 - WebAssembly Version

This is a WebAssembly (WASM) rewrite of the Three.js-based 3D game.

## Features

- **WebAssembly Core**: Game logic runs in C++ compiled to WASM for better performance
- **Three.js Rendering**: Uses Three.js for 3D graphics and rendering
- **Player Movement**: WASD controls with camera-relative movement
- **Vehicle System**: Press 'E' to enter/exit car
- **Shooting System**: Click to shoot bullets
- **Enemy AI**: Enemies spawn randomly and can be defeated
- **Camera System**: Mouse controls for camera rotation and zoom

## Files

- `game.cpp` - Main C++ game logic
- `game.html` - HTML file with Three.js integration
- `build.sh` - Build script for compiling to WASM
- `game.js` - Generated JavaScript glue code (after build)
- `game.wasm` - Generated WebAssembly binary (after build)

## Building

### Prerequisites

You need to install Emscripten SDK first:

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Build Instructions

1. Make sure Emscripten is activated in your shell:
   ```bash
   source /path/to/emsdk/emsdk_env.sh
   ```

2. Run the build script:
   ```bash
   ./build.sh
   ```

3. Start a local web server:
   ```bash
   python -m http.server 8000
   ```

4. Open your browser and navigate to:
   ```
   http://localhost:8000/game.html
   ```

## Controls

- **WASD** - Move player/car
- **Mouse** - Look around (hold left button and drag)
- **Mouse Wheel** - Zoom camera in/out
- **E** - Enter/Exit car
- **Left Click** - Shoot (when on foot)

## Architecture

The game uses a hybrid approach:
- **C++ (WASM)**: Game logic, physics, collision detection, entity management
- **JavaScript**: Input handling, Three.js rendering, DOM manipulation
- **Three.js**: 3D graphics, scene management, rendering pipeline

## Performance Benefits

- Faster game logic execution in C++
- Efficient memory management
- Better performance for complex calculations
- Optimized collision detection and physics

## Browser Compatibility

Requires a modern browser with WebAssembly support:
- Chrome 57+
- Firefox 52+
- Safari 11+
- Edge 16+
