#pragma once

#include <string>
#include <vector>

#include <windows.h>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/host/HeadlessHostPresence.h"
#include "engine/math/Vec2.h"
#include "engine/render/Camera2D.h"
#include "engine/simulation/SimulationIntent.h"
#include "engine/world/WorldState.h"

namespace war
{
    class DebugOverlayRenderer
    {
    public:
        void render(
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
            const HeadlessHostPresenceReport& headlessHostPresenceReport) const;

    private:
        [[nodiscard]] const char* entityTypeToText(EntityType type) const;
        [[nodiscard]] const char* hotspotTypeToText(WorldAuthoringHotspotType type) const;
    };
}
