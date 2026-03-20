#include "engine/render/BgfxDebugFrameRenderer.h"

#include <cstdio>

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>)
#    define WAR_HAS_BGFX 1
#  else
#    define WAR_HAS_BGFX 0
#  endif
#else
#  define WAR_HAS_BGFX 0
#endif

#if WAR_HAS_BGFX
#  include <bgfx/bgfx.h>
#endif

namespace war
{
    void BgfxDebugFrameRenderer::render(
        const WorldState& worldState,
        const Vec2& playerPosition,
        const std::vector<std::string>& eventLog,
        float lastDeltaTime,
        bool worldRendered,
        const char* worldStatus) const
    {
#if WAR_HAS_BGFX
        bgfx::dbgTextClear();

        bgfx::dbgTextPrintf(1, 1, 0x0f, "WAR Milestone 16 - bgfx active");
        bgfx::dbgTextPrintf(1, 2, worldRendered ? 0x0a : 0x0c, "Geometry path: %s", worldRendered ? "READY" : "FAILED");
        bgfx::dbgTextPrintf(1, 3, 0x0f, "Status: %s", worldStatus != nullptr ? worldStatus : "No status available.");

        if (!worldRendered)
        {
            bgfx::dbgTextPrintf(1, 5, 0x0e, "Expected binaries in assets/shaders/dx11/");
            bgfx::dbgTextPrintf(1, 6, 0x0e, "Expected files: vs_color.bin  fs_color.bin");
        }

        char buffer[160]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "Player: (%.1f, %.1f)  Entities: %zu  dt: %.4f",
            playerPosition.x,
            playerPosition.y,
            worldState.entities().count(),
            lastDeltaTime);
        bgfx::dbgTextPrintf(1, 8, 0x0f, "%s", buffer);

        uint16_t line = 11;
        bgfx::dbgTextPrintf(1, line++, 0x0d, "Recent Events:");
        for (const std::string& entry : eventLog)
        {
            bgfx::dbgTextPrintf(2, line++, 0x0f, "%s", entry.c_str());
            if (line > 28)
            {
                break;
            }
        }
#else
        (void)worldState;
        (void)playerPosition;
        (void)eventLog;
        (void)lastDeltaTime;
        (void)worldRendered;
        (void)worldStatus;
#endif
    }
}
