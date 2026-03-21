# Wrath and Ruin - Art Production and Asset Planning Track

## Source Of Truth Note

This document is a production-planning track, not the only technical art specification.

For locked technical direction and lead-artist onboarding, use these documents alongside it:

- `Docs/Wrath and Ruin - Art Audit and Direction Lock.md`
- `Docs/Wrath and Ruin - Lore and Visual Theme Guide.md`
- `Docs/Wrath and Ruin - Lead Artist Onboarding Package.md`
- `Docs/Wrath and Ruin - Art Asset Delivery Contract.md`
- `Docs/Wrath and Ruin - Lead Artist Milestone Runway.md`

If there is a conflict, the onboarding package wins.

## 1. Purpose

This document defines how art production should run alongside the active milestone roadmap.

The goal is not to turn the current slice into an art-complete game immediately.
The goal is to make sure the project stops treating visual content as an implicit future rescue task.

Engineering and art now need to move in parallel.

## 2. Current Production Reality

The current repo already proves a meaningful runtime slice:

- planetary mission progression
- ship runtime and boarding
- orbital travel continuity
- persistence and authority
- packaged demoability

What it does not yet prove is a stable, production-managed art lane.

That means:

- asset requests can still arrive informally
- milestone needs can still outrun art support
- placeholder visuals can distort playtest conclusions
- text-heavy atmosphere can be weakened when authored spaces do not yet look intentional

It also means the team must now respect the difference between:

- source-art production standards
- current engine-integration constraints

At the moment, the runtime still uses a BMP-backed atlas lane for immediate integration.
That should not define the long-term art target, but it must be accounted for in delivery planning.

For production-ready source art, the art lane now uses a fixed delivery contract:

- world gameplay assets = four PNG files mandatory
- inventory-bearing items = matching icon pack mandatory
- UI surfaces = locked UI surface pack mandatory

Use `Docs/Wrath and Ruin - Art Asset Delivery Contract.md` for the exact file-level rules.

## 3. Strategic Rule

From M45 onward, art must be treated as a parallel production workstream.

That means:

- engineering defines milestone-facing asset needs early
- art receives structured requests with priority and context
- placeholder use is intentional and documented
- playtests are judged with awareness of what is placeholder and what is representative

## 4. Immediate Art Goals For The Current Slice

The current art goal is not "finish all visuals."
The current art goal is:

- make the alpha slice readable
- make authored spaces memorable
- support the MUD soul through atmosphere and identity
- reinforce mission, hazard, ship, and orbital beats

## 5. Priority Asset Categories

These categories should guide the first asset list.

### 5.1 Critical Gameplay Readability

- player sprite set
- hostile combatant sprites
- interactable terminal and console variants
- container variants
- hazard tiles and hazard overlays
- ship-interior interactables
- orbital node or route-state markers

### 5.2 MUD-Feel Support Assets

- location header treatment
- room/interior description panel presentation support
- mission/journal text presentation support
- event-log readability styling support
- prompt/vitals bar presentation support
- command-bar presentation support

### 5.3 Environment Identity

- planetary interior tiles and prop variants
- shuttle interior set
- second-destination identity set
- docking and landing transition visuals
- orbital background and traffic-lane identity elements

### 5.4 Production And Packaging Support

- title and package branding assets
- launcher/update branding placeholders if needed
- website or onboarding-facing key art placeholders

## 6. The First Art Intake Pass

When the current asset list arrives, it should be sorted into four buckets:

1. must-have for current milestone validation
2. must-have for alpha playtest credibility
3. useful but placeholder-safe
4. deferred until after alpha

This prevents the art backlog from becoming an undifferentiated wish list.

## 7. Minimum Art Standard For The Next Playtest Phase

The next playtest phase does not require final art.
It does require enough authored identity that testers can feel the difference between:

- transit space
- med or diagnostic space
- quarantine or hazard space
- ship interior space
- orbital or destination-transition space

If those spaces still read as mostly the same place with different debug labels, the playtest will under-report the game's actual potential.

## 8. Room Description Support

One early text-facing requirement should now be treated as an art-adjacent presentation requirement:

- when the player enters an important building, room, interior, ship compartment, or destination zone, they should receive an authored description

This is not only a narrative feature.
It needs presentation support so it feels intentional:

- clear location title
- readable description body
- non-intrusive timing and layout
- visual distinction from combat/event spam

That means engineering and art should treat room-description presentation as a shared deliverable.

## 9. Ownership Split

### Lead Developer Owns

- runtime hooks for room or interior description triggers
- data shape for authored descriptions
- UI surfaces for event log, prompt, mission text, and command bar
- acceptance criteria tied to playtest behavior
- placeholder-safe rendering paths

### Lead Artist Owns

- visual identity for the current vertical slice
- style guidance for room-description presentation
- priority environment and prop asset direction
- consistency across planetary, ship, orbital, and destination spaces
- identifying which requested assets are critical versus nice-to-have

### Shared Ownership

- milestone asset review
- placeholder replacement planning
- UI readability for text-heavy surfaces
- playtest interpretation when placeholder content is still in use

## 10. Deliverables Expected From The Lead Artist

The lead artist should eventually maintain:

- an asset request board grouped by milestone
- an exact artist milestone plan aligned to the alpha runway
- a style guide for the current alpha slice
- a naming and export convention for source and runtime-ready assets
- a manifest discipline proving that required world packs and icon packs are included
- a placeholder-versus-final tracking sheet
- a review cadence with engineering before milestone acceptance

## 11. Immediate Recommendation

Before M45 begins in earnest:

- lock the first asset list
- mark every item by milestone dependency
- call out which assets are required for the MUD-feel MVP
- include room-description presentation in the first UI/art collaboration pass
- use the lore/theme guide so faction and location identity do not get invented ad hoc
- use the onboarding package so camera, scale, atlas, format, and state-variant rules stay locked
- use the lead artist milestone runway so the artist is executing a schedule instead of only a backlog
- use the art asset delivery contract so file-count and icon coverage are not left to interpretation

## 12. Working Rule

If an asset request does not strengthen:

- current slice readability
- room/interior identity
- MUD-feel text presentation
- mission clarity
- ship/orbital continuity
- or alpha playtest credibility

then it is probably not urgent yet.
