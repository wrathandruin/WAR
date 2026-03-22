# WAR Alpha Handoff

This document replaces the previous milestone stack. It is the single execution handoff for the lead developer.

## Delivery Rules

Every milestone must ship as one complete zip.

Required package format:

- zip name: `WAR_<milestone>_<slug>.zip`
- must include full source changes, no partial code
- must include validation evidence
- must include updated asset requests
- must include a short closeout note with completed work, known risks, and next-step blockers

No milestone is complete if:

- the repo does not build
- the packaged flow is not validated
- the assets requested are vague
- the handoff cannot be opened and understood in one pass

## Milestone Plan

### 0.1.1 Repo and Build Baseline

Objective:

- lock the repo structure around `WarEngine`, `Desktop`, `Server`, `Project`, `Assets`, `Docs`, `Tools`, and `Bin64`

Lead dev actions:

1. ensure all source formerly split between root `src` and `shared` lives under `WarEngine`
2. ensure Visual Studio and CMake build from the new paths
3. ensure all build outputs publish to `Bin64`
4. ensure repo-owned automation runs from `Project/scripts`
5. remove stale path assumptions from runtime diagnostics and packaging
6. confirm the root README matches the actual folder structure

Assets due:

- none beyond keeping current atlas and shader set valid

Acceptance:

- `WAR.sln` builds
- desktop and server outputs land in `Bin64`
- packaging scripts resolve the new repo layout

Timebox:

- `3 working days`

### 0.1.2 Renderer and Camera Baseline

Objective:

- make the game readable and premium at first glance

Lead dev actions:

1. lock camera zoom minimum and maximum bands
2. add fog of war and explored-memory presentation
3. add light/shadow support sufficient for room, hallway, and exterior readability
4. prepare material handling for base, mask, fx, and normal-ready expansion
5. add anti-repetition support for large shared textures and multi-tile surfaces
6. ensure walls, doors, hatches, corners, and T-junctions read cleanly

Assets due:

- wall and hatch base kit
- floor and room benchmark surfaces
- lighting benchmark scene textures

Acceptance:

- one benchmark room and one corridor scene render cleanly
- max zoom-out is capped and readable
- visual repetition is visibly reduced

Timebox:

- `7 working days`

### 0.1.3 Character, Combat, and Sheet Baseline

Objective:

- ship the first complete playable character

Lead dev actions:

1. implement multipart character rendering for legs, body, and head
2. add directional facing and idle, walk, interact, combat, hit, and death states
3. lock the hybrid D20 stat spine for level `1-10`
4. implement character sheet, inventory, and equipment views
5. implement first-pass combat with cover, concealment, and readability feedback
6. improve pathfinding so movement stops feeling mechanical

Assets due:

- starter body part set
- starter weapons and armor visuals
- UI panels for sheet, inventory, and equipment

Acceptance:

- one character can move, fight, equip, and display full core sheet data
- one encounter can be completed cleanly

Timebox:

- `7 working days`

### 0.1.4 Tutorial and Onboarding Slice

Objective:

- make the first ten minutes understandable without outside explanation

Lead dev actions:

1. implement the new tutorial route
2. add tutorial prompts, help surfaces, and onboarding NPC guidance
3. add first food, water, recovery, and death lesson
4. add first shop and first trainer interaction
5. add first quest journal and objective tracker

Assets due:

- tutorial room kit
- onboarding UI surfaces
- trainer, shop, and prompt assets

Acceptance:

- a new tester can finish onboarding and understand the next objective

Timebox:

- `5 working days`

### 0.1.5 Shuttle and Ship Prototype Slice

Objective:

- prove the first ship-grid and travel loop

Lead dev actions:

1. implement first primary grid ship prototype
2. support boarding, interior traversal, and launch transition
3. implement one basic flight action and one docking or arrival action
4. wire ship state persistence between tutorial exit and live-area arrival
5. add first ship terminal/computer interaction

Assets due:

- shuttle exterior
- shuttle interior kit
- starter cockpit and terminal presentation

Acceptance:

- the player can board, travel, and arrive through a believable ship sequence

Timebox:

- `6 working days`

### 0.1.6 Computers, Construction, and Utility Systems

Objective:

- prove non-combat player verbs

Lead dev actions:

1. implement first terminal/computer usage loop
2. implement first basic construction workflow
3. support one place, repair, or build interaction on a ship or local environment
4. connect computer and construction actions to inventory and state changes
5. expose failure and success feedback clearly in the UI

Assets due:

- terminal kit
- construction object kit
- repair/build state visuals

Acceptance:

- one computer task and one construction task can be completed end-to-end

Timebox:

- `5 working days`

### 0.1.7 Ceres Live Slice

Objective:

- land the player in the real game and let them complete a small but complete loop

Lead dev actions:

1. implement the Ceres arrival spaceport slice
2. add one complete quest chain
3. add one return loop between local area, ship use, and quest completion
4. ensure persistence survives the full route
5. polish room descriptions, communications, and objective readability

Assets due:

- Ceres spaceport exterior and interior kit
- quest-critical props and signage
- communications and room-description UI assets

Acceptance:

- the player can complete the tutorial-to-Ceres route in one sitting

Timebox:

- `7 working days`

### 0.1.8 Internal Alpha Candidate

Objective:

- stabilize and package the slice for internal use

Lead dev actions:

1. finish packaging and validation automation
2. remove dead paths and stale docs
3. produce the final alpha candidate zip
4. attach validation evidence and known-risk list
5. secure lead dev and lead artist sign-off

Assets due:

- final benchmark-scene art lock
- final UI polish pass
- final ship/tutorial/Ceres presentation lock

Acceptance:

- internal-alpha package builds, stages, launches, and validates cleanly

Timebox:

- `4 working days`

## Zip Handoff Checklist

Every milestone zip must contain:

- source delta
- packaged build or package path note
- validation report
- asset request sheet
- closeout note

## Lead Dev Prompt

You are the lead developer for WAR. Work only against the current milestone in this document. Deliver one complete zip per milestone with no half-finished code, no hidden follow-up work, and no stale paths. Build from the current repo layout, treat `WarEngine` as the source root, publish binaries to `Bin64`, use `Project/scripts` for automation, and keep docs concise. Your first response for every milestone must state the exact tasks you are executing, the files or folders you own, the validation you will run, and the zip name you intend to ship.
