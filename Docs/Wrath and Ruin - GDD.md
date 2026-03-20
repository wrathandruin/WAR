# Wrath and Ruin – Game Design Document

---

## 1. High Concept

Wrath and Ruin is a **hard sci-fi, top-down roguelike RPG** combining:

- Point-and-click tactical gameplay
- Turn-based combat (6-second rule system)
- Persistent, editable terrain (2D visuals with 3D simulation logic)
- MUD-inspired narrative and social systems
- Multiplayer persistent world with high consequence progression

The game emphasizes:
- survival
- systemic gameplay
- player-driven world shaping
- meaningful death and continuity

---

## 2. Core Pillars

### 2.1 Tactical Point-and-Click Gameplay
- Click to move
- Click to interact
- Click to attack
- Context-sensitive interactions
- Minimal reliance on typed commands

---

### 2.2 Turn-Based Combat (6-Second System)
- Each turn = 6 seconds of in-game time
- Action-based system:
  - move
  - attack
  - interact
  - reload
- Initiative-driven or queue-based execution

---

### 2.3 The MUD Soul
- Bottom-right text console
- Displays:
  - speech
  - emotes
  - combat narration
  - environmental storytelling
- Commands:
  - `say`
  - `emote`
  - `look`
  - `inspect`

Text is dynamically generated from simulation state.

---

### 2.4 Hard Sci-Fi Constraints

#### No-Fly List:
- No FTL travel
- No energy shields
- No artificial gravity (except spin/acceleration)

#### Systems:
- atmosphere
- pressure
- radiation
- heat
- component-based damage

---

### 2.5 2D Visuals, 3D World Logic
- Top-down 2D rendering
- Underlying simulation includes:
  - heightmaps
  - terrain deformation
  - structural volumes
  - layered interiors

---

### 2.6 Persistent Editable Terrain

Players can:
- flatten terrain
- dig horizontally into cliffs
- create interior spaces
- build on modified terrain

Players cannot:
- freely dig downward into full voxel space

---

## 3. Setting – The Solar Fracture

### Year: 2250

### Earth
- Ecological preserve
- Controlled by "The Mandate"

### Factions

#### Jovian League
- High-tech
- Bureaucratic
- Heavy gravity adapted

#### Belt Freeholds
- Independent
- Modular construction experts
- Resource-focused

---

## 4. Core Gameplay Loop

1. Complete tutorial
2. Develop character
3. Train skills
4. Acquire resources
5. Earn currency
6. Socialize and form relationships
7. Join or form a clan (optional)
8. Engage in combat
9. Capture or defend territory
10. Build structures and ships
11. Backup neural state
12. Avoid death

---

## 5. World Structure

### 5.1 Terrain Layers

#### Surface Layer
- Generated from displacement maps
- Defines base terrain

#### Delta Layer
Stores:
- terrain edits
- construction
- damage
- mining changes

#### Interior Layers
- tunnels
- bases
- ships
- underground spaces

---

## 6. Terrain System

### 6.1 Tile States
- Solid
- Surface
- Flattened
- Excavated
- Reinforced

---

### 6.2 Flattening
- Levels terrain
- Enables building
- Removes slope variance

---

### 6.3 Horizontal Digging
- Carves into cliffs
- Creates interior spaces
- Reveals ore deposits

Constraints:
- Must dig from exposed face
- Requires tools

---

## 7. Character System

### 7.1 Attributes
- Strength
- Agility
- Endurance
- Intellect
- Perception
- Willpower

---

### 7.2 Skills
- Ballistics
- Melee
- Evasion
- Zero-G
- Engineering
- Atmospherics
- Mining
- Medicine
- Piloting
- Salvage
- Social
- Trade

---

## 8. Progression System (SWRFUSS Hybrid)

### 8.1 Class Model

Players choose a **Primary Class**:
- full skill caps
- defines specialization

Secondary classes:
- reduced caps
- broader capability

---

### 8.2 Skill Progression
- usage-based progression
- training systems
- no strict character level gating

---

## 9. Needs & Survival

- Oxygen
- Hydration
- Fatigue
- Radiation

Failure results in:
- debuffs
- unconsciousness
- death

---

## 10. Combat System

- Turn-based
- 6-second turns
- Cover system
- Ballistic weapons
- Environmental damage

Damage affects:
- body parts
- equipment
- terrain

---

## 11. Construction System

- Tile-based building
- Requires flattened terrain
- Systems:
  - power
  - atmosphere
  - structure integrity

---

## 12. Narrative System

Dynamic text generation based on:
- environment
- actions
- combat
- social interactions

---

## 13. Multiplayer

- Authoritative server model
- Client sends intent
- Server validates actions
- Shared persistent world

---

## 14. Death & Continuity

### With Backup
- clone restoration
- skill decay
- progress rollback

### Without Backup
- permanent death
- loss of character

---

## 15. Economy

- resource gathering
- trading
- contracts
- scarcity-driven value

---

## 16. Social Systems

- emotes
- speech
- reputation
- alliances and rivalries

---

## 17. Visual Direction

- RimWorld-style terrain readability
- Police Stories-style characters
- strong silhouette clarity

---

## 18. Technical Architecture

- C++ custom engine
- SDL3 (platform)
- bgfx (rendering)
- EnTT (ECS)
- Emscripten (web)
- React (tooling)

---

## 19. Design Constraints

### Retained (SWRFUSS)
- class + skill system
- social depth
- persistence
- economy

### Removed
- text-only navigation
- command-only gameplay

---

## 20. Vision Statement

Wrath and Ruin is a **simulation-driven roguelike RPG** where:

- players shape the world physically
- text gives meaning to actions
- systems create emergent stories
- death always matters