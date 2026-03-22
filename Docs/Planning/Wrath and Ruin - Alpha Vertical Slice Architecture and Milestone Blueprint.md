# Wrath and Ruin - Alpha Vertical Slice Architecture and Milestone Blueprint

## Source Of Truth Note

This document defines the revised vertical-slice program for the first alpha-ready product.

It expands the existing roadmap rather than erasing already signed-off milestone history.
Use it as the planning brief for:

- the lead developer
- the lead artist
- production and milestone owners

Read alongside:

- `Docs/Planning/Wrath and Ruin - GDD.md`
- `Docs/Planning/Feature List.md`
- `Docs/Planning/Wrath and Ruin - Master Production Plan to Alpha.md`
- `Milestones/WAR_Strategic_Roadmap.md`
- `Docs/Art/Wrath and Ruin - Lead Artist Considerations.md`
- `Docs/Art/Wrath and Ruin - Art Production and Asset Planning Track.md`
- `Docs/Art/Wrath and Ruin - Art Asset Delivery Contract.md`

If an older planning document conflicts with this one on alpha scope, this document wins for forward planning.
If an older milestone record is already signed off, keep the signed record and treat this document as the next planning layer.

---

## 1. Purpose

Wrath and Ruin should stop being planned as a loose list of good ideas and start being planned as a narrow, complete, alpha-ready product.

The product target is not:

- a broad sandbox with partial systems everywhere
- a renderer tech demo with weak game logic
- a MUD port with a prettier map

The product target is:

- one crisp first 30-minute play experience
- one coherent level 1 through 10 rules spine
- one primary planetary location
- one disconnected onboarding lane
- one basic but real ship-grid and ship-prototype lane
- one basic but real flight lane
- one basic but real construction lane
- one basic but real computer-usage lane
- one strong text-supported top-down presentation layer
- one hosted, repeatable, supportable alpha package

This document therefore defines:

- the alpha-ready product architecture
- the gameplay and content architecture
- the renderer and technical architecture
- the art-production architecture
- the vertical-slice milestone ladder from current state to alpha candidate

---

## 2. Product Definition

### 2.1 Alpha Product Statement

The first alpha-ready Wrath and Ruin build must let a player:

- create or select a character quickly
- complete a guided disconnected onboarding area
- learn movement, interaction, equipment, terminals, shops, language/help, hazards, and basic combat
- board and traverse a shuttle
- arrive in the real Ceres slice
- explore a small but authored spaceport district
- take and complete several short quests
- use a computer terminal for a meaningful gate or task
- perform a basic construction or repair action
- board and operate a ship prototype on a primary ship grid
- complete a basic local flight and docking sequence
- continue progression and unlock class expression up to level 10
- preserve character, mission, inventory, and world state across reconnect and travel

### 2.2 The 30-Minute Rule

The first 30-minute playtest must feel crisp, complete, and representative.
That does not mean the player naturally earns ten levels in thirty minutes.

Production rule:

- the first-time player critical path should usually end around levels 3 to 4 inside a 30-minute session
- the alpha product must still support systems, content gates, and balance scaffolding through level 10
- internal validation must include prebuilt level 10 characters so advanced combat, flight, construction, and computer-use loops can be tested without forcing a 10-hour grind before validation

### 2.3 Alpha Success Criteria

The alpha-ready product is successful only if it proves all of the following at the same time:

- readable and attractive top-down tactical presentation
- strong core verbs
- authored atmosphere and text identity
- stable server authority and persistence
- a believable first ship and space loop
- a believable first progression loop
- real onboarding and replayability
- enough art lock that the lead artist is reviewing polish, not guessing the shape of the game

---

## 3. Experience Architecture

### 3.1 First-Time Player Golden Path

The first-time player path should be structured like this:

1. Character creation or quickstart archetype selection.
2. Wake in the disconnected onboarding academy.
3. Learn movement, examine, interact, equip, and use a terminal.
4. Complete a safe trainer-guided combat lesson.
5. Learn food, water, med use, and death/downed consequences.
6. Learn one simple construction or repair action.
7. Learn one simple ship-related task through a tutorial shuttle.
8. Depart onboarding on a shuttle and lose access to the newbie zone.
9. Arrive in the real Ceres spaceport slice.
10. Take a quest, shop, equip, and talk to at least one guide NPC.
11. Board a ship prototype or guided transport.
12. Complete one basic flight or docking objective.
13. Return to a persistent authored space with the next objective visible.

