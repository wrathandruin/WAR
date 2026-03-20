This package combines M14, M15, and M16 as a full-file drop-in update.

Full replacement files included:
- README.md
- src/engine/render/BgfxWorldRenderer.h
- src/engine/render/BgfxWorldRenderer.cpp
- src/engine/render/BgfxDebugFrameRenderer.h
- src/engine/render/BgfxDebugFrameRenderer.cpp
- src/game/GameLayer.h
- src/game/GameLayer.cpp
- WAR.vcxproj
- WAR.vcxproj.filters

New files included:
- docs/milestones/M14_FirstBgfxTileRenderer.md
- docs/milestones/M15_BgfxEntitiesPlayerAndPath.md
- docs/milestones/M16_BgfxShaderPipeline.md
- assets/shaders/src/vs_color.sc
- assets/shaders/src/fs_color.sc

Important:
- This package does not include compiled shader binaries.
- Compile the shader source into:
  assets/shaders/dx11/vs_color.bin
  assets/shaders/dx11/fs_color.bin

Example shader compile commands (from a bgfx tool build):
  shaderc.exe -f assets/shaders/src/vs_color.sc -o assets/shaders/dx11/vs_color.bin --platform windows --type vertex   -p vs_5_0 -i third_party/bgfx/src
  shaderc.exe -f assets/shaders/src/fs_color.sc -o assets/shaders/dx11/fs_color.bin --platform windows --type fragment -p ps_5_0 -i third_party/bgfx/src

If the binaries are missing, bgfx mode will still run and show clear debug text telling you what is missing.
