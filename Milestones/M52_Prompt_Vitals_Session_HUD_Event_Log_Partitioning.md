# M52 - Prompt Vitals Session HUD / Event Log Partitioning

## Title

M52 - Prompt Vitals Session HUD / Event Log Partitioning

## Description

M52 gives the player a session-facing text shell instead of relying mainly on debug surfaces.

This delivery separates location text, mission text, and system text into readable presentation blocks while preserving the M51 authored room-description runtime. The prompt/vitals line now behaves like a deliberate session HUD, and the event feed is partitioned into room, mission, and system lanes instead of presenting one undifferentiated stream.

## Download

Planned package name:

`WAR_M52_code_package.zip`

The milestone document is included inside the package at:

`Milestones/M52_Prompt_Vitals_Session_HUD_Event_Log_Partitioning.md`

## Included

- `README.md`
- `Milestones/M52_Prompt_Vitals_Session_HUD_Event_Log_Partitioning.md`
- `src/game/GameLayer.cpp`
- `src/engine/render/DebugOverlayRenderer.cpp`
- `src/engine/render/BgfxDebugFrameRenderer.cpp`

## What should be seen visually

- the session shell now presents location, mission, vitals, command, and reply surfaces as distinct product-facing blocks
- room descriptions no longer carry mission text inline by default
- mission objective and mission phase appear in a dedicated mission panel
- the event log is partitioned into room, mission, and system feeds for readability
- the bgfx fallback keeps the same partitioned shell direction in compact form

## Why this is important

The MUD identity depends not only on what text exists, but on how that text is surfaced. M52 turns the M51 text runtime into a readable session shell that can support command play, social text, and later UI art integration without reverting to a debug-only presentation.

## What should be coming up in the next milestone

M53 - Typed Command Bar / Core Command Routing / Help
