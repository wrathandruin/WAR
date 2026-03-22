# Wrath and Ruin - Lead Artist Milestone Runway

## Purpose

This document is the exact milestone runway for the lead artist.

Before this document, the project had:

- locked art direction
- locked measurements and export rules
- priority asset packs
- a production track

What it did not yet have was a true artist execution runway in milestone form.

This document fixes that.

Use it as the working art schedule for the alpha slice.

## Source Of Truth Order

Read and obey these documents in this order:

1. `Docs/Art/Wrath and Ruin - Lead Artist Onboarding Package.md`
2. `Docs/Art/Wrath and Ruin - Art Asset Delivery Contract.md`
3. `Docs/Art/Wrath and Ruin - Lore and Visual Theme Guide.md`
4. `Docs/Art/Wrath and Ruin - Art Audit and Direction Lock.md`
5. `Docs/Art/Wrath and Ruin - Art Production and Asset Planning Track.md`
6. `Docs/Prompts/ChatGPT Lead Artist Prompt.md`
7. `Docs/Planning/Wrath and Ruin - M45-M70 Alpha Playtest Runway.md`

If there is a conflict, the onboarding package wins.

## Non-Negotiable Rules

- The game is top-down tactical, not isometric.
- The art must read like an overhead tactical game with grounded hard-sci-fi materials.
- The project is a graphical MUD, so text-facing presentation is first-class art work, not side work.
- Immediate runtime-integration requests must respect the current BMP-backed lane.
- Production source art must not be crippled to placeholder atlas limits.
- Every asset family must have explicit size, footprint, pivot, and state rules.
- No asset request is complete unless it is tied to a milestone or validation need.

## Locked Technical Standards

Use the locked measurements from the onboarding package.

The most important current standards are:

- gameplay tile footprint: `48 x 48`
- walls and doors native canvas: `48 x 96`
- characters native canvas: `64 x 96`
- small props native canvas: `48 x 48`
- medium props native canvas: `96 x 96`
- tall props native canvas: `96 x 144`
- immediate runtime atlas compatibility lane: `runtime/active/world_atlas.bmp`, `320 x 128`, `5 x 2`, `64 x 64` cells

## Locked Production Texture Stack

Use the exact output rules in:

- `Docs/Art/Wrath and Ruin - Art Asset Delivery Contract.md`

Locked shorthand:

- world gameplay assets = four PNG files mandatory
- inventory-bearing items = matching icon pack mandatory
- UI surfaces = locked UI surface pack mandatory

## Definition Of Done For Every Art Milestone

No art milestone is complete unless it includes:

- exact asset list
- exact sizes
- exact gameplay footprints
- exact pivots
- required state variants
- layered source files
- review PNG exports
- required output packs exactly matching the art asset delivery contract
- engine-ready BMP exports when integration is requested
- preview or contact sheet
- manifest listing every output and whether icon coverage is included where required
- native-size readability review
- crowded-scene readability review

## How To Use This Runway

- `A` milestones are art milestones.
- They run in parallel with engineering milestones.
- The `Supports` line tells the artist which gameplay window the work is meant to strengthen.
- The lead artist should not skip ahead to late polish while early readability is still weak.
- if an asset in any milestone can appear in inventory, it must ship with a matching inventory icon pack in the same milestone batch

---

## A00 - Runtime Atlas Compatibility Baseline

**Supports:** `M45` onward immediately

**Goal**

Replace the current placeholder compatibility atlas with an intentional baseline set that is safe for immediate runtime use.

**Required outputs**

- `world_atlas.bmp`
- canonical location: `runtime/active/world_atlas.bmp`
- `world_atlas_base.png`
- `world_atlas_mask.png`
- size: `320 x 128`
- grid: `5 x 2`
- cell size: `64 x 64`

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

Optional but recommended if the atlas is also being used to prototype visibility support:

- `world_atlas_ui.png`

**Exit criteria**

- the current runtime atlas is replaced cleanly
- all ten cells are visually intentional and top-down safe
- the player, floors, walls, and key props are more readable than the placeholder pack

---

## A01 - Planetary Environment Identity Set

**Supports:** `M45-M48`

**Goal**

Make the first planetary slice feel authored instead of like one repeated industrial room set.

**Required outputs**

- Cargo Bay floor family, `48 x 48`
- Transit Spine floor family, `48 x 48`
- MedLab floor family, `48 x 48`
- Command Deck floor family, `48 x 48`
- Hazard Containment floor family, `48 x 48`
- one shared wall family, `48 x 96`
- one door or hatch family, `48 x 96`
- one landing-pad surface family, `48 x 48`

**Required prop families**

- crate variants, `48 x 48`
- locker variants, `48 x 48`
- terminal variants, `48 x 48`
- command console variants, `96 x 96`
- diagnostic station variants, `96 x 96`