### 3.2 The 30-Minute Session Beat Map

Suggested pacing:

- Minute 0-5: character entry, identity, controls, first room description
- Minute 5-10: movement, interaction, inventory, help, language, first terminal
- Minute 10-15: first combat, cover, healing, loot, first level-up beat
- Minute 15-20: first repair or construction interaction, shuttle boarding
- Minute 20-25: arrival at Ceres spaceport, first shop, first quest, stronger atmosphere
- Minute 25-30: first ship interface, first flight or docking step, next-session hook

### 3.3 Level 10 Validation Path

The lead developer and design team also need an explicit level 10 validation route.

That route must prove:

- level 10 character sheet coherence
- level 10 class identity
- level 10 encounter readability
- level 10 ship interaction and piloting permission gates
- level 10 construction and computer-usage mastery gates
- level 10 gear, economy, and mission reward sanity

### 3.4 Alpha Scene Set

The alpha product only needs a narrow authored scene set:

- onboarding academy
- shuttle interior
- Ceres spaceport district
- one small exterior or docking apron
- one orbital transition layer
- one station or secondary destination
- one ship prototype interior on a primary ship grid

If a new environment does not strengthen the first 30-minute route or the level 10 validation route, it should not be added before alpha.

---

## 4. Rules Architecture

### 4.1 Core System Lock

Wrath and Ruin should use a single coherent hybrid ruleset rather than trying to stack whole DnD editions on top of one another.

The recommended hybrid is:

- `5e` as the math glue
- `3.5` as the build and feat engine
- `4e` as the encounter and role engine
- `2e` as the flavor and kit engine
- `old-school DnD` as the exploration and procedure engine
- `SWRFUSS` as the content, verb, world, and profession inspiration layer

### 4.2 Core Character Sheet

Lock the core sheet to:

- Strength
- Dexterity
- Constitution
- Intelligence
- Wisdom
- Charisma

Lock primary derived values to:

- hit points
- armor class
- initiative
- speed
- proficiency bonus
- Fortitude
- Reflex
- Will
- passive perception or awareness
- carrying capacity
- recovery surges or equivalent

### 4.3 Action Economy

The alpha rules spine should keep the current six-second cadence and resolve characters through a compact but tactical action model:

- move
- standard action
- quick action
- reaction
- free interaction

This keeps the game readable while still supporting:

- cover
- overwatch or readied actions later
- weapon swaps
- interaction with doors, terminals, containers, and devices
- pilot and engineering actions during ship sequences

### 4.4 Combat Model

The alpha combat model should support:

- ranged and melee attacks
- cover
- concealment
- obscurity and visibility
- facing
- weapon range bands
- class abilities
- critical hits
- status effects
- downed state
- death or recovery consequences

Use:

- attack rolls against AC for weapons
- Fortitude, Reflex, and Will for hazards, powers, systems, morale pressure, toxins, fear, radiation, and tech effects

### 4.5 Progression Model

The alpha-ready product must support level 1 through 10.

The progression model should include:

- class level
- feat unlocks
- ability increases at milestone levels
- equipment progression
- skills or proficiency growth
- class feature unlocks
- trainer or terminal-gated knowledge where useful

Avoid:

- giant 3.5 modifier stacking
- exact 3.5 skill-point bookkeeping
- 4e power-card overload
- unclear 5e-only "GM fiat fixes everything" design

### 4.6 Class Set For Alpha

The alpha class set should stay narrow but expressive.

Recommended starter classes:

- Armsman: frontline combat, durability, suppression, boarding combat
- Operator: stealth, infiltration, terminals, recon, utility
- Engineer: repair, construction, systems, drones or deployables later
- Pilot: navigation, flight, vehicle handling, initiative and maneuvering
- Marshal: leadership, command, squad utility, social authority
- Adept: limited but distinct supernatural or rare high-sensitivity path if lore allows it

### 4.7 Backgrounds And Kits

Each character should also select:

- one background
- one kit

Background examples:

- Dockworker
- Freight Runner
- Security Recruit
- Salvage Tech
- Med Trainee
- Colony Drifter
- Academy Washout

Kit examples:

- Boarding Specialist
- Flight Cadet
- Reactor Hand
- Port Inspector
- Smuggler Runner
- Crisis Responder

