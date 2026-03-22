# Wrath and Ruin - M45-M70 Alpha Playtest Runway

## Purpose

This document replaces the idea that only four milestones remain before a meaningful alpha.

The current slice is strong, but it is not yet a real hosted graphical-MUD product.
The correct production move is to extend the roadmap into a more specific alpha-readiness runway that closes the missing product, networking, security, operations, content, and identity gaps before the first real alpha cohort.

This runway is intentionally specific.
It exists so the lead developer knows exactly what to execute and what not to hand-wave.

## What This Runway Adds Explicitly

The earlier roadmap was too compressed in the following areas:

- real remote transport and hosted-session reality
- account/session/ticket ownership
- protocol versioning and trust boundaries
- security, abuse controls, and recovery discipline
- room-description and text-layer identity
- typed command support and social presence
- content and description authoring pipeline
- parallel art production and UI/text presentation
- support, moderation, and cohort operations
- performance, accessibility, and onboarding

These are now explicit milestones rather than vague expectations inside "hardening."

## Alpha Cohort Definition

The first real alpha playtest should only happen once the project can prove:

- a hosted authoritative session beyond localhost-only proof
- remote connect, disconnect, reconnect, and recovery behavior
- room/interior descriptions, prompt/vitals direction, event-log readability, and typed command-bar basics
- local speech/emote and same-space shared presence
- stable persistence across mission, travel, ship, and destination state
- supportable crash, telemetry, admin, and recovery behavior
- controlled onboarding for the test cohort
- enough art and environment identity that the slice feels authored

The first real alpha playtest gate in this runway is **M68**.
M69 and M70 cover immediate execution and stabilization after the first cohort begins.

## Phase 5A - Hosted Product Foundation

### Title

M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline

### Description

M45 packages the current slice into the first internally hostable alpha candidate.

The goal is not broad public release, full auth, or large-scale multiplayer.
The goal is one honest internal-alpha product lane with hosted boot, remote connect direction, telemetry visibility, and the first explicit MUD-feel session surfaces.

### Required focus

- internal alpha package shape
- hosted deploy and boot lane
- remote client/server transport direction
- protocol ownership and first version stamp
- telemetry baseline for host, client, session, and build identity
- room-entry descriptions for key authored spaces
- first prompt or vitals direction
- first typed command-bar shell or MVP entry surface
- localhost fallback validation retained

### Acceptance signals

- a packaged internal-alpha lane exists and is operator-usable
- a client can connect to a non-localhost or explicitly hosted target lane
- protocol version mismatch is detectable and diagnosable
- connect and disconnect failures are visible instead of silent
- room-entry descriptions appear in key authored spaces
- a prompt or vitals direction and a typed command-entry surface are visible
- localhost validation still remains as a fallback test lane

### Why this is important

Without M45, the project is still proving a packaged prototype rather than an internally hostable product.

### Next milestone

M46 - Trust Boundary / Environment Config / Secrets Baseline

---

### Title

M46 - Trust Boundary / Environment Config / Secrets Baseline

### Description

M46 defines how the hosted product is configured and trusted.

This milestone should establish the first real environment model for local, staging, and hosted alpha, including how secrets, runtime config, and trust boundaries are owned.

### Required focus

- trust-boundary document
- environment configuration model
- secrets handling policy
- protocol and runtime config separation
- save-path and runtime-root hygiene in hosted environments

### Acceptance signals

- local, staging, and hosted config surfaces are distinct
- secrets are not hard-coded into repo-tracked files
- runtime status and config errors are diagnosable
- leadership can explain what is trusted and what is not

### Why this is important

Security problems are much cheaper to prevent here than after onboarding real testers.

### Next milestone

M47 - Account Session Ticket Handoff / Authenticated Entry

---

### Title

M47 - Account Session Ticket Handoff / Authenticated Entry

### Description

M47 establishes how a real player session enters the hosted runtime.

The current project should stop acting like the client and host can be coupled casually.
This milestone defines the first trustworthy session-entry model and reconnect identity direction.

### Required focus

- account/session ownership direction
- trusted ticket or session handoff model
- authenticated session entry
- reconnect token or resume identity approach
- session failure states and expiry handling

### Acceptance signals

- session entry is intentional rather than ad hoc
- a hosted player session can be validated and denied coherently
- reconnect identity has a documented path
- future web and launcher integration has a clean seam

### Why this is important

The game cannot become a real hosted product if entry into the world is still undefined.

### Next milestone

M48 - Crash Capture / Failure Bundles / Operator Triage Baseline

---

