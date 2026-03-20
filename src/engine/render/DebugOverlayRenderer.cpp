#include "engine/render/DebugOverlayRenderer.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace war
{
    namespace
    {
        const char* entityStateText(const Entity& entity)
        {
            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen ? "open" : "closed";

            case EntityType::Terminal:
                return entity.isPowered ? "powered" : "offline";

            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return "locked";
                }
                return entity.isOpen ? "open" : "closed";

            default:
                return "unknown";
            }
        }

        const char* hotspotStateText(const WorldAuthoringHotspot& hotspot)
        {
            return hotspot.encounterReady ? "encounter-ready" : "staged";
        }

        std::string tileText(bool hasTile, TileCoord tile)
        {
            if (!hasTile)
            {
                return "none";
            }

            return "(" + std::to_string(tile.x) + ", " + std::to_string(tile.y) + ")";
        }

        std::string hoverAffordance(
            const WorldState& worldState,
            bool hasHoveredTile,
            TileCoord hoveredTile,
            const Entity* hoveredEntity,
            const WorldAuthoringHotspot* hoveredHotspot)
        {
            if (!hasHoveredTile || !worldState.world().isInBounds(hoveredTile))
            {
                return "none";
            }

            if (worldState.world().isBlocked(hoveredTile))
            {
                return "blocked terrain";
            }

            if (hoveredEntity != nullptr)
            {
                return std::string("interactable ") + hoveredEntity->name;
            }

            if (hoveredHotspot != nullptr)
            {
                return std::string("authored hotspot ") + hoveredHotspot->label;
            }

            return "walkable terrain";
        }

        std::string hoverPrompt(
            const WorldState& worldState,
            bool hasHoveredTile,
            TileCoord hoveredTile,
            const Entity* hoveredEntity,
            const WorldAuthoringHotspot* hoveredHotspot)
        {
            if (!hasHoveredTile || !worldState.world().isInBounds(hoveredTile))
            {
                return "Move the cursor onto the world to preview actions.";
            }

            if (worldState.world().isBlocked(hoveredTile))
            {
                return "Blocked tile. Shift+RMB inspect for detail.";
            }

            if (hoveredEntity != nullptr)
            {
                return std::string("RMB interact with ") + hoveredEntity->name + " | Shift+RMB inspect";
            }

            if (hoveredHotspot != nullptr)
            {
                return std::string("RMB use ") + hoveredHotspot->label + " | Shift+RMB inspect hotspot";
            }

            return "LMB move | Shift+RMB inspect terrain";
        }
    }

    const char* DebugOverlayRenderer::entityTypeToText(EntityType type) const
    {
        switch (type)
        {
        case EntityType::Crate:
            return "crate";
        case EntityType::Terminal:
            return "terminal";
        case EntityType::Locker:
            return "locker";
        default:
            return "unknown";
        }
    }

    const char* DebugOverlayRenderer::hotspotTypeToText(WorldAuthoringHotspotType type) const
    {
        switch (type)
        {
        case WorldAuthoringHotspotType::Encounter:
            return "encounter";
        case WorldAuthoringHotspotType::Control:
            return "control";
        case WorldAuthoringHotspotType::Transit:
            return "transit";
        case WorldAuthoringHotspotType::Loot:
            return "loot";
        case WorldAuthoringHotspotType::Hazard:
            return "hazard";
        default:
            return "unknown";
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
        const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport) const
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(225, 225, 225));

        const Vec2 mouseWorld = camera.screenToWorld(mousePosition.x, mousePosition.y);
        const TileCoord mouseTile = worldState.world().worldToTile(mouseWorld);
        const TileCoord playerTile = worldState.world().worldToTile(playerPosition);
        const bool hoveredBlocked =
            hasHoveredTile && worldState.world().isInBounds(hoveredTile) && worldState.world().isBlocked(hoveredTile);
        const Entity* hoveredEntity =
            hasHoveredTile && worldState.world().isInBounds(hoveredTile)
            ? worldState.entities().getAt(hoveredTile)
            : nullptr;
        const WorldAuthoringHotspot* hoveredHotspot =
            hasHoveredTile && worldState.world().isInBounds(hoveredTile)
            ? worldState.authoringHotspotAt(hoveredTile)
            : nullptr;

        const std::string affordance = hoverAffordance(worldState, hasHoveredTile, hoveredTile, hoveredEntity, hoveredHotspot);
        const std::string prompt = hoverPrompt(worldState, hasHoveredTile, hoveredTile, hoveredEntity, hoveredHotspot);
        const std::string selected = tileText(hasSelectedTile, selectedTile);
        const std::string actionTarget = tileText(hasActionTargetTile, actionTargetTile);
        const std::string pathDestination =
            currentPath.empty() || pathIndex >= currentPath.size()
            ? "none"
            : tileText(true, currentPath.back());
        const std::string repoRoot = RuntimePaths::displayPath(runtimeBoundaryReport.repoRoot);
        const std::string assetRoot = RuntimePaths::displayPath(runtimeBoundaryReport.assetRoot);
        const std::string runtimeRoot = RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot);
        const std::string configRoot = RuntimePaths::displayPath(runtimeBoundaryReport.configDirectory);
        const std::string logsRoot = RuntimePaths::displayPath(runtimeBoundaryReport.logsDirectory);
        const std::string savesRoot = RuntimePaths::displayPath(runtimeBoundaryReport.savesDirectory);
        const std::string crashRoot = RuntimePaths::displayPath(runtimeBoundaryReport.crashDirectory);
        const std::string startupReport = RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath);
        const std::string suggestedPackageRoot = RuntimePaths::displayPath(localDemoDiagnosticsReport.suggestedPackageRoot);
        const char* runtimeIssue = runtimeBoundaryReport.issues.empty() ? "none" : runtimeBoundaryReport.issues.front().c_str();
        const char* demoIssue = localDemoDiagnosticsReport.issues.empty() ? "none" : localDemoDiagnosticsReport.issues.front().c_str();

        char buffer[4096]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "WAR Milestone 32"
            "LMB: move    RMB: interact    Shift+RMB: inspect    MMB drag: pan    Wheel: zoom"
            "Authoring: O region overlay    H hotspot overlay    7/8/9 palette"
            "Player world: (%.1f, %.1f)"
            "Player tile: (%d, %d)"
            "Mouse tile: (%d, %d)"
            "Hovered region: %s"
            "Hovered blocked: %s"
            "Hovered affordance: %s"
            "Prompt: %s"
            "Hovered entity: %s"
            "Hovered entity type: %s"
            "Hovered entity state: %s"
            "Hovered hotspot: %s"
            "Hovered hotspot type: %s"
            "Hovered hotspot state: %s"
            "Hovered hotspot summary: %s"
            "Selected tile: %s"
            "Move target: %s"
            "Path destination: %s"
            "Runtime mode: %s"
            "Build: %s"
            "Build timestamp: %s"
            "Repo root: %s"
            "Asset root: %s"
            "Runtime root: %s"
            "Runtime dirs: cfg=%s  log=%s"
            "Runtime dirs: save=%s  crash=%s"
            "Startup report: %s"
            "Suggested package root: %s"
            "Repo packaging scripts: %s"
            "Packaged assets ready: %s"
            "Packaged runtime ready: %s"
            "Launch script present: %s"
            "Smoke script present: %s"
            "Packaged lane ready: %s"
            "Runtime issue: %s"
            "Demo issue: %s"
            "Region overlay: %s"
            "Hotspot overlay: %s"
            "Camera: (%.1f, %.1f)  Zoom: %.2f"
            "Path nodes remaining: %zu"
            "Entities: %zu"
            "Hotspots: %zu"
            "Frame dt: %.4f",
            playerPosition.x,
            playerPosition.y,
            playerTile.x,
            playerTile.y,
            mouseTile.x,
            mouseTile.y,
            hasHoveredTile && worldState.world().isInBounds(hoveredTile)
            ? WorldRegionTags::debugName(worldState.regionTag(hoveredTile))
            : "none",
            hoveredBlocked ? "yes" : "no",
            affordance.c_str(),
            prompt.c_str(),
            hoveredEntity ? hoveredEntity->name.c_str() : "none",
            hoveredEntity ? entityTypeToText(hoveredEntity->type) : "none",
            hoveredEntity ? entityStateText(*hoveredEntity) : "none",
            hoveredHotspot ? hoveredHotspot->label.c_str() : "none",
            hoveredHotspot ? hotspotTypeToText(hoveredHotspot->type) : "none",
            hoveredHotspot ? hotspotStateText(*hoveredHotspot) : "none",
            hoveredHotspot ? hoveredHotspot->summary.c_str() : "none",
            selected.c_str(),
            actionTarget.c_str(),
            pathDestination.c_str(),
            runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged",
            localDemoDiagnosticsReport.buildConfiguration.c_str(),
            localDemoDiagnosticsReport.buildTimestamp.c_str(),
            repoRoot.c_str(),
            assetRoot.c_str(),
            runtimeRoot.c_str(),
            configRoot.c_str(),
            logsRoot.c_str(),
            savesRoot.c_str(),
            crashRoot.c_str(),
            startupReport.c_str(),
            suggestedPackageRoot.c_str(),
            localDemoDiagnosticsReport.repoPackagingScriptsReady ? "yes" : "no",
            localDemoDiagnosticsReport.packageAssetsReady ? "yes" : "no",
            localDemoDiagnosticsReport.packageRuntimeReady ? "yes" : "no",
            localDemoDiagnosticsReport.launchScriptPresent ? "yes" : "no",
            localDemoDiagnosticsReport.smokeScriptPresent ? "yes" : "no",
            localDemoDiagnosticsReport.packagedLaneReady ? "yes" : "no",
            runtimeIssue,
            demoIssue,
            worldState.regionOverlayEnabled() ? "on" : "off",
            worldState.authoringHotspotsVisible() ? "on" : "off",
            camera.getPosition().x,
            camera.getPosition().y,
            camera.getZoom(),
            pathIndex < currentPath.size() ? currentPath.size() - pathIndex : 0,
            worldState.entities().count(),
            worldState.authoringHotspots().size(),
            lastDeltaTime);

        TextOutA(dc, 16, 16, buffer, static_cast<int>(std::strlen(buffer)));

        int y = 700;
        TextOutA(dc, 16, y, "Event Log:", 10);
        y += 22;

        for (const std::string& entry : eventLog)
        {
            TextOutA(dc, 16, y, entry.c_str(), static_cast<int>(entry.size()));
            y += 18;
        }
    }
}
