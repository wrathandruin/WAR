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
        const SharedSimulationDiagnostics& simulationDiagnostics) const
    {
#if WAR_HAS_BGFX
        bgfx::dbgTextClear();

        bgfx::dbgTextPrintf(1, 1, 0x0f, "WAR M44 bgfx active");
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
        bgfx::dbgTextPrintf(1, 5, 0x0d, "Objective: %s", simulationDiagnostics.missionObjectiveText.c_str());
        bgfx::dbgTextPrintf(
            1,
            6,
            0x0e,
            "Mission: %s  Gate locked: %s  Ship prep: %s",
            simulationDiagnostics.missionPhaseText.c_str(),
            simulationDiagnostics.missionGateLocked ? "yes" : "no",
            simulationDiagnostics.shipRuntimePrepReady ? "yes" : "no");
        bgfx::dbgTextPrintf(
            1,
            7,
            0x0b,
            "Ship: %s  Owner: %s  Context: %s",
            simulationDiagnostics.shipName.c_str(),
            simulationDiagnostics.shipOwnershipText.c_str(),
            simulationDiagnostics.playerRuntimeContextText.c_str());
        bgfx::dbgTextPrintf(
            1,
            8,
            0x0b,
            "Boarded: %s  Docked: %s  Command: %s  Launch prep: %s",
            simulationDiagnostics.shipBoarded ? "yes" : "no",
            simulationDiagnostics.shipDocked ? "yes" : "no",
            simulationDiagnostics.shipCommandClaimed ? "yes" : "no",
            simulationDiagnostics.shipLaunchPrepReady ? "yes" : "no");
        bgfx::dbgTextPrintf(
            1,
            9,
            0x09,
            "Orbit: %s  Phase: %s  Node: %s -> %s  Ticks: %u",
            simulationDiagnostics.orbitalLayerActive ? "active" : "inactive",
            simulationDiagnostics.orbitalPhaseText.c_str(),
            simulationDiagnostics.orbitalCurrentNodeText.c_str(),
            simulationDiagnostics.orbitalTargetNodeText.c_str(),
            simulationDiagnostics.orbitalTravelTicksRemaining);
        bgfx::dbgTextPrintf(
            1,
            10,
            0x0c,
            "Frontier: %s  Site: %s  Home dock: %s",
            simulationDiagnostics.frontierSurfaceActive ? "surface-active" : "surface-inactive",
            simulationDiagnostics.frontierSiteText.c_str(),
            simulationDiagnostics.orbitalHomeDockReached ? "restored" : "pending");

        uint16_t line = 12;
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
#endif
    }
}
