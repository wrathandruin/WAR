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
        const std::string hostPath = RuntimePaths::displayPath(headlessHostPresenceReport.statusFilePath);
        const std::string persistentSavePath = RuntimePaths::displayPath(authoritativeHostProtocolReport.persistentSavePath);
        const std::string startupReport = RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath);
        const char* runtimeIssue = runtimeBoundaryReport.issues.empty() ? "none" : runtimeBoundaryReport.issues.front().c_str();
        const char* demoIssue = localDemoDiagnosticsReport.issues.empty() ? "none" : localDemoDiagnosticsReport.issues.front().c_str();
        const char* hostIssue = headlessHostPresenceReport.issues.empty() ? "none" : headlessHostPresenceReport.issues.front().c_str();
        const char* persistenceIssue = simulationDiagnostics.lastPersistenceError.empty()
            ? "none"
            : simulationDiagnostics.lastPersistenceError.c_str();

        std::ostringstream info;
        info
            << "WAR Milestone 37\n"
            << "LMB: move    RMB: interact    Shift+RMB: inspect    MMB drag: pan    Wheel: zoom\n"
            << "Authoring: O region overlay    H hotspot overlay    7/8/9 palette\n"
            << "Harness: J toggle    K latency preset    L jitter preset\n"
            << "Simulation owner: SharedSimulationRuntime with authoritative persistence diagnostics\n"
            << "Local authority active: " << (simulationDiagnostics.localAuthorityActive ? "yes" : "no") << "\n"
            << "Client prediction enabled: " << (simulationDiagnostics.clientPredictionEnabled ? "yes" : "no") << "\n"
            << "Fixed step enabled: " << (simulationDiagnostics.fixedStepEnabled ? "yes" : "no") << "\n"
            << "Fixed step seconds: " << simulationDiagnostics.fixedStepSeconds << "\n"
            << "Accumulator seconds: " << simulationDiagnostics.accumulatorSeconds << "\n"
            << "Presentation alpha: " << simulationDiagnostics.presentationAlpha << "\n"
            << "Rendered frames: " << simulationDiagnostics.renderedFrames << "\n"
            << "Simulation ticks: " << simulationDiagnostics.simulationTicks << "\n"
            << "Latency harness enabled: " << (simulationDiagnostics.latencyHarnessEnabled ? "yes" : "no") << "\n"
            << "Harness latency ms: "
            << simulationDiagnostics.intentLatencyMilliseconds << "/"
            << simulationDiagnostics.acknowledgementLatencyMilliseconds << "/"
            << simulationDiagnostics.snapshotLatencyMilliseconds << "\n"
            << "Harness jitter ms: " << simulationDiagnostics.jitterMilliseconds << "\n"
            << "Intents queued: " << simulationDiagnostics.intentsQueued << "\n"
            << "Intents processed: " << simulationDiagnostics.intentsProcessed << "\n"
            << "Intents acknowledged: " << simulationDiagnostics.intentsAcknowledged << "\n"
            << "Intents rejected: " << simulationDiagnostics.intentsRejected << "\n"
            << "Pending intents: " << simulationDiagnostics.pendingIntentCount << "\n"
            << "Last intent sequence: " << simulationDiagnostics.lastIntentSequence << "\n"
            << "Last snapshot sequence: " << simulationDiagnostics.lastSnapshotSequence << "\n"
            << "Last snapshot sim ticks: " << simulationDiagnostics.lastSnapshotSimulationTicks << "\n"
            << "Last snapshot age ms: " << simulationDiagnostics.lastSnapshotAgeMilliseconds << "\n"
            << "Corrections applied: " << simulationDiagnostics.correctionsApplied << "\n"
            << "Divergence events: " << simulationDiagnostics.divergenceEvents << "\n"
            << "Position drift: " << simulationDiagnostics.lastPositionDivergenceDistance << "\n"
            << "Path divergence: " << (simulationDiagnostics.lastPathDivergence ? "yes" : "no") << "\n"
            << "Entity divergence: " << (simulationDiagnostics.lastEntityDivergence ? "yes" : "no") << "\n"
            << "Snapshot read failures: " << simulationDiagnostics.snapshotReadFailures << "\n"
            << "Last snapshot read failed: " << (simulationDiagnostics.lastSnapshotReadFailed ? "yes" : "no") << "\n"
            << "Last snapshot read error: "
            << (simulationDiagnostics.lastSnapshotReadError.empty() ? "none" : simulationDiagnostics.lastSnapshotReadError)
            << "\n"
            << "Persistence active: " << (simulationDiagnostics.persistenceActive ? "yes" : "no") << "\n"
            << "Persistence slot: " << simulationDiagnostics.persistenceSlotName << "\n"
            << "Persistence schema version: " << simulationDiagnostics.persistenceSchemaVersion << "\n"
            << "Loaded schema version: " << simulationDiagnostics.persistenceLoadedSchemaVersion << "\n"
            << "Migrated from schema version: " << simulationDiagnostics.persistenceMigratedFromSchemaVersion << "\n"
            << "Persistence data loaded: " << (simulationDiagnostics.persistenceDataLoaded ? "yes" : "no") << "\n"
            << "Persistence migration applied: " << (simulationDiagnostics.persistenceMigrationApplied ? "yes" : "no") << "\n"
            << "Persistence save count: " << simulationDiagnostics.persistenceSaveCount << "\n"
            << "Persistence load count: " << simulationDiagnostics.persistenceLoadCount << "\n"
            << "Last persistence save ok: " << (simulationDiagnostics.lastPersistenceSaveSucceeded ? "yes" : "no") << "\n"
            << "Last persistence load ok: " << (simulationDiagnostics.lastPersistenceLoadSucceeded ? "yes" : "no") << "\n"
            << "Last persistence save epoch ms: " << simulationDiagnostics.lastPersistenceSaveEpochMilliseconds << "\n"
            << "Last persistence load epoch ms: " << simulationDiagnostics.lastPersistenceLoadEpochMilliseconds << "\n"
            << "Last persistence error: " << persistenceIssue << "\n"
            << "Headless host file: " << hostPath << "\n"
            << "Headless host online: " << (headlessHostPresenceReport.hostOnline ? "yes" : "no") << "\n"
            << "Host heartbeat fresh: " << (headlessHostPresenceReport.heartbeatFresh ? "yes" : "no") << "\n"
            << "Host heartbeat age ms: " << headlessHostPresenceReport.heartbeatAgeMilliseconds << "\n"
            << "Host mode: " << headlessHostPresenceReport.hostMode << "\n"
            << "Host state: " << headlessHostPresenceReport.hostState << "\n"
            << "Host pid: " << headlessHostPresenceReport.hostPid << "\n"
            << "Host tick ms: " << headlessHostPresenceReport.hostTickMilliseconds << "\n"
            << "Advertised host sim ticks: " << headlessHostPresenceReport.advertisedSimulationTicks << "\n"
            << "Local host lane ready: " << (headlessHostPresenceReport.localBootstrapLaneReady ? "yes" : "no") << "\n"
            << "Host pending inbound intents: " << headlessHostPresenceReport.pendingInboundIntentCount << "\n"
            << "Host pending outbound acks: " << headlessHostPresenceReport.pendingOutboundAcknowledgementCount << "\n"
            << "Host pending snapshots: " << headlessHostPresenceReport.pendingSnapshotCount << "\n"
            << "Host persistence save present: " << (headlessHostPresenceReport.persistenceSavePresent ? "yes" : "no") << "\n"
            << "Host persistence schema version: " << headlessHostPresenceReport.persistenceSchemaVersion << "\n"
            << "Host persistence loaded schema version: " << headlessHostPresenceReport.persistenceLoadedSchemaVersion << "\n"
            << "Host persistence migrated from version: " << headlessHostPresenceReport.persistenceMigratedFromSchemaVersion << "\n"
            << "Host persistence save count: " << headlessHostPresenceReport.persistenceSaveCount << "\n"
            << "Host persistence load count: " << headlessHostPresenceReport.persistenceLoadCount << "\n"
            << "Host persistence last save epoch ms: " << headlessHostPresenceReport.lastPersistenceSaveEpochMilliseconds << "\n"
            << "Host persistence last load epoch ms: " << headlessHostPresenceReport.lastPersistenceLoadEpochMilliseconds << "\n"
            << "Intent queue ready: " << (authoritativeHostProtocolReport.intentQueueReady ? "yes" : "no") << "\n"
            << "Ack queue ready: " << (authoritativeHostProtocolReport.acknowledgementQueueReady ? "yes" : "no") << "\n"
            << "Snapshot present: " << (authoritativeHostProtocolReport.snapshotPresent ? "yes" : "no") << "\n"
            << "Persistent save present: " << (authoritativeHostProtocolReport.persistentSavePresent ? "yes" : "no") << "\n"
            << "Persistent save path: " << persistentSavePath << "\n"
            << "Authority protocol lane ready: " << (authoritativeHostProtocolReport.authorityLaneReady ? "yes" : "no") << "\n"
            << "Player world: (" << playerPosition.x << ", " << playerPosition.y << ")\n"
            << "Player tile: (" << playerTile.x << ", " << playerTile.y << ")\n"
            << "Mouse tile: (" << mouseTile.x << ", " << mouseTile.y << ")\n"
            << "Hovered region: "
            << (hasHoveredTile && worldState.world().isInBounds(hoveredTile)
                    ? WorldRegionTags::debugName(worldState.regionTag(hoveredTile))
                    : "none")
            << "\n"
            << "Hovered blocked: " << (hoveredBlocked ? "yes" : "no") << "\n"
            << "Hovered affordance: " << affordance << "\n"
            << "Prompt: " << prompt << "\n"
            << "Hovered entity: " << (hoveredEntity ? hoveredEntity->name : std::string("none")) << "\n"
            << "Hovered entity type: " << (hoveredEntity ? entityTypeToText(hoveredEntity->type) : "none") << "\n"
            << "Hovered entity state: " << (hoveredEntity ? entityStateText(*hoveredEntity) : "none") << "\n"
            << "Hovered hotspot: " << (hoveredHotspot ? hoveredHotspot->label : std::string("none")) << "\n"
            << "Hovered hotspot type: " << (hoveredHotspot ? hotspotTypeToText(hoveredHotspot->type) : "none") << "\n"
            << "Hovered hotspot state: " << (hoveredHotspot ? hotspotStateText(*hoveredHotspot) : "none") << "\n"
            << "Hovered hotspot summary: " << (hoveredHotspot ? hoveredHotspot->summary : std::string("none")) << "\n"
            << "Selected tile: " << selected << "\n"
            << "Move target: " << actionTarget << "\n"
            << "Path destination: " << pathDestination << "\n"
            << "Runtime mode: " << (runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged") << "\n"
            << "Build: " << localDemoDiagnosticsReport.buildConfiguration << "\n"
            << "Build timestamp: " << localDemoDiagnosticsReport.buildTimestamp << "\n"
            << "Repo root: " << repoRoot << "\n"
            << "Asset root: " << assetRoot << "\n"
            << "Runtime root: " << runtimeRoot << "\n"
            << "Startup report: " << startupReport << "\n"
            << "Packaged lane ready: " << (localDemoDiagnosticsReport.packagedLaneReady ? "yes" : "no") << "\n"
            << "Runtime issue: " << runtimeIssue << "\n"
            << "Demo issue: " << demoIssue << "\n"
            << "Host issue: " << hostIssue << "\n"
            << "Region overlay: " << (worldState.regionOverlayEnabled() ? "on" : "off") << "\n"
            << "Hotspot overlay: " << (worldState.authoringHotspotsVisible() ? "on" : "off") << "\n"
            << "Camera: (" << camera.getPosition().x << ", " << camera.getPosition().y << ")  Zoom: " << camera.getZoom() << "\n"
            << "Path nodes remaining: " << (pathIndex < currentPath.size() ? currentPath.size() - pathIndex : 0) << "\n"
            << "Entities: " << worldState.entities().count() << "\n"
            << "Hotspots: " << worldState.authoringHotspots().size() << "\n"
            << "Frame dt: " << lastDeltaTime;

        const std::string infoText = info.str();
        RECT infoRect{ 16, 16, 1540, 860 };
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
