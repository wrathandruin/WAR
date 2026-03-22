# Wrath and Ruin - 0.1 Slice Execution Packages and Asset Schedule

## Purpose

This document turns the `0.1.x` vertical-slice plan into exact production instructions for the lead developer.

It exists to remove ambiguity.
Each slice below states:

- what the lead developer must do
- what must be delivered
- what must be zipped
- which assets are required
- when those assets are due
- what must be proven before sign-off

This is the operating document for the `0.1.1` through `0.1.15` runway.

Read alongside:

- `Docs/Planning/Wrath and Ruin - Alpha Vertical Slice Architecture and Milestone Blueprint.md`
- `Milestones/WAR_Strategic_Roadmap.md`
- `Docs/Standards/Wrath and Ruin - Milestone Delivery Standard.md`
- `Docs/Art/Wrath and Ruin - Lead Artist Considerations.md`
- `Docs/Art/Wrath and Ruin - Art Production and Asset Planning Track.md`

---

## 1. Global Operating Rules

The lead developer must follow these rules for every `0.1.x` slice:

1. Deliver complete files only. No patch-only handoffs.
2. Deliver one shippable slice at a time. No "half-finished next slice" code inside the current handoff.
3. Update docs, build scripts, and validation paths in the same delivery if behavior changes.
4. Stage one complete handoff zip per slice.
5. Include exact validation evidence or an explicit unverified limitation note.
6. Freeze the asset request list at the start of the slice and do not widen it casually mid-slice.
7. Do not declare sign-off if the package cannot be rebuilt, staged, and explained by another developer.

---

## 2. Handoff Package Contract

Every `0.1.x` slice must stage a handoff package under:

`out/handoffs/`

Zip naming pattern:

`WAR_0.1.x_<slice-slug>_handoff.zip`

Each zip must include:

- updated source files
- updated build or package scripts
- updated docs
- the exact slice brief
- validation notes or validation output
- asset manifest for that slice
- `handoff_manifest.txt`
- `open_items.txt` only if something is intentionally deferred

Every `handoff_manifest.txt` must state:

- slice id
- date
- branch or commit
- build configuration tested
- scripts run
- files changed
- assets integrated
- assets still pending
- next slice owner expectations

---

## 3. Default Slice Timing

Unless explicitly overridden below, every slice follows this cadence:

- Day 1 by 12:00: slice scope freeze and asset request freeze
- Day 2 by 18:00: lead artist confirmation of required asset cuts
- Day 3-4: first engineering implementation batch committed locally
- Day 4-5: first-pass art delivery or placeholder confirmation
- Day 6-7: integration-ready engineering baseline
- Day 7-8: integration-ready art exports
- Final 2 days: validation, docs, zip handoff, sign-off review

If a slice is two weeks long, use the same cadence twice:

- week 1 for first-pass implementation
- week 2 for hardening, integration, and sign-off

---

## 4. Master Calendar

Recommended calendar:

- Week 1: `0.1.1`
- Weeks 2-3: `0.1.2`
- Weeks 4-5: `0.1.3`
- Week 6: `0.1.4`
- Weeks 7-8: `0.1.5`
- Week 9: `0.1.6`
- Week 10: `0.1.7`
- Week 11: `0.1.8`
- Week 12: `0.1.9`
- Week 13: `0.1.10`
- Weeks 14-15: `0.1.11`
- Weeks 16-17: `0.1.12`
- Weeks 18-19: `0.1.13`
- Weeks 20-21: `0.1.14`
- Week 22: `0.1.15`

Target after Week 22:

- `0.2.0 Internal Alpha Candidate`

---

## 5. Slice Packages

### 5.1 Slice `0.1.1` - Alpha Lock / Repo Hygiene / Delivery Contract

Window:

- Week 1

Lead developer must:

- freeze the playable alpha scope
- freeze the level 1 through 10 rules target
- freeze class list, core verbs, and scene list
- freeze benchmark scenes for the lead artist
- clean the repo root and remove obvious path clutter
- consolidate source manifests into `SourceManifests/`
- write or update the root README and execution documents
- define the exact handoff zip contract for all future slices
- document the current root ownership map

