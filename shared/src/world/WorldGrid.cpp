#include "engine/world/WorldGrid.h"

#include <cassert>
#include <cmath>
#include <cstdio>

#include <windows.h>

namespace war
{
    WorldGrid::WorldGrid(int width, int height, int tileSize)
    {
        m_width = width > 0 ? width : 1;
        m_height = height > 0 ? height : 1;
        m_tileSize = tileSize > 0 ? tileSize : 1;

        m_tiles.resize(static_cast<size_t>(m_width * m_height));
    }

    void WorldGrid::generateTestMap()
    {
        assert(m_width > 0);
        assert(m_height > 0);
        assert(!m_tiles.empty());

        char debug[128]{};
        std::snprintf(
            debug,
            sizeof(debug),
            "Grid: %d x %d = %zu tiles\n",
            m_width,
            m_height,
            m_tiles.size());
        OutputDebugStringA(debug);

        for (WorldTile& tile : m_tiles)
        {
            tile.blocked = false;
        }

        auto setBlocked = [this](int x, int y)
        {
            const TileCoord tile{ x, y };
            if (isInBounds(tile))
            {
                m_tiles[index(tile)].blocked = true;
            }
        };

        for (int y = 8; y < 26; ++y)
        {
            setBlocked(10, y);
        }

        for (int x = 14; x < 26; ++x)
        {
            setBlocked(x, 16);
        }

        for (int y = 20; y < 34; ++y)
        {
            setBlocked(28, y);
        }

        for (int x = 22; x < 40; ++x)
        {
            setBlocked(x, 30);
        }

        for (int y = 6; y < 14; ++y)
        {
            for (int x = 34; x < 40; ++x)
            {
                setBlocked(x, y);
            }
        }

        for (int x = 41; x <= 47; ++x)
        {
            setBlocked(x, 1);
            setBlocked(x, 6);
        }

        for (int y = 1; y <= 6; ++y)
        {
            setBlocked(41, y);
            setBlocked(47, y);
        }
    }

    int WorldGrid::getWidth() const
    {
        return m_width;
    }

    int WorldGrid::getHeight() const
    {
        return m_height;
    }

    int WorldGrid::getTileSize() const
    {
        return m_tileSize;
    }

    bool WorldGrid::isInBounds(TileCoord tile) const
    {
        return tile.x >= 0 && tile.y >= 0 && tile.x < m_width && tile.y < m_height;
    }

    bool WorldGrid::isBlocked(TileCoord tile) const
    {
        if (!isInBounds(tile))
        {
            return true;
        }

        return m_tiles[index(tile)].blocked;
    }

    bool WorldGrid::isWalkable(TileCoord tile) const
    {
        return isInBounds(tile) && !m_tiles[index(tile)].blocked;
    }

    void WorldGrid::setBlocked(TileCoord tile, bool blocked)
    {
        if (!isInBounds(tile))
        {
            return;
        }

        m_tiles[index(tile)].blocked = blocked;
    }

    TileCoord WorldGrid::worldToTile(const Vec2& world) const
    {
        const float halfWorldWidth = static_cast<float>(m_width * m_tileSize) * 0.5f;
        const float halfWorldHeight = static_cast<float>(m_height * m_tileSize) * 0.5f;

        const float localX = world.x + halfWorldWidth;
        const float localY = world.y + halfWorldHeight;

        return {
            static_cast<int>(std::floor(localX / static_cast<float>(m_tileSize))),
            static_cast<int>(std::floor(localY / static_cast<float>(m_tileSize)))
        };
    }

    Vec2 WorldGrid::tileToWorldCenter(TileCoord tile) const
    {
        const float halfWorldWidth = static_cast<float>(m_width * m_tileSize) * 0.5f;
        const float halfWorldHeight = static_cast<float>(m_height * m_tileSize) * 0.5f;

        return {
            static_cast<float>(tile.x * m_tileSize + m_tileSize / 2) - halfWorldWidth,
            static_cast<float>(tile.y * m_tileSize + m_tileSize / 2) - halfWorldHeight
        };
    }

    size_t WorldGrid::index(TileCoord tile) const
    {
        return static_cast<size_t>(tile.y) * static_cast<size_t>(m_width)
             + static_cast<size_t>(tile.x);
    }
}
