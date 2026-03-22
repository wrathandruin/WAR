# WAR Art Direction and Asset Targets

## Visual Goal

The alpha must read as high-class, readable, and intentional rather than broad.

Visual priorities:

1. room readability
2. character silhouette clarity
3. ship and station identity
4. UI legibility
5. atmosphere

## Renderer Support Required From Engineering

The art team should expect support for:

- lighting
- shadows
- fog of war
- explored-memory presentation
- directional character presentation
- material channels for base, mask, fx, and future normal support
- multi-tile and anti-repetition texture usage
- room, wall, hatch, and ship-grid readability

## Benchmark Scenes

The lead artist must approve these four benchmark scenes before alpha lock:

1. tutorial room
2. corridor and hatch transition
3. shuttle interior
4. Ceres spaceport live area

Each benchmark scene must include:

- final tile language
- lighting pass
- signage
- interaction props
- player readability check at gameplay zoom

## Asset Packages By Milestone

### 0.1.2 Renderer and Camera Baseline

- floor, wall, hatch, and corner kit
- lighting benchmark textures
- readability test scene art

### 0.1.3 Character, Combat, and Sheet Baseline

- multipart player set
- starter NPC set
- starter weapons and armor visuals
- UI shell for sheet, inventory, and equipment

### 0.1.4 Tutorial and Onboarding Slice

- tutorial rooms
- onboarding signage
- trainer, shop, and prompt assets

### 0.1.5 Shuttle and Ship Prototype Slice

- shuttle exterior
- shuttle interior
- cockpit and terminal kit

### 0.1.6 Computers, Construction, and Utility Systems

- terminal states
- construction pieces
- repair/build feedback states

### 0.1.7 Ceres Live Slice

- Ceres spaceport environment kit
- quest props
- room description and communication presentation assets

### 0.1.8 Internal Alpha Candidate

- final polish pass on tutorial, shuttle, and Ceres benchmark scenes
- final UI cleanup
- final consistency pass across materials and signage

## SMART Asset Targets

- `Specific`: every milestone asset package must map to one gameplay need.
- `Measurable`: every package must include a checklist of delivered scene pieces.
- `Achievable`: only benchmark-scene assets and first-session-critical assets are mandatory for alpha.
- `Relevant`: no asset work should be produced if it does not serve the tutorial, shuttle, Ceres, combat, ship, UI, or construction slice.
- `Time-bound`: assets are due by the milestone listed in the alpha handoff doc.

## Lead Artist Sign-Off

The lead artist signs off alpha readiness when:

- all four benchmark scenes pass readability review
- the multipart character set is approved
- the shuttle and Ceres kits are final enough for internal playtest
- the UI shell is readable at gameplay scale