Required code and repo actions:

- verify all runtime manifest lookups support the consolidated source layout
- verify packaging scripts still stage the historical package layout
- remove no production folders that current builds depend on
- keep localhost and package validation truthful

Required docs:

- root `README.md`
- `Docs/Planning/Wrath and Ruin - Alpha Vertical Slice Architecture and Milestone Blueprint.md`
- `Docs/Planning/Wrath and Ruin - 0.1 Slice Execution Packages and Asset Schedule.md`
- `Docs/Standards/Wrath and Ruin - Milestone Delivery Standard.md`
- lead developer prompt for `0.1.1`

Required zip:

- `WAR_0.1.1_alpha_lock_handoff.zip`

Required assets:

- benchmark scene mood board
- Ceres palette sheet
- character rig proportion sheet
- door, hatch, wall language board
- terminal and ship silhouette board

Asset deadlines:

- Day 1 18:00: asset brief issued
- Day 2 18:00: benchmark scene list confirmed
- Day 4 18:00: mood board and palette first pass
- Day 5 18:00: benchmark scene direction locked

Exit gate:

- repo layout cleanup is in place
- source manifest reorganization is complete
- exact execution docs exist
- lead dev and lead artist have concrete sign-off checklists

### 5.2 Slice `0.1.2` - Renderer Foundation

Window:

- Weeks 2-3

Lead developer must:

- tighten min and max zoom rules
- implement camera context bands for world, interior, and flight views
- add layered material support inputs
- add dynamic local light support
- add ambient shadow or contact shadow support
- add fog-of-war and explored-memory support
- add global texture or macro-variation support
- add render debug controls for visibility and lighting review
- update validation notes with performance budgets and fallback behavior

Required code areas:

- camera
- bgfx world renderer
- shader program loading
- tile material data
- world render debug overlay

Required zip:

- `WAR_0.1.2_renderer_foundation_handoff.zip`

Required assets:

- lighting reference set
- floor macro textures
- wall material test sheet
- hazard overlay prototypes
- shadow and AO calibration board

Asset deadlines:

- Week 2 Day 2: lighting and material brief frozen
- Week 2 Day 4: first-pass material test exports
- Week 3 Day 2: integration-ready macro textures and overlay masks
- Week 3 Day 4: final alpha-blocking render test assets

Exit gate:

- camera no longer permits unreadable zoom states
- one benchmark scene proves lights, shadowing, fog-of-war, and material variation together
- renderer performance remains within agreed budget on the test hardware

### 5.3 Slice `0.1.3` - Character Runtime / Modular Composition / Animation Manager

Window:

- Weeks 4-5

Lead developer must:

- introduce the multipart character runtime
- support legs, torso, head, and gear layers
- create the animation manager state graph
- support facing and directional transitions
- support idle, walk, interact, attack, hit, and downed states
- hook equipment state into silhouette changes
- hook obscurity and identity overlays into rendering
- expose animation diagnostics for quick review

Required code areas:

- entity render proxy
- character presentation runtime
- animation state machine
- equipment-to-visual hook layer

Required zip:

- `WAR_0.1.3_character_runtime_handoff.zip`

Required assets:

- base bodies
- head pack
- leg pack
- torso gear pack
- helmet and face gear pack
- animation timing sheet

Asset deadlines:

- Week 4 Day 2: rig alignment and sheet layout confirmed
- Week 4 Day 5: idle and walk first pass
- Week 5 Day 2: interact and attack first pass
- Week 5 Day 4: final alpha-blocking modular set for starter archetypes

Exit gate:

- player and at least two NPC archetypes render as multipart characters
- facing is readable at gameplay zoom
- all core animation states transition correctly in play

### 5.4 Slice `0.1.4` - Session UI Shell / Text Presentation / Character Surfaces

Window:

- Week 6

Lead developer must:

- implement the baseline UI shell
- add the bottom vitals and hotkey strip
- add the left-side popout button rail
- add the upper-right room description panel
- add the lower-right communications and event panel
- add the character sheet, inventory, and equipment page shells
- ensure text hierarchy is readable and not debug-like

