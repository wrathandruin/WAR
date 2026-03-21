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
        const std::string& statusMessage,
        const SharedSimulationDiagnostics& simulationDiagnostics,
        const std::string& roomTitle,
        const std::string& promptLine,
        const std::string& commandBarText) const
    {
#if WAR_HAS_BGFX
        bgfx::dbgTextClear();

        bgfx::dbgTextPrintf(1, 1, 0x0f, "WAR M45 bgfx active");
        bgfx::dbgTextPrintf(1, 2, 0x0a, "Status: %s", statusMessage.c_str());

        char buffer[200]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "Player: (%.1f, %.1f)  Entities: %zu  dt: %.4f",
            playerPosition.x,
            playerPosition.y,
            worldState.entities().count(),
            lastDeltaTime);
        bgfx::dbgTextPrintf(1, 4, 0x0f, "%s", buffer);
        bgfx::dbgTextPrintf(1, 5, 0x0d, "Room: %s", roomTitle.c_str());
        bgfx::dbgTextPrintf(1, 6, 0x0a, "Prompt: %s", promptLine.c_str());
        bgfx::dbgTextPrintf(1, 7, 0x0b, "Command: %s", commandBarText.c_str());
        bgfx::dbgTextPrintf(1, 8, 0x0e, "Objective: %s", simulationDiagnostics.missionObjectiveText.c_str());

        uint16_t line = 10;
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
        (void)statusMessage;
        (void)simulationDiagnostics;
        (void)roomTitle;
        (void)promptLine;
        (void)commandBarText;
#endif
    }
}