**Exit criteria**

- all five core interior spaces are visually distinct at gameplay zoom
- screenshots of the planetary slice no longer read like the same room with different labels

---

## A02 - Interactable And Hazard Readability Kit

**Supports:** `M45-M49`

**Goal**

Make important objects and environmental danger readable without relying on debug interpretation.

**Required outputs**

- vacuum hazard overlay, `48 x 48`
- radiation hazard overlay, `48 x 48`
- toxic hazard overlay, `48 x 48`
- electrical hazard overlay, `48 x 48`
- powered terminal state family, `48 x 48`
- inactive terminal state family, `48 x 48`
- damaged terminal state family, `48 x 48`
- locked container state family, `48 x 48`
- opened container state family, `48 x 48`
- damaged prop state accents where needed

**Exit criteria**

- hazards are readable without relying entirely on text
- interactables communicate state clearly at native zoom
- hazard spaces feel risk-coded, not only mechanically tagged

---

## A03 - Character Readability And Role Silhouette Kit

**Supports:** `M45-M50`

**Goal**

Make players, allies, and hostiles readable as roles instead of anonymous placeholders.

**Required outputs**

- modular base human kit, `64 x 96`, `8 directions`
- worker or service role, `64 x 96`, `8 directions`
- security role, `64 x 96`, `8 directions`
- hostile scavenger or raider role, `64 x 96`, `8 directions`
- pilot or ship operator role, `64 x 96`, `8 directions`
- wounded civilian or survivor role, `64 x 96`, `8 directions`

**Required state support**

- idle
- walk
- interact or use
- inspect
- hit react
- death or downed

**Exit criteria**

- role silhouettes are readable at gameplay scale
- the current slice can visually distinguish ally, worker, hostile, pilot, and vulnerable civilian states

---

## A04 - Ship, Orbital, Docking, And Return-Loop Kit

**Supports:** `M42-M44` closure and `M45-M50`

**Goal**

Make ship and orbital travel spaces feel like real authored layers rather than abstract travel states.

**Required outputs**

- shuttle interior prop kit
- shuttle exterior top-down profile, recommended runtime-facing presentation blocks at `128 x 128` and `256 x 256`
- orbital route markers, `64 x 64`
- orbital node markers, `64 x 64`
- docking marker set, `64 x 64`
- landing marker set, `64 x 64`
- relay mast and beacon props, `96 x 144`
- Dust Frontier landing pad visuals, `48 x 48` and `96 x 96` support pieces

**Exit criteria**

- the player can visually read ship, orbital, docking, and landing context changes
- the return loop has a coherent visual language instead of only telemetry proof

---

## A05 - Room Description And Location Title Presentation

**Supports:** `M45-M51`

**Goal**

Support the first explicit MUD-feel requirement: entering a room should feel authored.

**Required outputs**

- location title treatment
- room description panel frame
- room description body typography guidance
- timing and transition guidance for first-entry presentation
- visual distinction from combat spam and event spam

**Required UI support pieces**

- panel frame assets
- divider treatments
- title icon support if used
- background treatment guidance

**Exit criteria**

- important rooms, interiors, ship compartments, and destination zones can present authored descriptions intentionally
- the presentation reads like product UX, not a debug text box

---

## A06 - Prompt, Event Log, And Command Bar Presentation Kit

**Supports:** `M45-M54`

**Goal**

Make the text layer feel like a core surface of the game rather than a temporary utility overlay.

**Required outputs**

- prompt or vitals strip treatment
- event-log framing support
- command-bar presentation treatment
- log category iconography if used
- cursor states
- text-surface spacing guidance

**Exit criteria**

- prompt, log, and command surfaces feel like one consistent product
- the graphical-MUD identity becomes visible even before full social playtests

---

## A07 - Mission, Objective, Journal, And Inspect Presentation Kit

**Supports:** `M45-M57`

**Goal**

Support mission readability and authored world interaction through consistent text-first presentation.

**Required outputs**

- mission panel framing
- journal framing
- objective marker family
- inspect output treatment
- success, warning, failure, and blocked-state visual treatment

**Required icon support**

- objective active
- objective complete
- objective blocked
- inspect available
- inspect exhausted

**Exit criteria**

- mission and inspect text feels authored, prioritized, and readable
- mission-critical state changes are visually clear without becoming flashy or noisy

---

## A08 - Faction Signage, Wayfinding, And Environmental Marking Kit

**Supports:** `M45-M58`

**Goal**

Translate faction identity and place identity into visible environmental language.

**Required outputs**

- UNEA signage language samples
- Martian Commonwealth signage language samples
- Belt or frontier signage language samples where relevant
- industrial warning markings
- medical markings
- command or restricted-access markings
- hazard quarantine markings
- ship stenciling samples

