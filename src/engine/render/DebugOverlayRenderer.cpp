#include "engine/render/DebugOverlayRenderer.h"

#include <sstream>
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
            case EntityType::Ship:
                return entity.isPowered ? "powered, docked" : "cold-docked";
            default:
                return "unknown";
            }
        }

        const char* hotspotStateText(const WorldAuthoringHotspot& hotspot)
        {
            switch (hotspot.type)
            {
            case WorldAuthoringHotspotType::Encounter:
                return hotspot.encounterReady ? "encounter-ready" : "inactive";
            case WorldAuthoringHotspotType::Control:
                return hotspot.encounterReady ? "ready" : "gated";
            case WorldAuthoringHotspotType::Transit:
                return hotspot.encounterReady ? "open" : "gated";
            case WorldAuthoringHotspotType::Loot:
                return hotspot.encounterReady ? "available" : "cleared";
            case WorldAuthoringHotspotType::Hazard:
                return hotspot.encounterReady ? "active" : "stabilized";
            default:
                return "unknown";
            }
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
        const AuthoritativeHostProtocolReport& authoritativeHostProtocolReport) const
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(225, 225, 225));

        const Vec2 mouseWorld = camera.screenToWorld(mousePosition.x, mousePosition.y);
        const TileCoord mouseTile = worldState.world().worldToTile(mouseWorld);
        const TileCoord playerTile = worldState.world().worldToTile(playerPosition);
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

        std::ostringstream info;
        info
            << "WAR Milestone 44\n"
            << "LMB: move    RMB: interact    Shift+RMB: inspect    MMB drag: pan    Wheel: zoom\n"
            << "Authoritative lane: " << (simulationDiagnostics.hostAuthorityActive ? "headless host" : "local") << "\n"
            << "Objective: " << simulationDiagnostics.missionObjectiveText << "\n"
            << "Mission phase: " << simulationDiagnostics.missionPhaseText << "\n"
            << "Mission beat: " << simulationDiagnostics.missionLastBeat << "\n"
            << "Mission complete: " << (simulationDiagnostics.missionComplete ? "yes" : "no") << "\n"
            << "Gate locked: " << (simulationDiagnostics.missionGateLocked ? "yes" : "no") << "\n"
            << "Ship runtime prep: " << (simulationDiagnostics.shipRuntimePrepReady ? "yes" : "no") << "\n"
            << "Active ship: " << simulationDiagnostics.shipName << " [" << simulationDiagnostics.activeShipId << "]\n"
            << "Ship ownership / occupancy: " << simulationDiagnostics.shipOwnershipText << " / " << simulationDiagnostics.shipOccupancyText << "\n"
            << "Ship docked / boarded: " << (simulationDiagnostics.shipDocked ? "yes" : "no") << " / " << (simulationDiagnostics.shipBoarded ? "yes" : "no") << "\n"
            << "Ship power / airlock / command: " << (simulationDiagnostics.shipPowerOnline ? "yes" : "no") << " / " << (simulationDiagnostics.shipAirlockPressurized ? "yes" : "no") << " / " << (simulationDiagnostics.shipCommandClaimed ? "yes" : "no") << "\n"
            << "Ship launch prep: " << (simulationDiagnostics.shipLaunchPrepReady ? "yes" : "no") << "\n"
            << "Ship beat: " << simulationDiagnostics.shipLastBeat << "\n"
            << "Orbital active / departure auth: " << (simulationDiagnostics.orbitalLayerActive ? "yes" : "no") << " / " << (simulationDiagnostics.orbitalDepartureAuthorized ? "yes" : "no") << "\n"
            << "Orbital phase: " << simulationDiagnostics.orbitalPhaseText << "\n"
            << "Orbital node / target: " << simulationDiagnostics.orbitalCurrentNodeText << " / " << simulationDiagnostics.orbitalTargetNodeText << "\n"
            << "Orbital travel / ticks: " << (simulationDiagnostics.orbitalTravelInProgress ? "yes" : "no") << " / " << simulationDiagnostics.orbitalTravelTicksRemaining << "\n"
            << "Orbital rule: " << simulationDiagnostics.orbitalRuleText << "\n"
            << "Orbital beat: " << simulationDiagnostics.orbitalLastBeat << "\n"
            << "Runtime context: " << simulationDiagnostics.playerRuntimeContextText << "\n"
            << "Player world: (" << playerPosition.x << ", " << playerPosition.y << ")\n"
            << "Player tile: (" << playerTile.x << ", " << playerTile.y << ")\n"
            << "Mouse tile: (" << mouseTile.x << ", " << mouseTile.y << ")\n"
            << "Hovered affordance: " << affordance << "\n"
            << "Prompt: " << prompt << "\n"
            << "Selected tile: " << selected << "\n"
            << "Move target: " << actionTarget << "\n"
            << "Path destination: " << pathDestination << "\n"
            << "Combat active: " << (simulationDiagnostics.combatActive ? "yes" : "no") << "\n"
            << "Combat label: " << simulationDiagnostics.currentCombatLabel << "\n"
            << "Combat round / ticks: " << simulationDiagnostics.combatRoundNumber << " / " << simulationDiagnostics.combatTicksRemaining << "\n"
            << "Hostile: " << simulationDiagnostics.hostileLabel << " " << simulationDiagnostics.hostileHealth << "/" << simulationDiagnostics.hostileMaxHealth << "\n"
            << "Health / armor / suit: " << simulationDiagnostics.playerHealth << "/" << simulationDiagnostics.playerArmor << "/" << simulationDiagnostics.suitIntegrity << "\n"
            << "Oxygen / radiation / toxic: " << simulationDiagnostics.oxygenSecondsRemaining << " / " << simulationDiagnostics.radiationDose << " / " << simulationDiagnostics.toxicExposure << "\n"
            << "Hazard: " << simulationDiagnostics.currentHazardLabel << "\n"
            << "Terrain consequence: " << simulationDiagnostics.currentTerrainConsequence << "\n"
            << "Inventory stacks/items: " << simulationDiagnostics.inventoryStackCount << "/" << simulationDiagnostics.inventoryItemCount << "\n"
            << "Equipped weapon: " << simulationDiagnostics.equippedWeaponText << "\n"
            << "Equipped suit: " << simulationDiagnostics.equippedSuitText << "\n"
            << "Equipped tool: " << simulationDiagnostics.equippedToolText << "\n"
            << "Loot collections: " << simulationDiagnostics.lootCollections << "\n"
            << "Encounter wins / survived: " << simulationDiagnostics.encounterWins << " / " << simulationDiagnostics.encountersSurvived << "\n"
            << "Combat rounds resolved: " << simulationDiagnostics.combatRoundsResolved << "\n"
            << "Snapshot age ms: " << simulationDiagnostics.lastSnapshotAgeMilliseconds << "\n"
            << "Host online: " << (headlessHostPresenceReport.hostOnline ? "yes" : "no") << "\n"
            << "Protocol lane ready: " << (authoritativeHostProtocolReport.authorityLaneReady ? "yes" : "no") << "\n"
            << "Build: " << localDemoDiagnosticsReport.buildConfiguration << " | " << localDemoDiagnosticsReport.buildTimestamp << "\n"
            << "Runtime mode: " << (runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged") << "\n"
            << "Frame dt: " << lastDeltaTime;

        const std::string infoText = info.str();
        RECT infoRect{ 16, 16, 1540, 760 };
        RECT measureRect = infoRect;
        DrawTextA(dc, infoText.c_str(), -1, &measureRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_CALCRECT);
        DrawTextA(dc, infoText.c_str(), -1, &infoRect, DT_LEFT | DT_TOP | DT_NOPREFIX);

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
