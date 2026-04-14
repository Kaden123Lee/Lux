# Lux Aim Trainer - Technical Documentation

This document provides a deep dive into the architecture of Lux and serves as a manual for developers looking to extend its functionality.

---

## 📂 Project Architecture

### 1. Core Module (`Core/`)
*   **`Config.h`**: The source of truth for all constants (room dimensions, FOV) and global declarations (`extern`). It defines the enums for `GameState` and `GameMode`.
*   **`Globals.cpp`**: Definitions for global variables used across the application (camera position, sensitivity, current state).
*   **`MathUtils.h`**: (If present) Utility functions for vector math and projections.

### 2. Renderer Module (`Renderer/`)
*   **`Renderer.cpp`**: Uses Windows GDI for high-performance 2D drawing representing a 3D space.
    *   **Double Buffering**: Uses `hBackBufferDC` to prevent flickering.
    *   **3D Projection**: Converts 3D coordinates $(x, y, z)$ into 2D screen coordinates using a perspective divide: `s = 1.0f / (depth * 0.1f + 1.0f)`.
    *   **Coordinate System**: The center of the screen is $(0,0)$. $X$ is horizontal, $Y$ is vertical (ground is positive $Y$ in the logic, but projection handles verticality), and $Z$ is depth.

### 3. Gameplay Module (`Gameplay/`)
*   **`TargetSystem.cpp`**: Manages the `std::vector<Target> targets`. 
    *   **`UpdateTargets(float delta)`**: Moves targets (for tracking modes) and handles health/lifespan.
    *   **`HandleClick(int x, int y, float delta)`**: Projects the crosshair into the world and checks for collisions with targets.

### 4. Modes Module (`Modes/`)
*   Each file implements the specific spawning logic for a mode.
*   **Spawning Logic**: Typically involves creating a `Target` struct, setting its position, size, and health, and pushing it to the global `targets` vector.

---

## 🛠 Adding a New Game Mode: "FlickMaster"

Follow this exact workflow to implement a new mode.

### Step 1: Register the Mode
In `Core/Config.h`, add `MODE_FLICKMASTER` to the `GameMode` enum.

### Step 2: Implementation File
Create `Modes/FlickMasterMode.cpp`:
```cpp
#include "../Gameplay/TargetSystem.h"
#include "../Core/Config.h"
#include <cstdlib>

void SpawnFlickMaster() {
    Target t;
    t.x = (rand() % 401 / 100.0f) - 2.0f; // Random X between -2.0 and 2.0
    t.y = (rand() % 201 / 100.0f) - 1.0f; // Random Y between -1.0 and 1.0
    t.z = 1.0f;                          // Constant depth
    t.radius = 0.5f;                     // Size of the sphere
    t.active = true;
    t.health = 100.0f;
    t.maxHealth = 100.0f;
    targets.push_back(t);
}
```

### Step 3: Header Declaration
In `Gameplay/TargetSystem.h`, add:
```cpp
void SpawnFlickMaster();
```

### Step 4: Dispatch Logic
In `Gameplay/TargetSystem.cpp`, update `SpawnTarget()`:
```cpp
void SpawnTarget() {
    switch (currentGameMode) {
        ...
        case MODE_FLICKMASTER: SpawnFlickMaster(); break;
    }
}
```

### Step 5: Input & UI
1.  **`main.cpp`**: Add a key binding (e.g., `if (GetAsyncKeyState('7'))`) in the `STATE_MENU` block to set `currentGameMode = MODE_FLICKMASTER`.
2.  **`Renderer.cpp`**: Add the label `"[7] FLICKMASTER"` to the menu rendering block.

---

## 🧬 The `Target` Struct Explained
Located in `Gameplay/TargetSystem.h`:
*   `float x, y, z`: World position.
*   `float vx, vy`: Velocity (used for moving targets).
*   `float health`: Current HP. For click-to-kill modes, set health small or handle in `HandleClick`.
*   `bool active`: If false, the target is ignored by the renderer and system.
*   `float lifetime`: Used to auto-destroy targets after a duration.

---

## 🎨 Rendering Custom UI Elements
To draw something new on screen, use the `RenderFrame` function in `Renderer.cpp`.

**Example: Drawing a custom rectangle**
```cpp
HBRUSH myBrush = CreateSolidBrush(RGB(255, 0, 0)); // Red
RECT myRect = { 10, 10, 100, 50 };
FillRect(hBackBufferDC, &myRect, myBrush);
DeleteObject(myBrush); // ALWAYS delete GDI objects to prevent memory leaks
```

---

## 🚀 Advanced Customization

### Adjusting FOV or Sensitivity
- **FOV**: Change `fovDeg` in `Config.h`. Note that Lux uses a simplified projection; actual FOV scaling requires updates to the projection math in `Renderer.cpp`.
- **Sensitivity**: Adjusted via `mouseSensitivity` in `Settings.cpp` (logic) and `main.cpp` (application).

### Adding Particles/Visual Effects
The renderer already has a `starfield` implementation. You can add more particle systems in `Renderer.cpp` by creating a struct, a global vector, and updating/drawing them in `RenderFrame`.

---

## 📦 Build Instructions
The project uses `w64devkit` (GCC/G++).
1.  Open a terminal in the project root.
2.  Run `.\build.bat`.
3.  The script will:
    *   Set the path to the local compiler.
    *   Compile all source files including those in subdirectories.
    *   Link GDI32 and User32 libraries.
    *   Launch `Lux.exe` on success.
