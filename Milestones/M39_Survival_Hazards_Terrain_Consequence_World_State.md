# Title

M39 - Survival Hazards / Terrain Consequence / World State

# Description

This milestone introduces environmental pressure to the planetary slice through authored terrain hazards, survival-state damage hooks, oxygen pressure, suit degradation, and persistent hazard-facing actor state.

# Download

WAR_M39_code_package.zip

The milestone document is included inside the package at:

Milestones/M39_Survival_Hazards_Terrain_Consequence_World_State.md

# Included

- terrain-hazard model in shared world state
- authored radiation, toxic, and vacuum hazard tiles
- shared survival-state runtime hooks
- authoritative snapshot persistence for survival state
- overlay and bgfx survival diagnostics
- M39 packaging and hazard acceptance lane updates

# What should be seen visually

- dangerous spaces should read clearly through diagnostics and authored space usage
- survival stats should change while standing in hazard lanes
- hazard state should persist through restart through the existing authoritative persistence lane
- the slice should gain real environmental tension instead of being only traversal plus loot

# Why this is important

M39 gives the planetary slice environmental consequence and supports the hard-science-fiction survival tone of the project.

# What should be coming up in the next milestone

M40 - Six-Second Combat / Encounter Resolution
