#pragma once

#include <cstddef>
#include <vector>

#include <windows.h>

#include "engine/math/Vec2.h"
#include "engine/render/Camera2D.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldState.h"

namespace war
{
    class WorldRenderer
    {
    public:
        void render(
            HDC dc,
            const RECT& clientRect,
            const WorldState& worldState,
            const Camera2D& camera,
            const Vec2& playerPosition,
            const std::vector<TileCoord>& currentPath,
            size_t pathIndex,
            bool hasHoveredTile,
            TileCoord hoveredTile) const;

    private:
        void drawTiles(HDC dc, const WorldState& worldState, const Camera2D& camera) const;
        void drawHoveredTile(
            HDC dc,
            const WorldState& worldState,
            const Camera2D& camera,
            bool hasHoveredTile,
            TileCoord hoveredTile) const;
        void drawPath(
            HDC dc,
            const WorldState& worldState,
            const Camera2D& camera,
            const std::vector<TileCoord>& currentPath,
            size_t pathIndex) const;
        void drawEntities(HDC dc, const WorldState& worldState, const Camera2D& camera) const;
        void drawPlayer(HDC dc, const Camera2D& camera, const Vec2& playerPosition) const;

        [[nodiscard]] RECT tileToScreenRect(
            const WorldState& worldState,
            const Camera2D& camera,
            TileCoord tile) const;
    };
}
