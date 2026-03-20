#pragma once

#include <cstddef>
#include <vector>

#include "engine/math/Vec2.h"
#include "engine/render/Camera2D.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldState.h"

namespace war
{
    class BgfxWorldRenderer
    {
    public:
        [[nodiscard]] bool render(
            const WorldState& worldState,
            const Camera2D& camera,
            const Vec2& playerPosition,
            const std::vector<TileCoord>& currentPath,
            size_t pathIndex,
            bool hasHoveredTile,
            TileCoord hoveredTile) const;

        [[nodiscard]] const char* statusMessage() const;
    };
}