### Title

M48 - Crash Capture / Failure Bundles / Operator Triage Baseline

### Description

M48 establishes the first failure-capture and operator-triage baseline for hosted play.

This milestone should make bootstrap, startup, and runtime failures diagnosable from packaged artifacts instead of guesswork.

### Required focus

- startup, runtime, and bootstrap failure-bundle directories
- copied runtime evidence attachments and manifests
- promoted operator-triage artifacts for the latest failure
- packaged validation of failure capture and triage surfaces
- preservation of the earlier hosted validation lanes

### Acceptance signals

- bootstrap and runtime failures create bundles in known locations
- bundle manifests and attachment manifests are preserved with the failure
- operator triage gets a promoted latest bundle, summary, and checklist
- support and engineering can reason about failures from packaged artifacts alone

### Why this is important

Hosted testing without failure capture and triage is just slower chaos.

### Next milestone

M49 - Beta Content Scale / Retention Expansion / Release-Candidate Discipline

---

### Title

M49 - Beta Content Scale / Retention Expansion / Release-Candidate Discipline

### Description

M49 moves the slice from alpha-hardening posture into a disciplined beta-candidate posture.

This milestone treats content breadth, tester return, and release-candidate packaging as one connected production discipline.

### Required focus

- staged beta content manifests
- retention summaries and launch journals
- release-candidate manifest, checklist, and latest pointer
- preserved M45 through M48 packaged validation flow
- beta candidate packaging discipline

### Acceptance signals

- the beta candidate stages real authored content manifests
- repeated launches leave behind retention surfaces that can be reviewed
- hosted/bootstrap runs leave behind release-candidate manifests and checklists
- the package reads like a beta candidate rather than only an internal-alpha bundle

### Why this is important

Without content scale and retention discipline, beta credibility is just branding.

### Next milestone

M50 - Launcher / Installer / Update Productionization

---

### Title

M50 - Launcher / Installer / Update Productionization

### Description

M50 moves the project from beta-candidate discipline into the first real market-facing distribution discipline.

The goal is not a final commercial launcher stack.
The goal is to make launcher entry, installer shape, and update direction explicit, reviewable, and testable.

### Required focus

- launcher source-of-truth manifest
- installer source-of-truth manifest
- update channel source-of-truth manifest
- launcher, installer, and updater runtime capture
- preserved M45 through M49 packaged validation flow

### Acceptance signals

- the staged package exposes dedicated Launcher and Installer lanes
- client launches leave behind launcher-session manifests and latest pointers
- host/bootstrap runs leave behind installer and updater runtime manifests
- the package reads like a real distributed product candidate

### Why this is important

If launcher, install, and update direction are still implied after beta packaging, the project is not ready to be treated like a market candidate.

### Next milestone

M51 - Room And Interior Description Runtime / Location Titles

---

## Phase 5B - Graphical MUD Session MVP

### Title

M51 - Room And Interior Description Runtime / Location Titles

### Description

M51 introduces the first real room or area description system for the new slice.

When the player enters an important building, interior, ship compartment, orbital node, or second-destination space, the game should present authored descriptive text and a readable location identity.

### Required focus

- location-title and room-description data shape
- entry-trigger logic
- repetition control and spam avoidance
- save/reconnect continuity for current location context
- authoring hooks for description content

### Acceptance signals

- entering meaningful spaces produces authored descriptive text
- the location title is readable and intentional
- descriptions do not spam constantly during normal movement
- reconnect preserves location identity coherently

### Why this is important

This is one of the earliest high-value wins for making the slice feel like Wrath and Ruin rather than a silent tactics prototype.

### Next milestone

M52 - Prompt Vitals Session HUD / Event Log Partitioning

---

### Title

M52 - Prompt Vitals Session HUD / Event Log Partitioning

### Description

M52 gives the player a session-facing text shell instead of relying mainly on debug surfaces.

The goal is a prompt/vitals/event-log presentation that feels like part of the product, not just engineering instrumentation.

### Required focus

- prompt or vitals strip
- event-log partitioning and readability
- mission text and room-description presentation separation
- severity and category treatment for text output
- art-friendly layout hooks

### Acceptance signals

- room descriptions, mission beats, hazards, combat, and social text do not blur together
- the player has a readable status shell
- the game feels more session-like and less debug-like

### Why this is important

The MUD soul is not only what text exists.
It is also how text is surfaced.

### Next milestone

M53 - Typed Command Bar / Core Command Routing / Help

---

### Title

M53 - Typed Command Bar / Core Command Routing / Help

### Description