**Exit criteria**

- spaces communicate ownership and purpose through markings and signage
- the slice feels more like a real lived facility and less like generic sci-fi floor art

---

## A09 - Terminal, Portrait, Comms, And Text-Channel Identity Kit

**Supports:** `M52-M59`

**Goal**

Strengthen the sense that communication, terminals, logs, and inspect text belong to one world.

**Required outputs**

- terminal-screen framing direction
- communications portrait frame, runtime target `256 x 256`, source target `512 x 512`
- speaker badge styles
- dialog or transmission surface treatment
- terminal category icon set

**Exit criteria**

- terminal and communication surfaces feel authored and consistent
- text-heavy interactions feel like world systems, not flat placeholders

---

## A10 - Shared Presence And Multi-User Readability Kit

**Supports:** `M59`

**Goal**

Support the first social and cooperative playtest beats with readable player presence.

**Required outputs**

- friendly player differentiation support
- same-space role markers where needed
- small cooperation cue icon set
- speech or emote bubble direction if approved
- shared-objective readability markers

**Exit criteria**

- two players in the same space can read each other clearly
- cooperation cues do not disappear into the floor art or UI noise

---

## A11 - Accessibility, Contrast, And Input-Readability Pass

**Supports:** `M60-M62`

**Goal**

Make the art layer resilient enough for real cohort testing.

**Required outputs**

- contrast review pass on floors, props, text panels, and hazards
- color-blind-risk review on critical hazard cues
- minimum text contrast guidance
- icon simplification pass where needed
- keyboard and mouse input glyph support if requested by engineering

**Exit criteria**

- critical gameplay information is readable without color-perfect vision
- text and tactical surfaces do not visually fight each other

---

## A12 - First Session Onboarding And Help Surface Kit

**Supports:** `M63-M64`

**Goal**

Help a new player understand the product without verbal explanation.

**Required outputs**

- onboarding panel treatment
- help-tip framing
- first-session mission card treatment
- tutorial callout styling
- failure or confusion-recovery prompt styling

**Exit criteria**

- first-session guidance looks deliberate and readable
- onboarding surfaces match the rest of the product rather than feeling bolted on

---

## A13 - Distribution, Launcher, And Cohort-Facing Branding Minimum

**Supports:** `M65`

**Goal**

Provide the minimum trustworthy visual layer for build distribution and first external handling.

**Required outputs**

- launcher or installer branding placeholders if needed
- package splash or title treatment if needed
- simple alpha cohort identifier treatment
- support or issue-report branding elements if requested

**Exit criteria**

- distributed builds do not look like anonymous dev utilities
- the alpha cohort receives a coherent product-facing shell

---

## A14 - Placeholder Burn-Down And Alpha Consistency Sweep

**Supports:** `M66-M68`

**Goal**

Remove the most damaging placeholder visuals before the first real alpha playtest.

**Required outputs**

- placeholder-versus-final tracking sheet update
- high-visibility placeholder replacements
- palette and material consistency sweep
- cross-layer consistency pass between planetary, ship, orbital, and destination spaces
- room-description, prompt, and journal presentation consistency pass

**Exit criteria**

- the slice no longer feels like a strong prototype with random placeholder leftovers
- the first alpha playtest build has a credible visual identity

---

## A15 - First Alpha Cohort Triage Pack

**Supports:** `M69-M70`

**Goal**

Respond to the first real cohort with targeted art fixes instead of broad reactive churn.

**Required outputs**

- prioritized fix batch for the top readability issues
- targeted room-description presentation refinements
- targeted UI or icon fixes from tester confusion
- final pre-alpha visual polish on the highest-impact rough edges

**Exit criteria**

- art response is disciplined and evidence-led
- the product looks more deliberate after cohort feedback rather than more inconsistent

---

## Recommended Execution Order

The lead artist should normally execute in this order:

1. `A00`
2. `A01`
3. `A02`
4. `A03`
5. `A04`
6. `A05`
7. `A06`
8. `A07`
9. `A08`
10. `A09`
11. `A10`
12. `A11`
13. `A12`
14. `A13`
15. `A14`
16. `A15`

If scope must be cut, cut late branding and cohort polish before cutting:

- room-description presentation
- planetary identity
- hazard readability
- character readability
- ship and orbital readability
- prompt, log, and command-bar support

## Immediate Recommendation

The lead artist should begin with:

1. `A00 - Runtime Atlas Compatibility Baseline`
2. `A01 - Planetary Environment Identity Set`
3. `A05 - Room Description And Location Title Presentation`

Those three milestones create the earliest believable gain in MUD-feel, readability, and slice identity without requiring the full late-alpha art stack first.
