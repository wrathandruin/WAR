#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "engine/math/Vec2.h"
#include "engine/render/Camera2D.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldState.h"

namespace war
{
    struct BgfxQuad
    {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
        uint32_t color = 0xffffffff;
    };

    struct BgfxRenderLayer
    {
        std::vector<BgfxQuad> quads;
    };

    struct BgfxWorldRenderData
    {
        BgfxRenderLayer tiles;
        BgfxRenderLayer path;
        BgfxRenderLayer hoveredTile;
        BgfxRenderLayer entities;
        BgfxRenderLayer player;
    };

    class BgfxRenderDataBuilder
    {
    public:
        [[nodiscard]] static BgfxWorldRenderData build(
            const WorldState& worldState,
            const Camera2D& camera,
            const Vec2& playerPosition,
            const std::vector<TileCoord>& currentPath,
            size_t pathIndex,
            bool hasHoveredTile,
            TileCoord hoveredTile);
    };
}
