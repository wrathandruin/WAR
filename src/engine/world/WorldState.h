#pragma once

#include <cstddef>
#include <vector>

#include "engine/gameplay/EntityManager.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldRegionTag.h"

namespace war
{
    class WorldState
    {
    public:
        WorldState();

        void initializeTestWorld();

        [[nodiscard]] WorldGrid& world();
        [[nodiscard]] const WorldGrid& world() const;

        [[nodiscard]] EntityManager& entities();
        [[nodiscard]] const EntityManager& entities() const;

        void setRegionTag(TileCoord tile, WorldRegionTagId tag);
        [[nodiscard]] WorldRegionTagId regionTag(TileCoord tile) const;
        [[nodiscard]] BgfxWorldThemeId visualThemeForTile(TileCoord tile) const;

        void setRegionOverlayEnabled(bool enabled);
        [[nodiscard]] bool regionOverlayEnabled() const;

        void setPaletteMode(BgfxThemePaletteMode paletteMode);
        [[nodiscard]] BgfxThemePaletteMode paletteMode() const;

    private:
        void fillRegionRect(int minX, int minY, int maxX, int maxY, WorldRegionTagId tag);
        [[nodiscard]] size_t index(TileCoord tile) const;

        WorldGrid m_world;
        EntityManager m_entities;
        std::vector<WorldRegionTagId> m_regionTags;
        bool m_regionOverlayEnabled = true;
        BgfxThemePaletteMode m_paletteMode = BgfxThemePaletteMode::Default;
    };
}
