# WAR — Milestone 7 (Render / Simulation Separation)

> Current development milestone: M7 — Render / Simulation Separation

Milestone 7 for **Wrath and Ruin (WAR)**.

## Focus of this milestone
This milestone separates rendering from simulation and gameplay orchestration.

- `GameLayer` no longer owns world drawing logic
- world rendering now lives in `WorldRenderer`
- debug/UI rendering now lives in `DebugOverlayRenderer`
- M6 gameplay behavior is preserved:
  - Left click = move
  - Right click = interact
  - Shift + Right click = inspect

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

### WorldRenderer
Handles:
- tile drawing
- hovered tile highlight
- path rendering
- entity rendering
- player rendering

### DebugOverlayRenderer
Handles:
- debug text
- hovered information
- event log

### GameLayer
Now responsible for:
- input collection
- system coordination
- presentation orchestration

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
- Separated render systems

---

## Controls

- **Left click**: Move
- **Right click**: Interact
- **Shift + Right click**: Inspect
- **Middle mouse drag**: Pan camera
- **Mouse wheel**: Zoom

---

## What changed from M6

- introduced `WorldRenderer`
- introduced `DebugOverlayRenderer`
- removed rendering logic from `GameLayer`
- formalized read-only rendering inputs
- improved separation between:
  - simulation
  - gameplay logic
  - presentation

---

## Current Status

WAR is now a structured gameplay prototype with:
- modular backend systems
- separated renderer layer
- a cleaner frame flow ready for future renderer replacement

---

## Next Milestone

### M8 — Stateful World Objects

- interactions should change object state
- crate can be opened once
- locker can be locked/unlocked
- terminal can toggle state
- event log should reflect persistent consequences
