# Wrath and Ruin - M40 Combat Runtime and Encounter Resolution Brief

## Purpose

M40 exists to prove one complete six-second combat encounter in the authoritative runtime.

This is not a breadth milestone.
It is a proof milestone.

## Scope

The implementation should prove:

- one authoritative encounter trigger
- one six-second round cadence
- readable player and hostile resolution
- persisted post-encounter state
- clear combat diagnostics

## Architectural Rule

Combat truth belongs to the shared/server runtime.

The desktop shell may display:
- encounter state
- hostile state
- combat logs
- persistence diagnostics

The desktop shell must not become the combat owner.

## M40 Definition Of Success

Success for M40 is:

- the player can trigger one authored encounter
- the encounter resolves through a six-second cadence
- combat outcome is explained in text and diagnostics
- post-encounter state persists
- the localhost authority lane still works
- packaging and runtime boundaries stay clean

## What This Is Not Yet

M40 is not:

- a broad NPC combat roster
- a final weapon system
- full tactical cover
- faction combat
- multi-actor squads
- deep AI behavior

Those come later.