### 4.8 Skill And Verb Architecture

Alpha skills should not be an enormous menu.
They should be grouped into meaningful gameplay lanes that support the first playtest and the level 10 target.

Recommended lanes:

- Combat
- Piloting
- Engineering
- Construction
- Computers
- Medicine
- Survival
- Diplomacy
- Leadership
- Stealth
- Perception
- Logistics

### 4.9 SWRFUSS Verb Mapping

Use SWRFUSS as the source for practical game verbs and profession flavor.
Map its useful actions into the new rules spine.

Alpha-critical verbs:

- move
- look or inspect
- open or close
- lock or unlock
- board
- enter
- launch
- land
- buy
- sell
- list
- inventory
- equipment
- wear
- wield
- eat
- drink
- practice
- train
- scan
- search
- repair
- use terminal
- say
- tell
- emote
- help
- quest

### 4.10 Level Bands

The level bands should be authored intentionally.

Levels 1-2:

- movement
- interaction
- one class identity hook
- one combat ability
- one non-combat utility

Levels 3-4:

- stronger class definition
- terminals and gated access
- better itemization
- early shuttle and travel proof

Levels 5-6:

- basic flight certification
- basic construction or repair authority
- more dangerous enemies and hazards

Levels 7-8:

- advanced mission chains
- stronger tech, command, or engineering roles
- first real prototype-ship responsibility

Levels 9-10:

- level 10 class proof
- high-difficulty encounter proof
- advanced computer or piloting tasks
- advanced construction or repair proof
- alpha-end aspirational content

---

## 5. World Architecture

### 5.1 World Topology

Use a narrow topology with explicit connected and disconnected spaces.

Connected spaces:

- Ceres surface slices
- Ceres spaceport district
- orbital transit space
- station or destination space
- vehicle interiors that exist as real runtime spaces

Disconnected spaces:

- newbie academy
- tutorial shuttle if desired
- training simulators
- any safe scripted onboarding chambers that should not persist into the public world

### 5.2 Ceres Scope Lock

For alpha, Ceres should remain small and intentional.

The playable real-world footprint only needs:

- one spaceport district
- one or two adjacent support spaces
- one docking or transit apron
- one orbital lane
- one secondary destination

### 5.3 Grid Architecture

The alpha-ready product needs explicit grid models, not just a single terrain board.

Required grids:

- planetary traversal grid
- building and interior grid
- primary ship prototype grid
- shuttle interior grid
- local orbital navigation grid or route network

### 5.4 Primary Ship Grid Requirement

One of the missing must-haves is the first ship-grid lane.
That must now be treated as alpha-critical.

Alpha ship-grid requirements:

- authorable multi-room ship interior
- room adjacency and door ownership
- hatches, doors, and airlock logic
- engineering room, cockpit, cargo, and entry space
- power, repair, and computer interaction points
- placement rules for at least a minimal construction or repair loop

### 5.5 Ship Prototype Set

Alpha should lock a minimum prototype roster:

- Shuttle Prototype A: tutorial and transport use
- Utility Skiff Prototype B: simple player-facing starter vessel
- Station Service Craft C: non-player contextual traffic or mission hook
- Primary Ship Prototype D: the first true ship-grid validation vessel

### 5.6 Building Kit Requirements

Buildings should no longer be implied by floor tiles alone.

Alpha environment architecture must support:

- walls
- corners
- T-junctions
- sealed doors
- sliding hatches
- windows or viewports where needed
- roof or upper-wall cutaway rules
- readable interior versus exterior boundaries

---

## 6. Runtime And Technical Architecture

### 6.1 Workspace Ownership

The repo should keep strengthening the product workspace split:

- `desktop/` for client entry, UI shell, renderer, local diagnostics
- `server/` for host bootstrap, services, authority, admin, packaging
- `shared/` for rules, simulation, data types, content loaders, serialization, combat, mission, and ship systems
- `contracts/` for protocol, persistence schemas, content contracts, version ownership
- `web/` only when real launcher, account, or portal work justifies it

### 6.2 Authoritative Runtime Ownership

The authoritative server must own:

- movement validation
- combat resolution
- visibility truth
- mission state
- inventory and equipment truth
- construction and repair truth
- ship systems and travel state
- persistence writes

The client should own:

