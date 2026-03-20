#include "engine/render/BgfxTileVisuals.h"

#include "engine/render/BgfxWorldTheme.h"

namespace war
{
    namespace
    {
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
        const BgfxWorldThemeId theme = worldState.visualThemeForTile(tile);
        const bool blocked = worldState.world().isBlocked(tile);

        if (blocked)
        {
            return BgfxWorldTheme::wallMaterial(theme);
        }

        return BgfxWorldTheme::floorMaterial(theme);
    }

    uint32_t BgfxTileVisuals::tintForTile(
        const WorldState& worldState,
        TileCoord tile)
    {
        const BgfxWorldThemeId theme = worldState.visualThemeForTile(tile);
        const BgfxThemePaletteMode paletteMode = worldState.paletteMode();
        const bool blocked = worldState.world().isBlocked(tile);

        if (blocked)
        {
            const int tileVariant = countBlockedNeighbors(worldState, tile) >= 3 ? 1 : 0;
            return BgfxWorldTheme::wallTint(theme, paletteMode, tileVariant);
        }

        const int tileVariant = coordinateHash(tile) % 2;
        return BgfxWorldTheme::floorTint(theme, paletteMode, tileVariant);
    }
}
