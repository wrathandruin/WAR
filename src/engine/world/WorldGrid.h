#pragma once

#include <cstddef>
#include <vector>

#include "engine/math/Vec2.h"

namespace war
{
    struct TileCoord
    {
        int x = 0;
        int y = 0;

        bool operator==(const TileCoord& rhs) const = default;
    };

    struct WorldTile
    {
        bool blocked = false;
    };

    class WorldGrid
    {
    public:
        WorldGrid(int width, int height, int tileSize);

        void generateTestMap();

        [[nodiscard]] int getWidth() const;
        [[nodiscard]] int getHeight() const;
        [[nodiscard]] int getTileSize() const;

        [[nodiscard]] bool isInBounds(TileCoord tile) const;
        [[nodiscard]] bool isBlocked(TileCoord tile) const;
        [[nodiscard]] bool isWalkable(TileCoord tile) const;
        void setBlocked(TileCoord tile, bool blocked);

        [[nodiscard]] TileCoord worldToTile(const Vec2& world) const;
        [[nodiscard]] Vec2 tileToWorldCenter(TileCoord tile) const;

    private:
        [[nodiscard]] size_t index(TileCoord tile) const;

        int m_width = 1;
        int m_height = 1;
        int m_tileSize = 1;
        std::vector<WorldTile> m_tiles;
    };
}