- input
- prediction only where safe
- presentation
- UI
- local feedback
- diagnostics

### 6.3 Zone And Node Model

Even though alpha concurrency will be small, the architecture should prepare for future zoning.

Define a clean seam between:

- planetary zones
- orbital zones
- ship interiors
- disconnected tutorial instances

Alpha only needs one host process if needed, but the data model should already distinguish:

- world id
- zone id
- layer id
- vessel id
- instance id

### 6.4 Persistence Domains

Persistence should be separated by domain:

- account or session identity
- character sheet and progression
- inventory and equipment
- mission state
- zone state
- ship state
- construction state
- authored world state deltas

### 6.5 Content Contract

Content should not stay implicit in code.
Alpha needs explicit data-driven contracts for:

- classes
- feats
- abilities
- items
- terminals
- quests
- room descriptions
- NPCs
- ship prototypes
- construction recipes or templates
- encounter definitions
- tutorial triggers

### 6.6 Mission System Requirements

The mission system must support:

- prerequisites
- step sequencing
- explicit objective states
- fail or reset rules
- reward bundles
- room-description hooks
- terminal hooks
- travel hooks
- level gating

### 6.7 Computer And Terminal System

Computer usage is alpha-critical.

The terminal system must support:

- authentication or access tiers
- interact prompt
- menu or command actions
- mission gating
- ship functions
- cargo or inventory lookup
- travel or docking confirmation
- audit/log text output

### 6.8 Construction System

Construction should stay basic but real at alpha.

Alpha construction needs:

- build permission checks
- repair actions
- place or replace a small set of ship or structure components
- construction materials or resource consumption
- construction failure or interruption states
- persistence

### 6.9 Scale Preparation Rule

The game does not need 10 million players at alpha.
It does need seams that make future scaling believable.

Prepare now for:

- zoned authority boundaries
- interest management
- message batching
- persistence ownership boundaries
- service extraction later

Do not attempt full massive-scale infrastructure at alpha if it slows the first shippable product.

---

## 7. Rendering Architecture

### 7.1 Rendering Goal

The renderer should no longer aim to be "prettier than before."
It should aim to make the game:

- readable
- atmospheric
- materially rich
- tactically clear
- recognizably premium in a top-down 2.5D style

### 7.2 Alpha Rendering Rules

Renderer work must support gameplay first.
The highest-priority renderer questions are:

- Can the player read walls, doors, hatches, hazards, and objectives instantly?
- Can the player read cover, concealment, visibility, and aggro?
- Can the player read their character's facing, state, and loadout?
- Can the player tell the difference between onboarding, shuttle, spaceport, ship, orbital, and station spaces?

### 7.3 Required Render Pipeline Features For Alpha

Alpha-mandatory renderer features:

- tighter minimum zoom-in
- capped maximum zoom-out
- authored zoom bands by context
- multipart character composition
- directional animation support
- dynamic local lighting
- ambient lighting model
- shadow system
- fog of war
- line-of-sight and explored-memory overlay
- material masks
- normal-map support
- emissive support
- ambient occlusion or authored occlusion support
- global texture overlays for anti-repetition
- multi-tile material stamping
- wall and roof cutaway or occlusion logic
- hazard overlays
- selection, target, and interaction highlights
- weather, dust, sparks, or atmosphere particles where useful

### 7.4 Reflection Policy

Reflections are desirable, but they should be scoped realistically.

Alpha recommendation:

- support limited planar or probe-based reflections on selected surfaces
- do not block alpha on full screen-space reflection complexity
- use reflections only on special floors, viewports, water, polished metal, or docking surfaces where they materially improve the scene

### 7.5 Shadow Policy

Shadows are not optional if the project wants a high-class renderer.

Alpha shadow requirements:

- directional ambient shadowing for world readability
- local light shadow masks where feasible
- wall and prop contact shadows
- character grounding shadows

Avoid giant fake baked sprite shadows that break readability.

### 7.6 Material Model

The renderer should move toward a layered material stack:

- base color
- mask
- normal
- emissive or FX
- UI or readability overlay where needed

This aligns well with the existing production art direction and reduces the gap between art delivery and runtime appearance.

### 7.7 Anti-Repetition Requirement

The world must stop looking like a repeated tile sheet.

Alpha needs:

