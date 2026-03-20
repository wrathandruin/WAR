#include "engine/render/BgfxTileVisuals.h"

namespace war
{
    namespace
    {
        uint32_t rgbaToAbgr(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
        {
            return (static_cast<uint32_t>(a) << 24)
                | (static_cast<uint32_t>(b) << 16)
                | (static_cast<uint32_t>(g) << 8)
                | static_cast<uint32_t>(r);
        }

        int countBlockedNeighbors(const WorldState& worldState, TileCoord tile)
        {
            int count = 0;

            const TileCoord north{ tile.x, tile.y - 1 };
            const TileCoord south{ tile.x, tile.y + 1 };
            const TileCoord west{ tile.x - 1, tile.y };
            const TileCoord east{ tile.x + 1, tile.y };

            count += worldState.world().isBlocked(north) ? 1 : 0;
            count += worldState.world().isBlocked(south) ? 1 : 0;
            count += worldState.world().isBlocked(west) ? 1 : 0;
            count += worldState.world().isBlocked(east) ? 1 : 0;

            return count;
        }

        int coordinateHash(TileCoord tile)
        {
            const int value = tile.x * 17 + tile.y * 31 + tile.x * tile.y * 3;
            return value < 0 ? -value : value;
        }
    }

    BgfxSpriteMaterialId BgfxTileVisuals::materialForTile(
        const WorldState& worldState,
        TileCoord tile)
    {
        const bool blocked = worldState.world().isBlocked(tile);

        if (blocked)
        {
            const int blockedNeighbors = countBlockedNeighbors(worldState, tile);
            if (blockedNeighbors >= 3)
            {
                return BgfxSpriteMaterialId::WallC;
            }

            return ((tile.x + tile.y) & 1) == 0
                ? BgfxSpriteMaterialId::WallA
                : BgfxSpriteMaterialId::WallB;
        }

        const int hash = coordinateHash(tile) % 6;
        if (hash == 0)
        {
            return BgfxSpriteMaterialId::FloorC;
        }

        if (hash <= 2)
        {
            return BgfxSpriteMaterialId::FloorB;
        }

        return BgfxSpriteMaterialId::FloorA;
    }

    uint32_t BgfxTileVisuals::tintForTile(
        const WorldState& worldState,
        TileCoord tile)
    {
        const bool blocked = worldState.world().isBlocked(tile);

        if (blocked)
        {
            const int blockedNeighbors = countBlockedNeighbors(worldState, tile);
            if (blockedNeighbors >= 3)
            {
                return rgbaToAbgr(220, 220, 220);
            }

            return rgbaToAbgr(208, 208, 208);
        }

        const int hash = coordinateHash(tile) % 5;
        if (hash == 0)
        {
            return rgbaToAbgr(235, 235, 235);
        }

        return rgbaToAbgr(255, 255, 255);
    }
}