Required code areas:

- application UI shell
- game layer UI
- command and event log surfaces
- character page routing

Required zip:

- `WAR_0.1.4_session_ui_shell_handoff.zip`

Required assets:

- vitals strip presentation kit
- command bar treatment
- room description panel treatment
- communications panel treatment
- inventory and equipment icon frame set

Asset deadlines:

- Day 2: UI wireframe lock
- Day 4: first-pass UI kit
- Day 5: integration-ready UI surfaces

Exit gate:

- the game is readable as a product surface
- the text layer feels first-class
- inventory, equipment, and room-description shells are visible and usable

### 5.5 Slice `0.1.5` - Onboarding Academy

Window:

- Weeks 7-8

Lead developer must:

- build the disconnected newbie academy
- script the tutorial progression controller
- add guided movement and interaction lessons
- add help-file hooks and greeting NPCs
- add the terminal tutorial
- add food, water, and med tutorials
- add trainer and shop onboarding steps
- enforce one-way exit from the newbie area

Required code areas:

- onboarding mission flow
- tutorial trigger system
- help and hint surfacing
- onboarding NPC and terminal behaviors

Required zip:

- `WAR_0.1.5_onboarding_academy_handoff.zip`

Required assets:

- onboarding floor and wall set
- academy signage
- tutorial terminal set
- trainer NPC look set
- shop kiosk set

Asset deadlines:

- Week 7 Day 2: academy room list and asset request freeze
- Week 7 Day 5: first-pass room kit and signage
- Week 8 Day 2: terminal, trainer, and kiosk integration-ready set
- Week 8 Day 4: final academy polish pass

Exit gate:

- a new player can reach the academy exit without outside instruction
- the academy teaches the intended first-session verbs
- the academy cannot be re-entered after departure

### 5.6 Slice `0.1.6` - Combat / Cover / Death Introduction

Window:

- Week 9

Lead developer must:

- finalize starting combat verbs
- tighten cover and concealment behavior
- add one training fight and one real onboarding fight
- implement downed and death introduction rules
- add combat reward, loot, and recovery hooks
- ensure the combat log is readable

Required code areas:

- combat runtime
- cover evaluation
- status effect and death handling
- combat UI and log presentation

Required zip:

- `WAR_0.1.6_combat_intro_handoff.zip`

Required assets:

- onboarding enemy silhouettes
- combat impact VFX
- damage and downed indicators
- cover marker overlays if needed

Asset deadlines:

- Day 2: enemy and VFX brief frozen
- Day 4: first-pass enemy and impact VFX
- Day 5: final alpha-blocking combat readability assets

Exit gate:

- the training fight is teachable
- the real fight is readable and fair
- death or downed consequences are visible and understandable

### 5.7 Slice `0.1.7` - World Interaction / Life Support / Early Economy

Window:

- Week 10

Lead developer must:

- implement shop and vendor flow
- implement buy, sell, list, and inventory review
- implement food and water use
- implement trainer and practice flow
- add language or access gating where appropriate
- make the Ceres starter hub feel habitable

Required code areas:

- vendor runtime
- inventory transaction flow
- needs and consumable use
- trainer interaction flow

Required zip:

- `WAR_0.1.7_world_interaction_handoff.zip`

Required assets:

- vending and merchant props
- food and water props
- med props
- hub signage
- interaction highlight set

Asset deadlines:

- Day 2: prop and hub dressing brief frozen
- Day 4: first-pass prop set
- Day 5: integration-ready hub readability pass

Exit gate:

- players can stock up, recover, and prepare for the next loop
- the Ceres starter hub reads as a real place

### 5.8 Slice `0.1.8` - Character Sheet / Equipment / Progression Through Level 5

Window:

- Week 11

Lead developer must:

- implement character sheet page details
- implement inventory and equipment page function
- implement level-up flow
- implement class feature unlocks through level 5
- implement feat hooks through level 5
- tune level 1 through 5 rewards and pacing

Required code areas:

- character data model
- progression runtime
- class and feat lookup data
- UI pages for sheet, equipment, and progression