- macro variation textures
- global texture passes
- multi-tile texture placement
- authored grime and decal overlays
- edge blending
- room or district-specific surface dressing

### 7.8 Character Presentation Requirement

Characters should be composed from:

- legs
- body
- head
- optional equipment layers

They must support:

- facing
- obscurity overlay
- highlight and target states
- hurt and downed states
- equipment silhouette differences

### 7.9 Camera Requirement

Camera work must be intentional.

Alpha camera rules:

- closer default gameplay zoom
- hard cap on maximum zoom-out
- special zoom contexts for flight, interiors, and tactical combat
- smooth but not floaty transitions
- no zoom setting that makes assets unreadable

### 7.10 Performance Rule

Do not build an overengineered renderer that kills delivery.

The alpha renderer should choose:

- a constrained, strong 2.5D tactical look
- predictable batching
- limited but effective lights
- targeted reflections
- authored shadowing

over:

- expensive generalized rendering experiments that do not improve the first playtest

---

## 8. Character Architecture

### 8.1 Character Runtime Requirements

The character runtime must unify:

- rules sheet
- visual composition
- animation state
- equipment state
- mission state
- survival state
- visibility and obscurity state

### 8.2 Animation Manager

Alpha needs a real animation manager, not ad hoc sprite swaps.

It must support:

- idle
- walk
- interact
- attack
- hit
- death or downed
- recover
- use terminal
- repair or construct
- pilot seated or pilot active where needed

### 8.3 Direction And Facing

Characters must visibly turn toward:

- movement direction
- attack direction
- interaction target
- pilot or terminal engagement direction

### 8.4 Character UI Surfaces

Alpha must include:

- character sheet
- inventory
- equipment
- ability or feat view
- skills view
- active effects
- mission journal

### 8.5 Obscurity And Introduction Overlays

Because identity matters, characters should support:

- introduction overlay or identity panel
- obscured or unknown identity treatment
- faction or social markers where appropriate
- status clarity without clutter

---

## 9. UI, Audio, And Text Architecture

### 9.1 UI Shell

The alpha UI should follow the requested shape:

- clear vitals and hotkeys at the bottom
- left side panel for popout buttons
- room description in the upper right
- communications and event panel in the lower right
- command or terminal input surface integrated cleanly

### 9.2 Text Surfaces

Text remains first-class.

Alpha text surfaces must include:

- room and interior descriptions
- event log
- quest journal
- inspect text
- terminal text
- speech
- emotes
- help

### 9.3 Audio Layers

Alpha audio needs:

- ambient location loops
- machine and object hums
- interaction sounds
- combat sounds
- door and hatch sounds
- UI feedback
- travel and ship ambience

### 9.4 UI Readability Rule

No UI surface should feel like a debug overlay pretending to be final product.
Even placeholder UI must have:

- clear hierarchy
- intentional spacing
- visual identity
- controlled information density

---

## 10. Art Production Architecture

### 10.1 Lead Artist Mission

The lead artist is not only producing assets.
The lead artist is locking the visual truth of the alpha slice.

That includes:

- materials
- silhouettes
- palette
- environment identity
- UI presentation support
- animation readability
- prop and ship identity

### 10.2 Alpha-Critical Art Packs

The alpha-critical art packs are:

- modular character pack
- environment floor and wall pack
- door and hatch pack
- interactable terminal and prop pack
- hazard and overlay pack
- ship prototype pack
- item and inventory icon pack
- UI presentation pack
- VFX pack
- atmosphere and skybox or background pack

### 10.3 Final Art Approval Scenes

The lead artist should sign off final direction through a small set of benchmark scenes:

- onboarding academy corridor
- onboarding terminal room
- shuttle entry bay
- Ceres spaceport concourse
- docking apron
- ship prototype cockpit
- ship prototype engineering bay
- orbital docking scene
- station interior checkpoint

### 10.4 Art Lock Decisions Needed Early

These decisions should be locked before mid-alpha:

- final camera and silhouette style
- Ceres palette and material treatment
- shuttle and ship silhouette family
- door, hatch, and wall language
- character modular rig proportions
- terminal and UI visual language
- hazard iconography and overlay style

### 10.5 Art Review Cadence

Recommended cadence:

- weekly engineering-to-art request review
- milestone midpoint visual review
- milestone exit art acceptance review
- monthly benchmark-scene review against the target look

### 10.6 Placeholder Policy

