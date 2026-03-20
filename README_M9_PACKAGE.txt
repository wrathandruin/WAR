This package is a complete M9 patch for Render Backend Abstraction.

Changed full replacement files:
- README.md
- src/game/GameLayer.h
- src/game/GameLayer.cpp
- WAR.vcxproj
- WAR.vcxproj.filters

New files:
- src/engine/render/RenderBackendType.h
- src/engine/render/IRenderDevice.h
- src/engine/render/GdiRenderDevice.h
- src/engine/render/GdiRenderDevice.cpp
- src/engine/render/BgfxRenderDevice.h
- src/engine/render/BgfxRenderDevice.cpp
- docs/milestones/M9_RenderBackendAbstraction.md

Copy these files over your current project, then rebuild.

Note:
- GDI is still the active runtime backend.
- bgfx is represented by a compile-safe stub in this milestone.
- the real bgfx hookup is the next milestone.