M53 adds the first typed command surface to the graphical client.

This is not a return to command-only navigation.
It is a deliberate support layer that makes the graphical slice feel like a real MUD-descended session.

### Required focus

- typed command bar
- command parsing and dispatch seam
- help and discoverability
- small core command set
- failure and unknown-command messaging

### Minimum command set

- `look`
- `say`
- `emote`
- `help`
- `inv`
- mission/help shortcut commands as needed

### Acceptance signals

- commands can be typed and resolved without destabilizing the main client loop
- the player can discover and use the command layer
- command output is surfaced in the same session shell as other text

### Why this is important

Without a command surface, the current client still feels too far from the product’s MUD identity.

### Next milestone

M54 - Speech Emote And Local Social Text Visibility

---

### Title

M54 - Speech Emote And Local Social Text Visibility

### Description

M54 makes the first real social text loop visible in the new slice.

The focus is narrow but important: same-space speech, emotes, and social text visibility.

### Required focus

- local speech routing
- emote routing
- same-space text visibility rules
- social-log formatting
- persistence and reconnect expectations for live session continuity

### Acceptance signals

- two players in the same space can see speech and emote output
- social text is distinguishable from combat and system messages
- the session feels inhabited instead of solo

### Why this is important

This is the first milestone where the new slice starts proving social identity instead of only authored progression.

### Next milestone

M55 - Shared Presence / Same-Space Awareness / Interest Rules

---

### Title

M55 - Shared Presence / Same-Space Awareness / Interest Rules

### Description

M55 proves that another player can meaningfully exist in the same slice.

The goal is not MMO-scale concurrency.
The goal is first-class shared presence, visibility, and same-space awareness.

### Required focus

- same-space player visibility
- presence list or nearby-presence shell
- first interest or relevance rules
- minimal multi-user consistency in room and mission context
- diagnostics for shared-presence failures

### Acceptance signals

- two players can observe one another in the same meaningful space
- shared presence obeys clear visibility rules
- room descriptions and social text remain coherent in multi-user use

### Why this is important

The first real alpha playtest must be able to evaluate the game as shared space, not only as solo authored content.

### Next milestone

M56 - Reconnect Resume / Session Continuity / Drop Recovery

---

### Title

M56 - Reconnect Resume / Session Continuity / Drop Recovery

### Description

M56 hardens the player session against ordinary drop and return behavior.

This milestone should cover reconnect flow, session continuity, and restoring the player into a coherent state without requiring operator rescue every time.

### Required focus

- reconnect flow
- resume identity and session continuity
- reconnect-safe room, mission, and ship context restoration
- host-side stale-session handling
- reconnection diagnostics

### Acceptance signals

- a player can drop and return without losing the session fiction
- mission, room, ship, and orbital context return coherently
- operators can distinguish reconnect from bad duplicate sessions

### Why this is important

Real hosted testing always includes disconnects.
If reconnect is weak, all playtest conclusions become noisy.

### Next milestone

M57 - Description Mission Journal Data Schema / Authoring Pipeline

---

### Title

M57 - Description Mission Journal Data Schema / Authoring Pipeline

### Description

M57 moves critical authored text and mission content out of code-owned sprawl and into a deliberate content pipeline.

This milestone should make descriptions, mission beats, journal text, and related authored strings scalable.

### Required focus

- data schema for room descriptions
- data schema for mission and journal text
- content validation rules
- load errors and fallback behavior
- source-of-truth ownership for authored text

### Acceptance signals

- mission and description content is no longer primarily hard-coded
- validation catches bad or missing content
- writers and designers have a clearer future seam

### Why this is important

The current slice cannot scale or localize its text identity if content remains mostly embedded in runtime code.

### Next milestone

M58 - Art Integration Pass 1 / Environment Identity / Text Presentation

---

### Title

M58 - Art Integration Pass 1 / Environment Identity / Text Presentation

### Description

M58 is the first milestone where the new art-production track should materially change the feel of the slice.

The goal is not final art.
The goal is that planetary, ship, orbital, and destination spaces feel intentionally different, and that text-heavy surfaces feel presented rather than merely printed.

### Required focus

- first prioritized asset batch integration
- environment identity for key spaces
- room-description and prompt/event-log presentation styling
- placeholder-versus-final tracking discipline
- art validation in packaging

### Acceptance signals

- key spaces no longer read as the same place with different labels
- text-heavy surfaces feel intentional
- the slice is more credible for the first real cohort

### Why this is important

Engineering alone cannot carry atmosphere and identity all the way to alpha credibility.

