# Wrath and Ruin - Lead Artist Onboarding Package

## Purpose

This is the source-of-truth onboarding package for the lead artist.

It exists to remove interpretation drift.
If this document conflicts with older art notes, this document wins.

## Read This First

Before producing art direction, style frames, or asset plans, read:

1. `Docs/Art/Wrath and Ruin - Art Audit and Direction Lock.md`
2. `Docs/Art/Wrath and Ruin - Lore and Visual Theme Guide.md`
3. `Docs/Art/Wrath and Ruin - Art Asset Delivery Contract.md`
4. `Docs/Art/Wrath and Ruin - Lead Artist Milestone Runway.md`
5. `Docs/Art/Wrath and Ruin - Art Production and Asset Planning Track.md`
6. `Docs/Planning/Wrath and Ruin - GDD.md`
7. `Docs/Planning/Feature List.md`
8. `Docs/Planning/Wrath and Ruin - Master Production Plan to Alpha.md`
9. `Docs/Planning/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md`
10. `Milestones/WAR_Strategic_Roadmap.md`

## 1. Non-Negotiable Direction

Wrath and Ruin is:

- a top-down tactical hard-science-fiction RPG
- a graphical MUD
- human-only in character fantasy
- industrial, scarce, damaged, repaired, and lived-in
- text-supported and roleplay-supportive

Wrath and Ruin is not:

- isometric
- 3/4 tactics perspective
- glossy space opera
- alien fantasy
- neon cyberpunk spectacle
- painterly noise
- over-rendered faux-3D sprite art

## 2. Camera And Perspective Lock

### 2.1 The correct viewpoint

The correct viewpoint is:

- true top-down tactical readability
- overhead floors and ground surfaces
- slight readability cheat only on tall objects

### 2.2 What is allowed

Allowed:

- slight front-face cheat on doors, walls, tall props, and characters
- a small amount of body or front-panel read where needed for clarity
- bottom-anchored tall sprites that still read from above

### 2.3 What is not allowed

Not allowed:

- isometric diamond floors
- diagonal wall systems designed like isometric games
- visible perspective vanishing
- side-on building facades
- props that read like side-view icons placed on top of a top-down map

### 2.4 Important clarification

The engine currently uses an orthographic 2D projection.
That is fine.

The problem to avoid is not orthographic projection.
The problem to avoid is the wrong art language.

Use top-down art language.
Do not use isometric or faux-3D map language.

### 2.5 Visual target reference

Use this mental target:

- readable like RimWorld or Police Stories
- more grounded and industrial than either
- more text-supportive and systemic than either

## 3. Technical Lock

### 3.1 World and view facts

Locked facts from the current repo:

- world size: `48 x 36` tiles
- one gameplay tile footprint: `48 x 48`
- default client window: `1600 x 900`
- approximate on-screen view at zoom `1.0`: `33 x 18` tiles
- zoom range currently supports both broader view and closer inspection

### 3.2 Source-art scale

Recommended working scale:

- create at `2x` when helpful
- export native-sized review assets for approval
- never assume source-size art is what ships to runtime unchanged

### 3.3 Deliverable formats

Every approved asset must include:

- layered source file: `.psd` or `.aseprite`
- review export: `.png`
- production texture exports:
  - `*_base` RGBA
  - `*_mask` RGBA for gameplay-facing assets
  - `*_fx` RGBA when the asset family needs damage, grime, pulse, or special state blending support
  - `*_ui` RGBA when the asset family needs hover, selection, silhouette, focus, or readability support
- engine-ready export when requested: `.bmp`
- preview/contact sheet
- documented pivot/origin
- documented footprint

### 3.4 Production texture stack

For production-ready source art, use the exact delivery contract in:

- `Docs/Art/Wrath and Ruin - Art Asset Delivery Contract.md`

Locked default:

- world gameplay assets use a four-file PNG stack
  - `*_base`
  - `*_mask`
  - `*_fx`
  - `*_ui`
- inventory-bearing items require a matching icon pack
- UI-facing assets use the locked UI surface pack unless a different contract is explicitly approved

Do not reduce this to a one-image or two-image delivery because the current runtime is still simple.

### 3.5 Engine-ready BMP rule

The current runtime loader only supports uncompressed Windows BMP.

Engine-ready runtime exports must be:

- uncompressed Windows BMP
- `24-bit` for fully opaque atlas content
- `32-bit` for cutout art needing alpha

Do not hand engineering PNG-only runtime art for immediate integration.

Important:

The BMP lane is an integration compatibility lane, not the full production source-art standard.

When immediate integration is requested, still provide the matching production texture stack above.

### 3.6 Atlas rules

Production atlas rules:

- atlas max: `2048 x 2048`
- preferred atlas size: `1024 x 1024`
- padding between sprites: `4 px`
- outer padding: `4 px`
- no touching sprites

### 3.7 Pivot rules

- floors and terrain: tile center
- walls and doors: center-bottom of occupied tile
- characters: center-bottom
- props: center of gameplay footprint unless documented otherwise
- ships and large modules: documented per prefab