Placeholder art is allowed only if all three are true:

- the gameplay system is still under active construction
- the placeholder is labeled in the milestone board
- the placeholder does not distort playtest conclusions

---

## 11. Relationship To The Existing Milestone Runway

The current M45-M70 roadmap remains useful, but it under-emphasizes several newly declared alpha-critical lanes:

- renderer uplift beyond the current atlas pass
- multipart characters and animation manager
- basic computer-usage gameplay
- basic construction gameplay
- primary ship-grid prototype gameplay
- level 1 through 10 balance and content proof
- final art approval checkpoints for the lead artist

Use the current M45-M70 milestones as enabling production lanes for:

- hosted sessions
- trust boundaries
- session identity
- crash and operator tooling
- graphical MUD session surfaces
- cohort readiness

Use the new `0.1.x` slice ladder below as the full alpha product plan.

---

## 12. Vertical Slice Milestone Ladder

The product should now be planned as `0.1.1` through `0.1.15`, leading to `0.2.0 Internal Alpha Candidate`.

Each slice must exit with:

- gameplay proof
- technical proof
- content proof
- art proof
- validation proof

### 12.1 Slice 0.1.1 - Alpha Lock

Objective:

- lock product scope, rules spine, camera direction, ship-grid requirement, and art benchmark scenes

Engineering deliverables:

- freeze the WAR Hybrid D20 alpha rules spine
- freeze class list, level band goals, and core verbs
- freeze world topology and grid types
- freeze ship prototype requirements
- freeze renderer feature cutoff for alpha

Art deliverables:

- lock palette
- lock character rig direction
- lock door, wall, hatch, terminal, and ship shape language
- produce benchmark mood boards and scene boards

Exit criteria:

- no open dispute on classes, levels 1-10 target, or primary ship-grid requirement
- no open dispute on alpha benchmark scenes
- no open dispute on zoom target and camera readability rules

### 12.2 Slice 0.1.2 - Renderer Foundation

Objective:

- upgrade the renderer from prototype presentation to alpha-grade readability

Engineering deliverables:

- zoom clamp revision
- camera-context rules
- material-channel support
- light and shadow baseline
- fog-of-war pass
- line-of-sight overlay
- explored-memory overlay
- global texture and macro-variation support

Art deliverables:

- material reference sheets
- lighting test assets
- hazard overlay concepts
- macro texture concepts for Ceres surfaces and ship interiors

Exit criteria:

- the game reads materially richer than the current atlas-only baseline
- the camera no longer allows unreadable zoom states
- at least one benchmark scene proves lighting, shadowing, and fog of war together

### 12.3 Slice 0.1.3 - Character Runtime

Objective:

- introduce the final character presentation and control baseline

Engineering deliverables:

- multipart character composition
- animation manager
- facing and turn rules
- hurt, downed, and interaction states
- identity and obscurity overlays
- equipment-layer support

Art deliverables:

- first modular body pack
- head pack
- torso and leg test pack
- helmet and gear test pack
- animation timing sheet

Exit criteria:

- the player character is no longer a single flat placeholder
- direction and state are readable in play
- at least one NPC class variant reads clearly from silhouette alone

### 12.4 Slice 0.1.4 - Session UI And Text Shell

Objective:

- make the game feel like a real graphical MUD session rather than a debug client

Engineering deliverables:

- vitals and hotkey bar
- left-side popout lane
- room description panel
- communications panel
- event log separation
- command entry shell
- character sheet and journal shell

Art deliverables:

- UI visual treatment
- room description presentation kit
- prompt and vitals treatment
- command-bar treatment
- communication panel treatment

Exit criteria:

- players can play without feeling buried in debug surfaces
- text has hierarchy and atmosphere
- the lead artist signs off the UI direction as compatible with the world art direction

### 12.5 Slice 0.1.5 - Onboarding Academy

Objective:

- build the disconnected newbie area and first-time tutorial route

Engineering deliverables:

- onboarding progression controller
- tutorial prompts and help hooks
- safe trainer NPC logic
- food, water, and med introduction
- movement tutorial triggers
- terminal tutorial triggers

Content deliverables:

- authored onboarding rooms
- help files
- greeting NPCs and guide programs
- one shop
- one skills trainer

Art deliverables:

- onboarding floor and wall kit
- training props
- signage
- tutorial room dressing

