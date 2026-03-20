#pragma once

#include <string>
#include <vector>

#include "engine/gameplay/ActionQueue.h"
#include "engine/math/Vec2.h"
#include "engine/simulation/SimulationIntent.h"
#include "engine/world/WorldState.h"

namespace war
{
    class ActionSystem
    {
    public:
        static void processPending(
            WorldState& worldState,
            ActionQueue& actions,
            PlayerActorRuntimeState& playerActorState,
            const Vec2& playerPosition,
            std::vector<TileCoord>& currentPath,
            size_t& pathIndex,
            std::vector<std::string>& eventLog);

    private:
        static void pushEvent(std::vector<std::string>& eventLog, const std::string& message);
        static void rebuildPathTo(
            WorldState& worldState,
            const Vec2& playerPosition,
            TileCoord targetTile,
            std::vector<TileCoord>& currentPath,
            size_t& pathIndex,
            std::vector<std::string>& eventLog);

        [[nodiscard]] static const char* entityTypeToText(EntityType type);
    };
}