### 3.8 Readability test rules

No asset is approved without checks at:

- native gameplay size
- crowded scene composition
- dark-background contrast
- light-background contrast
- top-down gameplay camera

## 4. Two Asset Lanes

## 4.1 Lane A - Immediate Runtime Integration

This is for art that must drop into the current client immediately.

Current hard constraint:

- canonical active atlas: `assets/textures/runtime/active/world_atlas.bmp`
- compatibility alias retained: `assets/textures/world_atlas.bmp`
- size: `320 x 128`
- grid: `5 x 2`
- cell size: `64 x 64`

Current slot map:

Row 1:

1. Industrial Floor
2. Sterile Floor
3. Emergency Floor
4. Industrial Wall
5. Sterile Wall

Row 2:

1. Emergency Wall
2. Player
3. Crate
4. Terminal
5. Locker

Important note:

This current atlas is an interim runtime pack, not the final production target.
It is a small compatibility lane for the current code.

If an asset is requested for Lane A, also deliver the matching production source exports:

- `*_base`
- `*_mask`
- `*_fx`
- `*_ui`

If the asset is inventory-bearing, also deliver the full matching inventory icon pack.

### 4.2 Lane B - Production Source Art

This is the real art-production lane.

It should follow the full measurements below even if the immediate runtime does not yet support every asset family at final fidelity.

Do not cripple source art to match the current placeholder atlas.

Lane B assumes the multi-map production stack from section `3.4`.
Do not treat single-flat-image delivery as production complete unless the asset is explicitly approved as an exception.

## 5. Exact Asset Measurements

### 5.1 Terrain and floors

- gameplay footprint: `48 x 48`
- native export: `48 x 48`
- source size: `96 x 96`

Use for:

- exterior ground
- interior floors
- landing pads
- hazard ground
- floor overlays

### 5.2 Walls, doors, hatches

- gameplay footprint: `1 tile`
- native export canvas: `48 x 96`
- source size: `96 x 192`
- pivot: center-bottom

Use for:

- interior walls
- bulkheads
- blast doors
- airlocks
- hatches
- exterior wall segments

### 5.3 Characters

- gameplay footprint: `1 tile`
- native export canvas: `64 x 96`
- source size: `128 x 192`
- pivot: center-bottom
- facings: `8 directions`

### 5.4 Small props

- gameplay footprint: usually `1 tile`
- native export canvas: `48 x 48`
- source size: `96 x 96`

Use for:

- crates
- small lockers
- valves
- access panels
- portable tools

### 5.5 Medium props

- native export canvas: `96 x 96`
- source size: `192 x 192`

Use for:

- consoles
- workbenches
- medical carts
- command tables
- cargo trolleys

### 5.6 Tall props

- native export canvas: `96 x 144`
- source size: `192 x 288`

Use for:

- machinery stacks
- relay structures
- storage towers
- mast equipment

### 5.7 Small drones

- native export canvas: `48 x 48`
- source size: `96 x 96`
- facings: `8`

### 5.8 Medium drones and turrets

- native export canvas: `64 x 64`
- source size: `128 x 128`
- facings: `8`

### 5.9 Shuttle exterior

- components: `96 x 96`
- larger hull chunks: `128 x 128`
- full top-down shuttle profile: `256 x 256`
- source files at `2x`

### 5.10 Orbital objects

- small: `64 x 64`
- medium: `96 x 96`
- large platform chunks: `128 x 128`
- background modules: `512 x 512`

### 5.11 Icons

Every inventory-bearing item must include a matching icon pack.

Locked inventory icon target:

- native icon export: `64 x 64`
- source icon size: `128 x 128`

Required icon files:

- `*_icon_base.png`
- `*_icon_mask.png`
- `*_icon_fx.png`
- `*_icon_ui.png`

For non-inventory UI iconography:

- small UI icons: `32 x 32`
- large UI icons: `64 x 64`

Do not assume a world sprite is an acceptable substitute for an inventory icon.

### 5.12 Portraits

- runtime portrait: `256 x 256`
- source portrait: `512 x 512`

## 6. Current Slice Environment Identity

These environments must not blur together.

### 6.1 Cargo Bay

Read as:

- storage-heavy
- palletized
- mechanical
- loading and tie-down logic

Must include:

- cargo markings
- pallet and crate logic
- service rails
- dock wear
- boarding hardware

### 6.2 Transit Spine

Read as:

- route circulation
- service movement
- wayfinding
- corridor flow

Must include:

- directional markings
- conduit access
- route terminal identity
- circulation-read materials

### 6.3 MedLab

Read as:

- clinical but frontier-grade
- diagnostic
- decon-aware

Must include:

- sterile floor and panel language
- diagnostic equipment
- med storage
- subtle medical accent color

### 6.4 Command Deck

Read as:

- restricted
- coordinated
- control-oriented

Must include:

- stronger console density
- secure storage
- authority-oriented materials
- comms and data surfaces

### 6.5 Hazard Containment

Read as:

- dangerous
- sealed
- response-oriented

Must include:

- hazard markings
- containment storage
- damaged and contaminated states
- response gear logic

