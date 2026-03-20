#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "engine/gameplay/EntityManager.h"
#include "engine/world/WorldGrid.h"
#include "engine/world/WorldRegionTag.h"

namespace war
{
    enum class WorldAuthoringHotspotType
    {
        Encounter,
        Control,
        Transit,
        Loot,
        Hazard
    };

    struct WorldAuthoringHotspot
    {
        int id = 0;
        TileCoord tile{};
        WorldRegionTagId region = WorldRegionTagId::CargoBay;
        WorldAuthoringHotspotType type = WorldAuthoringHotspotType::Encounter;
        std::string label;
        std::string summary;
        bool encounterReady = false;
    };

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

        void clearAuthoringHotspots();
        void addAuthoringHotspot(const WorldAuthoringHotspot& hotspot);

        [[nodiscard]] const std::vector<WorldAuthoringHotspot>& authoringHotspots() const;
        [[nodiscard]] const WorldAuthoringHotspot* authoringHotspotAt(TileCoord tile) const;

        void setAuthoringHotspotsVisible(bool visible);
        [[nodiscard]] bool authoringHotspotsVisible() const;

    private:
        void fillRegionRect(int minX, int minY, int maxX, int maxY, WorldRegionTagId tag);
        [[nodiscard]] size_t index(TileCoord tile) const;

        WorldGrid m_world;
        EntityManager m_entities;
        std::vector<WorldRegionTagId> m_regionTags;
        bool m_regionOverlayEnabled = true;
        BgfxThemePaletteMode m_paletteMode = BgfxThemePaletteMode::Default;
        std::vector<WorldAuthoringHotspot> m_authoringHotspots;
        bool m_authoringHotspotsVisible = true;
    };
}