Required zip:

- `WAR_0.1.8_progression_l1_l5_handoff.zip`

Required assets:

- equipment icons
- item rarity or role markers
- sheet stat iconography
- class insignia set

Asset deadlines:

- Day 2: icon list frozen
- Day 4: first-pass icon and sheet marker set
- Day 5: integration-ready progression UI set

Exit gate:

- level 1 through 5 feels real and testable
- character identity is visible in both numbers and presentation

### 5.9 Slice `0.1.9` - Computers / Security / Terminal Gameplay

Window:

- Week 12

Lead developer must:

- implement terminal access tiers
- implement terminal action menus or command actions
- implement quest-gated computer tasks
- implement alarm or failure states
- connect computers to ship and mission state
- make computer use readable in UI and event text

Required code areas:

- terminal runtime
- access and permission rules
- mission hooks
- terminal UI and text formatting

Required zip:

- `WAR_0.1.9_computer_gameplay_handoff.zip`

Required assets:

- terminal state variants
- alarm state overlays
- access-level iconography
- terminal interaction VFX

Asset deadlines:

- Day 2: terminal state list frozen
- Day 4: first-pass terminal states
- Day 5: integration-ready alarm and use-state set

Exit gate:

- computers gate meaningful gameplay
- terminals are readable and feel like a first-class verb

### 5.10 Slice `0.1.10` - Construction / Repair Basics

Window:

- Week 13

Lead developer must:

- implement basic repair action flow
- implement simple placement or replacement actions
- implement material use or cost
- implement ownership and permission checks
- persist repair and build state
- add one onboarding repair and one field repair task

Required code areas:

- construction runtime
- repair interactions
- material consumption
- persistence hooks

Required zip:

- `WAR_0.1.10_construction_basics_handoff.zip`

Required assets:

- broken-state props
- repaired-state props
- placement ghost or preview visuals
- tool-use VFX

Asset deadlines:

- Day 2: repair state list frozen
- Day 4: broken and repaired first pass
- Day 5: integration-ready build-preview visuals

Exit gate:

- the player can change real world or ship state through repair or build actions
- construction is no longer theoretical

### 5.11 Slice `0.1.11` - Primary Ship Grid / Ship Prototype Runtime

Window:

- Weeks 14-15

Lead developer must:

- implement the ship-grid authoring format
- implement room adjacency and hatch logic
- implement cockpit, engineering, cargo, and entry nodes
- implement interior occlusion and cutaway rules
- implement ship interactables inside the grid
- build the tutorial shuttle final interior
- build the first main prototype ship

Required code areas:

- ship interior data format
- ship interaction runtime
- door and hatch state logic
- ship interior rendering and visibility rules

Required zip:

- `WAR_0.1.11_ship_grid_handoff.zip`

Required assets:

- ship interior floor and wall kit
- hatch and airlock kit
- cockpit kit
- engineering room kit
- cargo room kit

Asset deadlines:

- Week 14 Day 2: ship-room list and prototype cut lock
- Week 14 Day 5: shuttle and ship room first pass
- Week 15 Day 2: cockpit and engineering integration-ready set
- Week 15 Day 4: final prototype ship pass

Exit gate:

- the primary ship grid is playable
- ship spaces feel functionally and visually distinct

### 5.12 Slice `0.1.12` - Flight / Navigation / Docking

Window:

- Weeks 16-17

Lead developer must:

- implement the first pilot control loop
- implement nav computer flow
- implement launch and land transitions
- implement docking permissions and route state
- implement one Ceres departure and one return lane
- add ship condition feedback during travel

Required code areas:

- piloting runtime
- navigation and route state
- travel transitions
- docking logic
- flight UI and feedback

Required zip:

- `WAR_0.1.12_flight_and_docking_handoff.zip`

Required assets:

- orbital route markers
- docking cues
- thruster or motion FX
- flight HUD treatment

Asset deadlines:

- Week 16 Day 2: flight readability brief frozen
- Week 16 Day 5: route marker and HUD first pass
- Week 17 Day 2: integration-ready docking visuals
- Week 17 Day 4: final travel readability pass