### Next milestone

M59 - Multi-User Scenario Beat / Cooperation And Communication Validation

---

## Phase 5C - Alpha Cohort Readiness

### Title

M59 - Multi-User Scenario Beat / Cooperation And Communication Validation

### Description

M59 adds at least one narrow scenario that proves the game can be playtested as a shared social/product experience rather than only as a solo progression lane.

### Required focus

- one cooperative or coordinated beat
- communication requirement between players
- same-space scenario logic
- shared objective visibility
- scenario-specific validation

### Acceptance signals

- two players can complete a small coordinated beat
- communication materially helps or is required
- the test proves more than solo mission pacing

### Why this is important

This is where the alpha slice starts proving that the MUD heritage still changes how the game plays.

### Next milestone

M60 - Hosted Staging Acceptance / Soak / Alerting

---

### Title

M60 - Hosted Staging Acceptance / Soak / Alerting

### Description

M60 turns the hosted lane into something that can survive longer and noisier test sessions.

This milestone should cover staged-host acceptance, soak behavior, and first alerting expectations.

### Required focus

- hosted staging environment
- soak-test path
- alerting or failure-notification baseline
- long-session stability validation
- host/runtime watchdog expectations

### Acceptance signals

- staged hosted acceptance is repeatable
- soak sessions expose issues through artifacts instead of mystery
- failures reach operators fast enough to matter

### Why this is important

Short happy-path tests are not enough for alpha readiness.

### Next milestone

M61 - Performance Budgets / Hitch Memory Network Diagnostics

---

### Title

M61 - Performance Budgets / Hitch Memory Network Diagnostics

### Description

M61 establishes performance and stability budgets for the alpha slice.

The goal is not AAA optimization.
The goal is to stop performance surprises from poisoning the first cohort.

### Required focus

- frame hitch visibility
- memory and runtime budget reporting
- transport performance metrics
- content-heavy area checks
- build configuration comparison

### Acceptance signals

- major hitch classes are visible and triaged
- performance regressions are measurable
- the cohort build has an explicit budget target

### Why this is important

Alpha feedback is hard to trust when performance instability dominates perception.

### Next milestone

M62 - Accessibility / Keybinding / Input Resilience / Readability

---

### Title

M62 - Accessibility / Keybinding / Input Resilience / Readability

### Description

M62 adds the minimum accessibility and input resilience required for real testers.

This milestone should ensure that the new text surfaces, command surfaces, and tactical play are not overly brittle or opaque.

### Required focus

- readable text sizing and contrast review
- keybinding and input resilience
- command-bar usability
- log readability and scroll behavior
- mouse and keyboard coexistence

### Acceptance signals

- testers can comfortably use the text and tactical surfaces together
- basic input customization or resilience exists
- accessibility concerns are visible early rather than being deferred until beta

### Why this is important

If testers cannot comfortably perceive or use the product, their feedback will be low-signal.

### Next milestone

M63 - First Session Onboarding / Tutorialization / Expectation Setting

---

### Title

M63 - First Session Onboarding / Tutorialization / Expectation Setting

### Description

M63 defines the first-session path for alpha testers.

The current product should not assume players already understand how to read a graphical MUD hybrid.
It needs explicit onboarding for movement, descriptions, commands, mission flow, and social expectations.

### Required focus

- first-session objective framing
- movement and interaction teaching
- command-bar introduction
- room-description and prompt literacy
- failure and confusion reduction in the opening minutes

### Acceptance signals

- a fresh tester can understand the slice without verbal coaching
- first-session confusion points are reduced and measured
- the product teaches its hybrid identity rather than assuming it

### Why this is important

Strong systems are not enough if the first fifteen minutes are confusing.

### Next milestone

M64 - Support Workflow / Player Reports / Incident Response

---

### Title

M64 - Support Workflow / Player Reports / Incident Response

### Description

M64 establishes the first real support lane for the alpha cohort.

This milestone should make it possible to intake issues, respond to incidents, and classify player problems without engineering improvisation.

### Required focus

- support intake flow
- issue categorization and triage
- incident-response baseline
- known-issues communication path
- operator-facing support references

### Acceptance signals

- tester issues have a defined intake path
- incidents have a defined response owner and runbook
- leadership can tell whether a test failure is gameplay, ops, or support in nature

### Why this is important

If support is undefined, the first cohort can create noise faster than the team can learn from it.

### Next milestone

M65 - Launcher Installer Update Channel / Build Distribution

---

### Title

M65 - Launcher Installer Update Channel / Build Distribution

### Description

