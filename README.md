# WAR Milestone 1

Milestone 1 project for **Wrath and Ruin (WAR)**.

## Scope
This build targets the first visible technical milestone:

- Win32 window creation
- Real-time main loop
- Basic top-down camera
- Grid world rendering using GDI
- Click-to-move player actor
- Debug text overlay
- Visual Studio solution + VS Code workspace
- CMake support for IDE indexing and future expansion

## Notes
- This is a Windows-first bootstrap.
- Rendering currently uses **Win32 GDI** for simplicity and fast iteration.
- The 6-second combat rule is **not** part of this milestone.
- Movement is real-time point-and-click.

## Build
### Visual Studio
Open `WAR.sln`.

### VS Code
Open `WAR.code-workspace`.

### CMake
```bash
cmake -S . -B build
cmake --build build --config Debug
```

## Controls
- **Left click**: Move actor to clicked world position
- **Mouse wheel**: Zoom camera
- **Middle mouse drag**: Pan camera

## Recommended next step
After this milestone:
1. Move rendering from GDI to a real renderer path
2. Introduce terrain chunk data
3. Add obstacle-aware movement/pathfinding
4. Add proper input abstraction