Exit criteria:

- a new player can learn the basic controls and verbs without external explanation
- the onboarding area feels authored and distinct
- the player exits with a clear next step

### 12.6 Slice 0.1.6 - Combat And Death Introduction

Objective:

- complete the first truly representative combat loop

Engineering deliverables:

- class starting combat abilities
- cover and concealment finalization
- intro enemy roster
- damage, healing, and status effect tuning
- downed and death consequence loop
- combat log readability

Content deliverables:

- one safe training fight
- one real onboarding fight
- one early Ceres combat encounter

Art deliverables:

- enemy silhouettes
- combat impact VFX
- damage, blood, or sci-fi impact standards
- death and downed indicators

Exit criteria:

- combat feels tactical rather than mechanical
- the player understands danger, healing, and failure
- level 1 and level 10 combat readability are both testable

### 12.7 Slice 0.1.7 - World Interaction And Life Support

Objective:

- make the world feel inhabited and usable outside of combat

Engineering deliverables:

- shop and vendor runtime
- item buying and selling
- food and water runtime
- language and help runtime
- trainer and practice runtime
- object interaction polish

Content deliverables:

- one merchant
- one food source
- one water source
- one trainer
- one language or coded access tutorial
- one clear social hub in Ceres

Art deliverables:

- merchant props
- kiosk and vending art
- food and med props
- signage and life-support dressing

Exit criteria:

- the player can maintain themselves, equip themselves, and prepare for the next quest
- Ceres starts to feel like a place instead of a corridor chain

### 12.8 Slice 0.1.8 - Character Systems Through Level 5

Objective:

- make the character sheet, equipment game, and level-up loop real

Engineering deliverables:

- character sheet page
- inventory page
- equipment page
- class and feat hooks
- level-up flow
- level 1 through 5 balance pass

Content deliverables:

- starter equipment sets
- level-up rewards
- early mission rewards
- class-specific item hooks

Art deliverables:

- icons
- inventory slot art
- equipment silhouette pass

Exit criteria:

- the player can understand who they are and why gear matters
- the level 1 through 5 loop is not spreadsheet work
- level-up decisions feel meaningful but not overwhelming

### 12.9 Slice 0.1.9 - Computers And Security Gameplay

Objective:

- make computer usage a first-class system

Engineering deliverables:

- terminal access tiers
- login or authorization states
- mission-locked computer actions
- ship terminal actions
- security alarms or failure states
- terminal UI and text formatting

Content deliverables:

- onboarding terminal sequence
- Ceres security terminal
- ship systems terminal
- quest-gated computer task

Art deliverables:

- terminal variants
- screen states
- access or alarm indicators
- hacking or use VFX

Exit criteria:

- computers feel like gameplay, not flavor text
- terminal use can gate quests, travel, and ship functions
- computer mastery is visible in the level curve

### 12.10 Slice 0.1.10 - Construction And Repair Basics

Objective:

- prove that players can build, repair, or replace a small set of meaningful world elements

Engineering deliverables:

- place or repair interaction model
- component health or state
- material consumption
- permission and ownership checks
- persistence for changed state

Content deliverables:

- tutorial repair task
- ship repair task
- one build placement task in Ceres or onboard a prototype ship

Art deliverables:

- repair state variants
- broken versus repaired props
- construction ghost or preview visuals

Exit criteria:

- construction is no longer theoretical
- the player can complete a simple real task that changes the world or ship state

### 12.11 Slice 0.1.11 - Primary Ship Grid And Prototypes

Objective:

- stand up the first real ship-grid gameplay lane

Engineering deliverables:

- ship-grid authoring format
- room modules
- hatch and door state logic
- ship device placement hooks
- cockpit and engineering interactions
- interior navigation and occlusion rules

Content deliverables:

- tutorial shuttle interior final pass
- starter utility ship
- main prototype ship interior

Art deliverables:

- ship hull and interior kit
- cockpit kit
- engineering room kit
- cargo and hatch kit

Exit criteria:

- players can move through a believable ship interior
- ship rooms feel different in function and art identity
- the ship is no longer a menu state pretending to be a world object

### 12.12 Slice 0.1.12 - Flight, Navigation, And Docking

Objective:

- deliver the first satisfying flight loop

Engineering deliverables:

- pilot control model
- nav computer actions
- launch and land state transitions
- docking permissions
- route or local orbital traversal
- ship condition interactions during flight