M65 defines how the alpha build reaches the cohort and stays current.

The goal is not a final commercial launcher.
The goal is one controlled distribution/update path that can support a real alpha without manual chaos.

### Required focus

- alpha distribution lane
- installer or launcher direction
- update channel or manifest model
- version visibility to testers and operators
- rollback-aware build distribution

### Acceptance signals

- the cohort can receive the build through a controlled path
- build version is visible in support and telemetry surfaces
- updates stop being a bespoke engineering exercise

### Why this is important

No real alpha should depend on ad hoc zip passing.

### Next milestone

M66 - Analytics Funnel / Survey / Feedback Instrumentation

---

### Title

M66 - Analytics Funnel / Survey / Feedback Instrumentation

### Description

M66 makes the alpha cohort measurable.

The team should be able to connect telemetry, player reports, surveys, and qualitative notes into one usable feedback picture.

### Required focus

- first-session and mission funnel analytics
- reconnect and dropout analytics
- survey and feedback capture path
- event-to-feedback correlation
- retention and confusion indicators

### Acceptance signals

- leadership can tell where players get stuck or leave
- telemetry and qualitative feedback can be read together
- the cohort produces actionable product learning

### Why this is important

An alpha without feedback instrumentation is mostly anecdote.

### Next milestone

M67 - Content Reliability / Description Coverage / Art Pass 2

---

### Title

M67 - Content Reliability / Description Coverage / Art Pass 2

### Description

M67 ensures the slice is content-complete enough to withstand the first cohort.

This milestone should tighten description coverage, mission text, environment identity, and encounter reliability so testers are seeing a deliberate slice rather than an uneven one.

### Required focus

- description coverage pass
- mission and journal text reliability
- environment identity pass 2
- encounter and destination consistency
- placeholder audit before cohort launch

### Acceptance signals

- key spaces no longer feel text-empty
- art and text coverage are good enough for honest perception testing
- placeholder gaps are known and controlled

### Why this is important

The first cohort should not be spent discovering that half the slice still feels unfinished by default.

### Next milestone

M68 - Alpha Cohort Rehearsal / Go-No-Go Gate

---

### Title

M68 - Alpha Cohort Rehearsal / Go-No-Go Gate

### Description

M68 is the final gate before the first real alpha playtest.

This milestone should rehearse the full flow: build distribution, hosted boot, connect, reconnect, mission progression, room descriptions, commands, social presence, support intake, telemetry, recovery, and operator response.

### Required focus

- full dry-run of the alpha cohort flow
- go/no-go review
- release-candidate discipline for the cohort build
- launch-day checklists
- final blocker classification and ownership

### Acceptance signals

- the team can run the slice end to end without heroics
- go/no-go is based on evidence, not optimism
- leadership can explain exactly what the cohort will test and why

### Why this is important

This is the milestone where "we think we are ready" becomes "we have rehearsed and can prove it."

### Next milestone

M69 - Alpha Cohort 1 Execution / Live Observation / Rapid Triage

---

## Immediate Post-Playtest Follow-On

### Title

M69 - Alpha Cohort 1 Execution / Live Observation / Rapid Triage

### Description

M69 runs the first real alpha playtest and captures the first live product truth.

The goal is not to broaden scope during the test.
The goal is to observe, support, and triage rapidly without destabilizing the build.

### Required focus

- run the first cohort
- live observation and incident handling
- crash and telemetry review during the session
- rapid triage classification
- protect build discipline during the test

### Acceptance signals

- the team can run the cohort without losing operational control
- issues are triaged fast enough to be useful
- the product produces real playtest evidence instead of noise

### Why this is important

The first cohort is a product event, not just another QA pass.

### Next milestone

M70 - Alpha Stabilization / Roadmap Recut / Beta Gate Reset

---

### Title

M70 - Alpha Stabilization / Roadmap Recut / Beta Gate Reset

### Description

M70 consolidates what the first real alpha actually taught the team.

This milestone should separate:

- product-truth issues
- content-truth issues
- operations-truth issues
- and roadmap-truth issues

It should then recut the roadmap honestly for the next stage.

### Required focus

- stabilize the highest-value blockers from the first cohort
- recut priorities based on evidence
- separate alpha fixes from post-alpha ambitions
- reset the beta gate with current product truth
- update planning, docs, and production expectations

### Acceptance signals

- the roadmap after the first cohort is evidence-based
- leadership can say what changed and why
- the project exits the first alpha more organized than it entered it

### Why this is important

The first real alpha playtest only matters if the team can turn it into disciplined next steps.
