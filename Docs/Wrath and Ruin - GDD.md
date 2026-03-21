# Wrath and Ruin - Game Design Document

## 1. High Concept

Wrath and Ruin is a **top-down hard science-fiction RPG roguelike sandbox** that preserves the soul of a persistent multiplayer MUD while presenting the world through a tactical graphical client.

The game combines:

- point-and-click tactical movement and interaction
- six-second combat resolution
- persistent editable terrain
- text-supported roleplay and narrative feedback
- multiplayer continuity and consequence
- connected planetary and space gameplay

The player fantasy is not “arcade action in space.”
It is **living in a harsh, persistent frontier where physical survival, social identity, and tactical decisions all matter**.

---

## 2. Product Positioning

Wrath and Ruin should feel like:

- a roguelike with real persistence
- a multiplayer RPG with consequence
- a hard-sci-fi sandbox with grounded systems
- a MUD whose world can now be seen and navigated directly

It should not feel like:

- a room-based MUD with a minimap pasted on top
- an action game that abandoned text and roleplay depth
- a broad survival sandbox with no authored mission spine

---

## 3. Core Pillars

### 3.1 Tactical Top-Down Play

- click to move
- click to inspect and interact
- click to attack or engage
- readable world-state feedback
- tactical positioning and line-of-sight matter

### 3.2 The MUD Soul

- text remains first-class
- speech, emotes, logs, and inspect text remain central
- dynamic narration explains combat, hazards, and social context
- commands remain available as a support layer, not the primary navigation model

### 3.3 Hard Sci-Fi Consequence

- no fantasy safety nets
- environmental conditions matter
- ships, structures, and terrain obey grounded constraints
- injury, exposure, and logistics carry consequence

### 3.4 Persistent Editable World

- terrain can be changed in meaningful ways
- mission and world state persist
- the world remembers what players did
- death, loss, and recovery have real cost

### 3.5 Planet And Space Continuity

- on-foot planetary play and ship/space play must belong to the same game
- space is not a disconnected minigame
- travel should serve mission, survival, and persistence

### 3.6 Multiplayer Continuity

- the world is shared
- the server is authoritative
- social identity matters
- consequence carries across sessions

---

## 4. Core Gameplay Loops

### 4.1 Moment-To-Moment Loop

1. observe the world
2. move and position
3. inspect, interact, or engage
4. resolve risk through combat, survival, or negotiation
5. collect information, loot, or access

### 4.2 Session Loop

1. enter the world
2. pursue a local objective
3. manage resources, inventory, and risk
4. survive a conflict or hazard
5. improve position, equipment, or world state
6. log out with meaningful persistence

### 4.3 Alpha Vertical Slice Loop

1. connect to a hosted session
2. explore a planetary location
3. interact with terrain, containers, terminals, and hazards
4. resolve at least one six-second combat encounter
5. board a ship
6. transition to space or orbital travel
7. reach a second destination
8. complete a mission chain
9. return to a persistent world state

---

## 5. World And Setting

Wrath and Ruin is grounded in a hard-sci-fi frontier where environment, scarcity, and mobility define daily life.

### Setting Principles

- civilization is fragmented
- survival is physical, social, and economic
- planets, stations, outposts, and vessels all matter
- space is connective tissue, not empty menu space

### World Structure

The long-term world may become broad, but the early product must stay narrow:

- one planetary location for alpha
- one local space or orbital layer
- one second destination
- one mission chain spanning both

---

## 6. System Pillars

### 6.1 Terrain

- top-down readable terrain
- flattening and horizontal digging
- terrain state affects traversal, cover, and construction potential
- terrain changes persist

### 6.2 Combat

- six-second combat cadence
- action declaration and resolution
- tactical positioning
- cover and environmental consequence
- readable combat narration

### 6.3 Survival

- oxygen and atmosphere pressure
- radiation or environmental exposure
- resource pressure that is meaningful but not tedious
- hazards that reinforce the hard-sci-fi tone

### 6.4 Inventory And Equipment

- equipment loadout
- loot and containers
- tools that matter to traversal and interaction
- equipment and condition should influence survival and combat outcomes

### 6.5 Progression

- skill and capability growth
- equipment progression
- mission and reputation consequences
- progression should reinforce identity and specialization rather than raw number inflation

### 6.6 Ships And Space

- player ship as a real runtime object
- boarding and embark flow
- local space or orbital movement
- docking and landing transitions
- ships as connective gameplay, not only transportation

### 6.7 Missions And Narrative

- mission chains provide structure to the sandbox
- terminals, dialogue, and logs deliver context
- room, interior, and destination descriptions should add immediate authored identity when the player enters meaningful spaces
- authored content should coexist with systemic outcomes

---

## 7. Social And Text Layer

Wrath and Ruin should preserve the strongest parts of its MUD heritage:

- speech
- emotes
- inspect text
- room and interior descriptions
- event and combat logs
- world flavor text
- roleplay support

The text layer gives meaning to the visible world.
It should never be treated as disposable flavor.

---

## 8. Visual And Audio Direction

### Visual Direction

- top-down readability comes first
- strong silhouette clarity for player, NPCs, hazards, and interactables
- grounded hard-sci-fi materials and structures
- terrain and structural states must be readable at a glance

### Audio Direction

- environmental ambience should reinforce mood and threat
- hazards and combat need strong readability
- UI and feedback sounds should help cognition, not overwhelm

---

## 9. Technical Architecture Direction

The product should be built around:

- a Windows-first client for early delivery
- a server-authoritative runtime
- shared simulation logic between client and server where appropriate
- bgfx or equivalent rendering abstraction for the client
- data-driven content and mission definitions
- reproducible build and packaging practices
- clear separation between versioned source and mutable runtime state

The new repo should inherit production lessons from the old project even when code is not directly ported.

---

## 10. Delivery Horizons

### Foundation

Build the production base:

- runtime boundaries
- packaging direction
- diagnostics
- authoritative architecture
- deployability planning

### Alpha

Deliver one complete, deployable, demoable vertical slice.

### Beta

Expand usability, tester support, content reliability, and operational maturity.

### Market

Prepare commercial packaging, onboarding, update flow, support, analytics, and release operations.

---

## 11. Scope Discipline

### Must Be True Before Alpha

- one complete planet-to-space slice exists
- persistence works
- the server is authoritative
- the game is deployable for controlled testing

### Must Not Expand Before Alpha

- large content breadth
- large world count
- deep economy simulation
- wide ship roster
- broad crafting tree
- social platform feature sprawl

### Acceptable After Alpha Or Beta

- more destinations
- deeper faction systems
- broader economy
- richer progression breadth
- larger content libraries

---

## 12. Final Vision Statement

Wrath and Ruin is a **persistent hard-sci-fi roguelike RPG** where:

- players act in a visible tactical world
- text gives social and narrative meaning to those actions
- planets and space belong to one continuous experience
- the server preserves consequence and continuity
- the product is shaped for real deployment, real demos, and eventual market readiness

The goal is not only to build an interesting prototype.
The goal is to build a foundation strong enough to become a real product.
