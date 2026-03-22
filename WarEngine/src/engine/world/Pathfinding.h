#pragma once

#include <vector>

#include "engine/world/WorldGrid.h"

namespace war
{
    class Pathfinding
    {
    public:
        [[nodiscard]] static std::vector<TileCoord> findPath(
            const WorldGrid& grid,
            TileCoord start,
            TileCoord goal);
    };
}
