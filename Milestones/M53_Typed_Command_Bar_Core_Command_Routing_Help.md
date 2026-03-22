# M53 - Typed Command Bar / Core Command Routing / Help

## Title

M53 - Typed Command Bar / Core Command Routing / Help

## Description

M53 turns the session shell into a real typed-command surface instead of a loose debug input strip.

This delivery adds a deliberate command parsing and dispatch seam, explicit help and discoverability text, syntax-aware command failures, and the minimum M53 command set required by the runway: `look`, `say`, `emote`, `help`, and `inv`. It preserves the M51 location runtime and the M52 session HUD / event-log partitioning while making typed command input feel like an intentional product layer.

## Download

WAR_M53_code_package.zip

The milestone document is included inside the package at:

Milestones/M53_Typed_Command_Bar_Core_Command_Routing_Help.md

## Included

- README.md
- Milestones/M53_Typed_Command_Bar_Core_Command_Routing_Help.md
- src/game/GameLayer.cpp
- src/engine/render/DebugOverlayRenderer.cpp
- src/engine/render/BgfxDebugFrameRenderer.cpp

## What should be seen visually

- the command bar now supports readable typed command usage for `help`, `look`, `say`, `emote`, and `inv`
- the session HUD now exposes command discoverability directly instead of relying on guesswork
- bad syntax such as empty `say` or `emote` commands produces explicit shell feedback instead of silent failure
- unknown commands produce deliberate error messaging that points the player back to `help`
- the command shell remains integrated with the same session-facing text shell introduced in M52

## Why this is important

Wrath and Ruin should feel like a graphical MUD-descended product, not only a tactical client with debug text. M53 makes typed commands an intentional, understandable, supportable part of the session experience without widening into the later social-routing and shared-presence milestones.

## What should be coming up in the next milestone

M54 - Speech Emote And Local Social Text Visibility
