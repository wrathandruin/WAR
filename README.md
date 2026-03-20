# WAR — Milestone 8 (Stateful World Objects)

> Current development milestone: M8 — Stateful World Objects

Milestone 8 for **Wrath and Ruin (WAR)**.

## Focus of this milestone
This milestone makes world objects stateful so interactions have consequences.

- crates can be opened once
- terminals toggle power state
- lockers have lock/open state
- inspect reflects current object state
- entity visuals now reflect their current state

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
- object state changes

### WorldRenderer
Handles:
- tile drawing
- hovered tile highlight
- path rendering
- entity rendering
- player rendering
- state-based entity visuals

### DebugOverlayRenderer
Handles:
- debug text
- hovered information
- event log

### GameLayer
Responsible for:
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
- Stateful world objects

---

## Controls

- **Left click**: Move
- **Right click**: Interact
- **Shift + Right click**: Inspect
- **Middle mouse drag**: Pan camera
- **Mouse wheel**: Zoom

---

## What changed from M7

- entities now carry state:
  - `isOpen`
  - `isLocked`
  - `isPowered`
- crates open once
- terminals toggle on/off
- lockers can be locked/opened
- inspect now reports current object state
- entity visuals reflect current state

---

## Current Status

WAR now has a stateful interaction loop:
- movement
- inspect
- interact
- persistent object state during runtime
- separated simulation and presentation layers

---

## Next Milestone

### M9 — bgfx Rendering Backend

- replace GDI renderer implementation with bgfx
- preserve renderer interfaces
- keep gameplay systems unchanged
