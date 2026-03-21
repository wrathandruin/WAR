# Title

M40 - Six-Second Combat / Encounter Resolution

# Description

This milestone proves the first real six-second combat lane in the authoritative runtime.

The goal is not combat breadth.
The goal is one readable, persistent, authoritative encounter that can be validated in the packaged local demo lane.

# Download

WAR_M40_code_package.zip

The milestone document is included inside the package at:

Milestones/M40_Six_Second_Combat_Encounter_Resolution.md

# Included

- authoritative encounter trigger and combat runtime
- six-second combat round cadence
- persisted actor, inventory, equipment, and encounter state
- combat diagnostics in overlay and status output
- staged package and M40 acceptance wrapper

# What should be seen visually

- moving into an authored encounter space should trigger combat
- one hostile encounter should resolve through a six-second loop
- the event log should explain what happened
- player and hostile state should be visible
- post-encounter state should persist after restart

# Why this is important

M40 is one of the core alpha proof points.
Without a real encounter loop, the slice is still only a movement/survival/inventory foundation.

# What should be coming up in the next milestone

M41 - Planetary Mission Slice / Objectives / Gate Logic
