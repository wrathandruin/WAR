#include "engine/render/DebugOverlayRenderer.h"

#include <sstream>
#include <string>

namespace war
{
    namespace
    {
        std::string tileText(bool hasTile, TileCoord tile)
        {
            if (!hasTile)
            {
                return "none";
            }

            return "(" + std::to_string(tile.x) + ", " + std::to_string(tile.y) + ")";
        }
    }

    const char* DebugOverlayRenderer::entityTypeToText(EntityType type) const
    {
        switch (type)
        {
        case EntityType::Crate: return "crate";
        case EntityType::Terminal: return "terminal";
        case EntityType::Locker: return "locker";
        case EntityType::Ship: return "ship";
        default: return "unknown";
        }
    }

    const char* DebugOverlayRenderer::hotspotTypeToText(WorldAuthoringHotspotType type) const
    {
        switch (type)
        {
        case WorldAuthoringHotspotType::Encounter: return "encounter";
        case WorldAuthoringHotspotType::Control: return "control";
        case WorldAuthoringHotspotType::Transit: return "transit";
        case WorldAuthoringHotspotType::Loot: return "loot";
        case WorldAuthoringHotspotType::Hazard: return "hazard";
        default: return "unknown";
        }
    }

    void DebugOverlayRenderer::render(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        const Vec2& playerPosition,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile,
        bool hasSelectedTile,
        TileCoord selectedTile,
        bool hasActionTargetTile,
        TileCoord actionTargetTile,
        const std::vector<std::string>& eventLog,
        float lastDeltaTime,
        const POINT& mousePosition,
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
        const SharedSimulationDiagnostics& simulationDiagnostics,
        const HeadlessHostPresenceReport& headlessHostPresenceReport,
        const AuthoritativeHostProtocolReport& authoritativeHostProtocolReport,
        const std::string& roomTitle,
        const std::string& roomDescription,
        const std::string& promptLine,
        const std::string& commandBarText,
        const std::string& commandEcho) const
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(225, 225, 225));

        const Vec2 mouseWorld = camera.screenToWorld(mousePosition.x, mousePosition.y);
        const TileCoord mouseTile = worldState.world().worldToTile(mouseWorld);
        const TileCoord playerTile = worldState.world().worldToTile(playerPosition);
        const std::string hovered = tileText(hasHoveredTile, hoveredTile);
        const std::string selected = tileText(hasSelectedTile, selectedTile);
        const std::string actionTarget = tileText(hasActionTargetTile, actionTargetTile);
        const std::string pathDestination =
            currentPath.empty() || pathIndex >= currentPath.size()
                ? "none"
                : tileText(true, currentPath.back());

        std::ostringstream info;
        info
            << "WAR Milestone 45\n"
            << "Connect target: " << localDemoDiagnosticsReport.connectTargetName << "\n"
            << "Transport: " << localDemoDiagnosticsReport.connectTransport << "\n"
            << "Lane mode: " << localDemoDiagnosticsReport.connectLaneMode << "\n"
            << "Runtime root: " << localDemoDiagnosticsReport.runtimeRootDisplay << "\n"
            << "Authoritative lane: " << (simulationDiagnostics.hostAuthorityActive ? "headless host" : "local") << "\n"
            << "Host instance: " << headlessHostPresenceReport.hostInstanceId << "\n"
            << "Host session: " << headlessHostPresenceReport.sessionId << "\n"
            << "Protocol lane ready: " << (authoritativeHostProtocolReport.authorityLaneReady ? "yes" : "no") << "\n"
            << "Snapshot present: " << (authoritativeHostProtocolReport.snapshotPresent ? "yes" : "no") << "\n"
            << "Objective: " << simulationDiagnostics.missionObjectiveText << "\n"
            << "Mission phase: " << simulationDiagnostics.missionPhaseText << "\n"
            << "Player tile: (" << playerTile.x << ", " << playerTile.y << ")\n"
            << "Mouse tile: (" << mouseTile.x << ", " << mouseTile.y << ")\n"
            << "Hovered tile: " << hovered << "\n"
            << "Selected tile: " << selected << "\n"
            << "Move target: " << actionTarget << "\n"
            << "Path destination: " << pathDestination << "\n"
            << "Restore state: " << headlessHostPresenceReport.restoreState << "\n"
            << "Runtime mode: " << (runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged") << "\n"
            << "Frame dt: " << lastDeltaTime;

        const std::string infoText = info.str();
        RECT infoRect{ 16, 16, 760, 760 };
        RECT measureRect = infoRect;
        DrawTextA(dc, infoText.c_str(), -1, &measureRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_CALCRECT);
        DrawTextA(dc, infoText.c_str(), -1, &infoRect, DT_LEFT | DT_TOP | DT_NOPREFIX);

        std::ostringstream presentation;
        presentation
            << "Room\n"
            << roomTitle << "\n\n"
            << roomDescription << "\n\n"
            << "Prompt / Vitals\n"
            << promptLine << "\n\n"
            << "Typed Command Bar\n"
            << commandBarText << "\n\n"
            << "Reply\n"
            << commandEcho;

        const std::string presentationText = presentation.str();
        RECT presentationRect{ 800, 16, 1560, 860 };
        DrawTextA(dc, presentationText.c_str(), -1, &presentationRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK);

        int y = measureRect.bottom + 24;
        TextOutA(dc, 16, y, "Event Log:", 10);
        y += 22;

        for (const std::string& entry : eventLog)
        {
            TextOutA(dc, 16, y, entry.c_str(), static_cast<int>(entry.size()));
            y += 18;
        }
    }
}
