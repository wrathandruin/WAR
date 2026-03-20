# Wrath and Ruin — Master Production Plan to Alpha

---

# 1. Overview

Wrath and Ruin is a simulation-heavy, multiplayer, terrain-deforming roguelike RPG. This plan defines a **production-grade roadmap to Alpha**, including:

* Milestones
* Weekly sprint structure
* Technical sequencing
* Keystone system architectures

Estimated Timeline: **12–18 months**

---

# 2. Milestones

## 2.1 Pre-Production (4–6 weeks)

### Objectives

* Lock architecture
* Validate high-risk systems
* Define data models

### Deliverables

* ECS schema
* Terrain data model (heightmap + delta)
* Networking model (authoritative intent pipeline)
* Terrain prototype (flatten + dig)

### Risks

* Terrain scalability
* ECS inefficiency

### Validation

* Terrain supports flatten + cliff carving
* ECS handles 5k+ entities

---

## 2.2 Engine Foundation (8–12 weeks)

### Objectives

Build simulation-capable engine core

### Deliverables

* bgfx rendering
* EnTT ECS base
* Input system (click + raycast)
* Fixed timestep loop
* Save/load v1 (chunk-based)

### Validation

* Click-to-move works
* World persists

---

## 2.3 Vertical Slice (10–14 weeks)

### Objectives

Prove core gameplay loop

### Deliverables

* Terrain (flatten + horizontal dig)
* Pathfinding
* Turn-based combat
* Survival (oxygen)
* Basic AI
* MUD console v1

### Validation

* Player can move, dig, fight, survive

---

## 2.4 Core Systems (12–20 weeks)

### Objectives

Expand into full sandbox

### Deliverables

* Terrain delta system
* Construction + interiors
* Atmosphere simulation
* Combat v2 (cover + ballistics)
* AI v2 (task-based)
* Character + skills
* Inventory + economy
* Narrative system
* Save/load v2 (delta persistence)

---

## 2.5 Multiplayer (10–16 weeks)

### Objectives

Authoritative server implementation

### Deliverables

* Intent-based networking
* ECS replication
* Terrain delta sync
* Combat sync
* Persistence server

---

## 2.6 Alpha (8–12 weeks)

### Objectives

Stable, complete gameplay loop

### Deliverables

* All core systems integrated
* Multiplayer stable (5–20 players)
* UI v2
* Death system (permadeath + backup)

---

# 3. Weekly Sprint Plan (High-Level)

## Phase 1: Pre-Production (Weeks 1–6)

### Week 1–2

* Finalize architecture
* Define ECS components

### Week 3–4

* Terrain prototype
* Digging + flattening spike

### Week 5–6

* Networking model design
* Save system planning

---

## Phase 2: Engine Foundation (Weeks 7–18)

### Week 7–10

* Rendering (bgfx)
* ECS core systems

### Week 11–14

* Input + raycasting
* Movement system

### Week 15–18

* Save/load v1
* Debug tooling

---

## Phase 3: Vertical Slice (Weeks 19–32)

### Week 19–22

* Terrain system v1

### Week 23–26

* Pathfinding
* Navigation

### Week 27–30

* Combat v1
* Turn system

### Week 31–32

* AI basic
* UI + MUD console

---

## Phase 4: Core Systems (Weeks 33–52)

### Week 33–38

* Terrain delta system
* Chunk streaming

### Week 39–44

* Construction + interiors
* Atmosphere simulation

### Week 45–48

* Combat v2
* Equipment + damage

### Week 49–52

* Character + skills
* Inventory + economy

---

## Phase 5: Multiplayer (Weeks 53–68)

### Week 53–56

* Networking core

### Week 57–60

* Entity replication

### Week 61–64

* Terrain sync

### Week 65–68

* Multiplayer testing + fixes

---

## Phase 6: Alpha (Weeks 69–80)

### Week 69–72

* Integration + stability

### Week 73–76

* UI polish
* Social systems

### Week 77–80

* Performance optimization
* Bug fixing

---

# 4. Keystone System Architectures

## 4.1 Terrain System (CRITICAL)

### Structure

* Base Heightmap (immutable seed)
* Delta Layer:

  * modifications
  * digging
  * construction

### Tile States

* Solid
* Flattened
* Excavated
* Reinforced

### Chunking

* World divided into chunks (e.g. 32x32)
* Each chunk stores:

  * height data
  * delta modifications

### Key Systems

* Terrain Query System
* Terrain Modification System
* Pathfinding Integration

### Critical Rules

* Never mutate base terrain
* Always apply delta layer

---

## 4.2 ECS Architecture (EnTT)

### Core Components

* Transform
* Actor
* TerrainTile
* Physics
* Inventory

### Systems

* Movement System
* Combat System
* Terrain System
* AI System
* Networking System

### Execution Model

* Fixed order update
* Deterministic

---

## 4.3 Networking Architecture

### Model

Authoritative server

### Flow

1. Client sends intent
2. Server validates
3. Server simulates
4. Server sends snapshot

### Key Concepts

* No client authority
* Deterministic simulation
* Delta replication

---

## 4.4 Combat System

### Turn Model

* 6-second turns
* Action queue

### Systems

* Ballistics
* Cover evaluation
* Damage resolution

---

## 4.5 AI Architecture

### Model

* Task-based system

### Layers

* Perception
* Decision
* Execution

---

## 4.6 Save/Load System

### Structure

* Chunk-based persistence
* Delta storage

### Requirements

* Fast load
* Partial streaming

---

# 5. Technical Sequencing

## Must Come First

1. ECS + simulation loop
2. Terrain system
3. Save/load

## Parallelizable

* UI
* Narrative
* AI behaviors

---

# 6. Alpha Definition

## Playable

* Full gameplay loop
* Multiplayer interaction
* Terrain editing

## Incomplete

* Content depth
* Advanced AI

## Stable

* Core systems
* Multiplayer (small scale)

---

# 7. Final Notes

* Terrain is the keystone system
* Determinism is mandatory
* Vertical slice must validate digging + combat
* Multiplayer should not be rushed early

---

End of Document
