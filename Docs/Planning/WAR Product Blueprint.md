# WAR Product Blueprint

## Product Goal

Build a clean internal-alpha vertical slice that delivers a crisp 30-minute first-session experience and proves the foundation for a level 1-10 play loop.

The alpha slice must let one player:

1. create or randomize a character quickly
2. complete a guided onboarding/tutorial area
3. learn movement, interaction, combat, survival, and UI basics
4. board and travel through a shuttle sequence
5. arrive in the Ceres live area
6. complete a short quest chain
7. use a ship, basic computer systems, and one construction workflow
8. reach a believable level-10-ready rules envelope, even if the slice only exercises a smaller subset directly

## System Spine

Use `WAR Hybrid D20` as the core rules frame:

- `5e` bounded math for readable scaling
- `3.5` style build depth for feats, classes, and progression identity
- `4e` encounter roles for combat pacing
- old-school exploration pressure for supplies, danger, and world tone

Minimum supported gameplay pillars for alpha:

- combat
- travel
- interaction
- inventory and equipment
- basic survival
- ship usage
- computer usage
- basic construction
- onboarding and questing

## Alpha Experience Definition

The first 30 minutes must feel complete, not broad.

Required beats:

1. character start and orientation
2. tutorial movement and interaction
3. first item pickup, equip, and inventory check
4. first combat encounter
5. first terminal/computer use
6. first food, water, or recovery interaction
7. shuttle boarding and travel transition
8. arrival in Ceres live area
9. one quest chain with at least one branching objective state
10. one ship prototype interaction and one basic flight action

## SMART Targets

- `Specific`: ship one authoritative tutorial-to-Ceres flow with one complete player route.
- `Measurable`: a new tester can finish the full route in `30 minutes +/- 10 minutes`.
- `Achievable`: scope is limited to one planet system, one live destination, one ship class, one starter quest chain, and one starter construction loop.
- `Relevant`: every feature added must serve onboarding, travel, combat, progression, or retention in the first session.
- `Time-bound`: every milestone has a fixed delivery window and a zip handoff requirement.

## Feature Lock For Alpha

Must be functional by alpha:

- directional multipart characters
- movement, interaction, combat, and death states
- cover, concealment, fog of war, and room readability
- inventory, equipment, character sheet, stats, class, abilities, and skills
- starter economy, shop, trainer, and recovery loop
- basic ship grid prototypes
- boarding, launch, travel, and docking transitions
- first terminal/computer gameplay
- first construction workflow
- packaged build and validation flow

Out of scope until after alpha unless it directly unblocks the slice:

- accounts at production scale
- large world breadth
- multiple star systems
- deep crafting trees
- broad faction reputation systems
- large-scale live-ops tooling beyond alpha support needs

## Quality Bar

The alpha should read as intentional and premium in the areas players see first.

Priority order:

1. tactical readability
2. responsiveness
3. world identity
4. stable packaging
5. progression clarity
6. visual richness

Renderer minimums for alpha:

- lighting
- shadows
- fog of war
- visibility memory
- normal-ready material support
- non-repetitive terrain and wall presentation
- camera zoom clamp and authored zoom bands
- basic VFX feedback for selection, danger, and combat

## Final Sign-Off Condition

The slice is ready for internal alpha only when:

- the build packages cleanly from the repo
- the tutorial-to-Ceres route is playable end-to-end
- the renderer supports the readability baseline
- the ship prototype loop works
- the lead dev signs off the delivery package
- the lead artist signs off the benchmark scenes and asset kit