Content deliverables:

- tutorial flight lesson
- one Ceres departure route
- one dock-at-station route
- one return route

Art deliverables:

- orbital background identity
- docking visuals
- flight HUD hooks
- ship-thrust and travel FX

Exit criteria:

- flight feels like gameplay, not a disguised fast-travel button
- launch, travel, and docking are all testable and readable

### 12.13 Slice 0.1.13 - Ceres Quest Arc And Level 6-10 Content

Objective:

- complete the core content arc that carries the player beyond the tutorial promise

Engineering deliverables:

- mission chain support for mid-level objectives
- stronger encounter scripting
- better reward pacing
- level 6 through 10 balance pass
- quest and journal polish

Content deliverables:

- Ceres spaceport quest chain
- station quest chain
- one ship or transport job
- one combat escalation beat
- one repair or computer mastery beat

Art deliverables:

- Ceres identity pass
- station identity pass
- higher-tier enemy and gear pass
- midgame quest prop set

Exit criteria:

- the level 6 through 10 route is playable
- the product no longer feels like it only supports tutorial play

### 12.14 Slice 0.1.14 - Hosted Alpha Operations And Multiplayer Proof

Objective:

- prove the product works as a hosted game and not only a local slice

Engineering deliverables:

- remote transport
- reconnect and recovery
- trust-boundary enforcement
- session identity handoff
- crash capture
- telemetry
- support and operator triage tools
- shared-presence and communication validation

Content deliverables:

- same-space test scenario
- support checklist
- hosted acceptance scripts

Art and UI deliverables:

- multiplayer readability indicators
- chat and social-state polish
- session-state presentation cleanup

Exit criteria:

- a small alpha cohort can actually connect, play, disconnect, recover, and be supported
- hosted play is diagnosable

### 12.15 Slice 0.1.15 - Alpha Polish, Art Lock, Audio Lock, And Rehearsal

Objective:

- finish the alpha candidate rather than endlessly growing it

Engineering deliverables:

- performance budget pass
- input and accessibility cleanup
- save, reconnect, and mission continuity hardening
- final blocker triage

Content deliverables:

- benchmark encounter coverage
- onboarding text finalization
- room descriptions and journal coverage
- final tutorial pacing pass

Art deliverables:

- benchmark scene final pass
- remaining critical placeholder replacements
- VFX and atmosphere pass
- final ship and environment identity lock

Audio deliverables:

- ambience pass
- combat and interaction sound pass
- UI sound pass
- travel and ship sound pass

Exit criteria:

- the team can run a rehearsal without apologizing for missing pillars
- the lead developer can support the build
- the lead artist signs off the representative scene set
- production can label the build `0.2.0 Internal Alpha Candidate`

---

## 13. Alpha Candidate Definition

`0.2.0 Internal Alpha Candidate` is reached only when all of the following are true:

- the first 30-minute route is stable and satisfying
- levels 1 through 10 are structurally supported
- the player can fight, fly, build, repair, and use computers
- the primary ship-grid and prototype ship lane works
- the Ceres arc is authored and readable
- the renderer looks intentional and premium enough to sell the promise
- hosted sessions are supportable
- the lead artist has approved the visual identity of benchmark scenes
- the lead developer has approved the architecture and runtime boundaries

---

## 14. Lead Developer Confirmation Checklist

The lead developer should explicitly confirm:

- the WAR Hybrid D20 rules lock
- the level 1 through 10 support target
- the narrow Ceres scope
- the primary ship-grid requirement
- the renderer feature cutoff for alpha
- the zone and persistence architecture
- the computer and construction systems as alpha-critical
- the hosted-ops lane as non-optional

---

## 15. Lead Artist Confirmation Checklist

The lead artist should explicitly confirm:

- character rig and modular breakdown
- wall, door, hatch, and prop language
- ship prototype visual family
- Ceres material and palette treatment
- UI presentation direction
- benchmark scene targets
- placeholder replacement priorities
- art-lock criteria for alpha

---

## 16. Working Rule

If a task does not strengthen:

- the first 30-minute player experience
- the level 10 rules and content proof
- the first ship-grid and first flight proof
- the first construction and computer-use proof
- the alpha benchmark scenes
- or the hosted alpha support lane

then it is not an alpha-critical task.
