# WAR — Milestone 6 (Architecture Pass 1)

Milestone 6 for **Wrath and Ruin (WAR)**.

## Focus of this milestone
This milestone introduces the first backend architecture pass, separating gameplay systems from the prototype layer.

- World state is centralized
- Entities are managed through a dedicated system
- Actions are processed through a gameplay system
- `GameLayer` is no longer a god object

## Core Systems

### WorldState
Owns simulation data:
- `WorldGrid`
- `EntityManager`

### EntityManager
Handles:
- entity storage
- tile-based lookup
- entity queries

### ActionSystem
Resolves:
- Move
- Inspect
- Interact

### GameLayer
Now responsible for:
- input collection
- system coordination
- rendering

---

## Scope

- Win32 window
- Real-time main loop
- Backbuffered GDI rendering
- Camera pan + zoom
- Tile-based world (`WorldGrid`)
- A* pathfinding
- Action queue system
- Entity system
- Contextual interaction
- Centralized world state
- Modular gameplay systems

---

## Controls

- **Left click**: Move
- **Right click**: Interact
- **Shift + Right click**: Inspect
- **Middle mouse drag**: Pan camera
- **Mouse wheel**: Zoom

---

## What changed from M5

- introduced `WorldState`
- introduced `EntityManager`
- introduced `ActionSystem`
- refactored `GameLayer` into coordinator
- removed direct gameplay logic from `GameLayer`
- improved separation between:
  - simulation
  - gameplay logic
  - rendering

---

## Current Status

WAR is now a **structured gameplay prototype** with:
- a working interaction loop
- modular backend systems
- a foundation ready for:
  - stateful objects
  - persistence
  - combat systems

---

## Next Milestone

### M7 — Render / Simulation Separation

- introduce `WorldRenderer`
- move rendering out of `GameLayer`
- separate debug/UI rendering
- clean up frame pipeline