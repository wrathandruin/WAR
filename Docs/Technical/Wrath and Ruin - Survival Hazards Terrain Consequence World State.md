# Wrath and Ruin - Survival Hazards Terrain Consequence World State

## Purpose

This document defines the M39 hazard model, terrain-consequence rules, and runtime ownership boundaries for the planetary slice.

## Ownership

### Shared runtime owns

- terrain-hazard definitions on authored tiles
- player survival state
- oxygen drain, exposure accumulation, suit-integrity loss, and hazard damage
- consequence counters and hazard-facing diagnostics state
- authoritative snapshot shape for survival state

### Server owns

- authoritative advancement of hazard consequences
- publication of authoritative snapshot state

### Desktop owns

- presentation of hazard diagnostics
- player-facing visibility and review surfaces
- no gameplay-critical hazard truth

## Hazard model

Approved hazard types for this milestone:

- `radiation-leak`
- `toxic-residue`
- `vacuum-breach`

Each hazard tile carries:

- tile coordinate
- label
- type
- severity
- active flag

## Survival state

The player survival state now includes:

- health
- max health
- armor
- suit integrity
- oxygen seconds remaining
- radiation dose
- toxic exposure
- hazard ticks
- terrain-consequence events
- current hazard label
- current terrain consequence text
