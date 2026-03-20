This package is a complete M4 patch built from the public repo's current M2.1 state and includes the full files needed for M3 and M4 systems on top of that state.

Files included here are complete replacements for the files listed:
- WAR.vcxproj
- WAR.vcxproj.filters
- src/platform/win32/Win32Window.h
- src/platform/win32/Win32Window.cpp
- src/game/GameLayer.h
- src/game/GameLayer.cpp

New files:
- src/engine/gameplay/Action.h
- src/engine/gameplay/ActionQueue.h
- src/engine/gameplay/Entity.h
- docs/milestones/M1_WindowAndRendering.md
- docs/milestones/M2_WorldAndTraversal.md
- docs/milestones/M3_InteractionLayer.md
- docs/milestones/M4_EntitiesAndInteraction.md

Copy these files over your current project, then rebuild.
