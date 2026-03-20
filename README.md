# WAR Starter Project

Starter structure for **Wrath and Ruin (WAR)** with:
- **Visual Studio solution** (`WAR.sln`)
- **VS Code workspace** (`WAR.code-workspace`)
- **CMake build** for editor/tooling compatibility
- Clean separation between `engine`, `game`, `platform`, and `third_party`

## Intended use

This starter is deliberately lean. It gives you:
- a compilable C++20 executable target
- room for an engine/game split
- a place for assets, tools, docs, and tests
- both VS and VS Code entry points

## Open in Visual Studio

Open `WAR.sln`.

## Open in VS Code

Open `WAR.code-workspace`.

Recommended extensions:
- C/C++
- CMake Tools

## Build with CMake

```bash
cmake -S . -B build
cmake --build build
```

## Current executable

The current executable is a minimal bootstrap that prints startup diagnostics and initializes the WAR app shell.

## Suggested next steps

1. Add platform/window bootstrap under `src/platform/`
2. Add renderer bootstrap under `src/engine/render/`
3. Add game loop and fixed timestep under `src/engine/core/`
4. Add world/terrain modules under `src/engine/world/`
5. Add input + point-and-click movement under `src/game/`
