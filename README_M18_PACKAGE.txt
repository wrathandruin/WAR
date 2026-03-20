This package is a full-file M18 drop-in update.

Included full replacement files:
- README.md
- src/engine/render/BgfxRenderData.h
- src/engine/render/BgfxRenderData.cpp
- src/engine/render/BgfxWorldRenderer.h
- src/engine/render/BgfxWorldRenderer.cpp
- WAR.vcxproj
- WAR.vcxproj.filters

New files:
- docs/milestones/M18_BgfxCameraProjectionCleanup.md
- src/engine/render/BgfxViewTransform.h
- src/engine/render/BgfxViewTransform.cpp

Notes:
- This package assumes your repo is already on the working M17 baseline.
- No patches or snippets are required.
- Shader binaries are still expected in assets/shaders/dx11 and are copied to the output folder by the project file.