### 6.6 Responder Shuttle Khepri

Read as:

- compact
- serviceable
- dependable
- industrial aerospace, not hero ship

Must include:

- hull panel logic
- docking collar logic
- shuttle helm and nav console identity
- life-support and safety cues

### 6.7 Orbital Route Layer

Read as:

- controlled traffic space
- route discipline
- sparse but readable navigation logic

Must include:

- route arrows
- destination pings
- debris and transfer markers
- relay/platform silhouettes

### 6.8 Dust Frontier

Read as:

- remote relay outpost
- exposed, patched, utilitarian
- sparse but purposeful

Must include:

- landing pad logic
- relay mast or tower logic
- scaffold and cable logic
- emergency generator cues
- frontier signage and numbers

## 7. Mandatory Asset State Rules

If an object changes state in gameplay, its art must support the correct states.

### 7.1 Core state list

- default
- highlighted
- active
- inactive
- damaged
- repaired
- objective-active

### 7.2 If lockable

- locked
- unlocked

### 7.3 If openable

- closed
- open

### 7.4 If exhaustible

- used
- exhausted

## 8. Priority Delivery Packs

## 8.1 Pack 00 - Immediate Runtime Atlas Replacement

Purpose:

- replace the current placeholder atlas without waiting on pipeline expansion

Deliver exactly:

- `runtime/active/world_atlas.bmp`
- compatibility alias: `world_atlas.bmp`
- size: `320 x 128`
- grid: `5 x 2`
- `64 x 64` cells

Required cells:

1. Industrial Floor
2. Sterile Floor
3. Emergency Floor
4. Industrial Wall
5. Sterile Wall
6. Emergency Wall
7. Player
8. Crate
9. Terminal
10. Locker

Notes:

- this pack is simplified and compatibility-focused
- it does not replace the larger production source-art packs
- wall cells here are flat top-down runtime materials, not full `48 x 96` wall-canvas delivery

## 8.2 Pack 01 - Planetary Slice Identity Kit

Purpose:

- make Khepri Dock visually authored enough for real playtests

Deliver:

- Cargo Bay floor set
- Transit Spine floor set
- MedLab floor set
- Command Deck floor set
- Hazard Containment floor set
- one facility wall family
- one door/hatch family
- crate, locker, terminal, command-console, diagnostic-station variants
- hazard overlays for vacuum, radiation, toxic, electrical

## 8.3 Pack 02 - Character Readability Kit

Purpose:

- make player, ally, and hostile silhouettes readable in the current slice

Deliver:

- modular base character kit
- one worker/service look
- one security look
- one hostile scavenger/raider look
- one pilot/ship operator look
- one wounded civilian/survivor look

Minimum animation set for this pack:

- idle
- walk
- interact/use
- inspect
- hit react
- death/downed

## 8.4 Pack 03 - Ship, Orbital, And Return-Loop Kit

Purpose:

- support M42-M44 continuity visually

Deliver:

- shuttle interior prop kit
- shuttle exterior top-down profile
- orbital route markers
- docking marker set
- Dust Frontier landing pad visuals
- relay mast / beacon / frontier support props

## 8.5 Pack 04 - MUD-Feel UI Support Kit

Purpose:

- support the text-first identity in the graphical client

Deliver:

- location title treatment
- room description panel framing
- event-log framing support
- mission/journal framing support
- prompt/vitals strip treatment
- command-bar presentation treatment
- cursor states
- objective markers

## 9. Naming Convention

Use this naming format:

`WAR_[category]_[set]_[object]_[state]_[dir]_[frame]_[maptype]`

Examples:

- `WAR_char_security_torso_default_N_01_base`
- `WAR_char_security_torso_default_N_01_mask`
- `WAR_prop_terminal_diagnostic_active_fx`
- `WAR_tile_floor_medlab_fill_A_base`
- `WAR_ui_prompt_strip_default_ui`

## 10. Definition Of Done

No asset is approved unless it includes:

- source file
- review PNG
- the exact required output pack from `Docs/Art/Wrath and Ruin - Art Asset Delivery Contract.md`
- matching inventory icon pack if the asset is inventory-bearing
- engine-ready BMP if requested for integration
- preview sheet
- manifest or manifest entry covering the asset outputs
- pivot documented
- footprint documented
- correct state variants
- readability check at native gameplay scale
- crowded-scene readability check

## 11. What The Lead Artist Must Not Do

Do not:

- redesign the project into isometric art
- build wide cinematic facades that break tactical readability
- start with a giant “full game” asset wish list
- deliver style-only art with no measurements or pivots
- deliver only a flat color export when the production texture stack is required
- ship runtime art without engine-ready BMP when integration is requested
- treat text surfaces as someone else’s problem

## 12. First Response Expected From The Lead Artist

When onboarding, the lead artist should respond with:

1. Confirmed Understanding
2. Camera And Scale Confirmation
3. Environment Identity Call
4. Pack 00 Through Pack 04 Execution Order
5. Risk Flags
6. First Deliverable Batch

Anything vague should be resolved against this document rather than improvised.
