# M51 - Room And Interior Description Runtime / Location Titles

## Title

M51 - Room And Interior Description Runtime / Location Titles

## Intent / acceptance summary

M51 introduces the corrected room and interior description runtime for the post-M50 hosted graphical-MUD slice.

This milestone implements:

- authored location-title and room-description runtime
- entry-trigger logic for meaningful spaces
- repetition control so room text fires on entry instead of every movement tick
- reconnect continuity for current location context
- authoring hooks for region and hotspot description content

## Included

- `README.md`
- `Milestones/M50_Launcher_Installer_Update_Productionization.md`
- `Milestones/M51_Room_And_Interior_Description_Runtime_Location_Titles.md`
- `src/engine/world/WorldState.h`
- `shared/src/world/WorldState.cpp`
- `shared/src/world/WorldSemanticDressing.cpp`
- `src/game/GameLayer.h`
- `src/game/GameLayer.cpp`
- `src/engine/render/DebugOverlayRenderer.cpp`
- `src/engine/render/BgfxDebugFrameRenderer.cpp`

## Completion notes

The package is intended to be copied directly over the repo root.

M51 remains deliberately narrow:
- no prompt/vitals HUD partitioning beyond preserving the existing prompt line
- no typed-command expansion beyond preserving the signed-off M48-M50 shell behavior
- no M52+ aggregation or sign-off language

## What should be seen in runtime

- entering cargo bay, transit spine, medlab, command deck, hazard containment, shuttle interior, orbital nodes, and Dust Frontier surfaces now produces readable location identity and authored entry text
- room-entry text does not spam during ordinary movement inside the same space
- reconnect and restart preserve the last known location title and description coherently until live runtime context resolves again

## Next milestone

M52 - Prompt Vitals Session HUD / Event Log Partitioning