Exit gate:

- launch, travel, docking, and return all function
- flight feels like gameplay rather than disguised fast travel

### 5.13 Slice `0.1.13` - Ceres Quest Arc / Level 6-10 Content

Window:

- Weeks 18-19

Lead developer must:

- complete the midgame quest arc
- tune level 6 through 10 class progression
- implement mid-tier rewards and encounters
- add one higher-skill computer task
- add one higher-skill repair or construction task
- add one stronger ship or travel responsibility beat

Required code areas:

- quest progression
- reward pacing
- level 6 through 10 features
- encounter tuning

Required zip:

- `WAR_0.1.13_ceres_arc_l6_l10_handoff.zip`

Required assets:

- midgame enemy set
- mid-tier gear set
- quest prop set
- destination identity expansion

Asset deadlines:

- Week 18 Day 2: quest prop and enemy list frozen
- Week 18 Day 5: first-pass midgame art set
- Week 19 Day 2: integration-ready quest and gear set
- Week 19 Day 4: final Ceres arc polish pass

Exit gate:

- the game supports real play beyond tutorial promise
- level 10 is structurally credible

### 5.14 Slice `0.1.14` - Hosted Alpha Ops / Multiplayer Proof

Window:

- Weeks 20-21

Lead developer must:

- validate remote transport and reconnect
- validate session identity and trust boundaries
- validate crash capture and operator triage
- validate shared presence, communications, and same-space awareness
- validate support and live-ops evidence generation
- produce an internal hosted playtest rehearsal package

Required code areas:

- remote session flow
- reconnect and resume
- host diagnostics
- presence and communication
- support and live-ops paths

Required zip:

- `WAR_0.1.14_hosted_alpha_ops_handoff.zip`

Required assets:

- multiplayer state indicators
- social presence readability pass
- hosted diagnostics dashboard art pass if needed

Asset deadlines:

- Week 20 Day 2: multiplayer readability brief frozen
- Week 20 Day 5: first-pass presence and indicator set
- Week 21 Day 3: integration-ready hosted playtest UI pass
- Week 21 Day 4: final hosted ops readability pass

Exit gate:

- a small hosted internal cohort can connect, play, recover, and be supported
- failure states are diagnosable

### 5.15 Slice `0.1.15` - Alpha Polish / Art Lock / Audio Lock / Rehearsal

Window:

- Week 22

Lead developer must:

- stop widening scope
- remove remaining alpha-blocking placeholders
- run full regression and packaging validation
- complete performance and input cleanup
- complete room-description and journal coverage review
- prepare the internal alpha candidate handoff

Required code areas:

- bug fixing
- optimization
- packaging and validation
- onboarding and continuity cleanup

Required zip:

- `WAR_0.1.15_alpha_polish_handoff.zip`

Required assets:

- benchmark scene final pass
- remaining high-priority placeholders replaced
- final VFX and atmosphere pass
- final audio ambience and interaction pass

Asset deadlines:

- Day 2: final replacement list frozen
- Day 4: last alpha-blocking art and audio drop
- Day 5: integrated alpha candidate review

Exit gate:

- the team can run the first 30-minute rehearsal cleanly
- the level 10 validation path is present
- lead dev and lead artist are both comfortable signing the candidate

---

## 6. Asset Ownership By Slice

The lead developer must open an asset request board for each slice and include:

- asset id
- slice id
- benchmark scene affected
- gameplay dependency
- placeholder allowed yes or no
- due date
- owner
- integration status

No slice may close if alpha-blocking assets still have undefined owners.

---

## 7. Required Validation At Every Slice Close

Each slice closeout must state:

- what built
- what ran
- what scripts were used
- what gameplay proof was observed
- what art proof was observed
- what remains intentionally unverified

If a slice cannot prove its own success, it is not ready to hand off.

---

## 8. Final Rule

The lead developer is not allowed to hand off:

- partial systems
- partial files
- undocumented scope drift
- hand-edited local state with no manifest
- or code that is not zip-packaged for review

Every `0.1.x` slice must leave the repo cleaner, more truthful, and more shippable than it was at the start of the slice.
