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
        std::string locationKey;
        std::string locationTitle;
        std::string locationDescription;
        bool encounterReady = false;
    };

    struct WorldRegionDescription
    {
        WorldRegionTagId region = WorldRegionTagId::CargoBay;
        std::string key;
        std::string title;
        std::string entryDescription;
    };

    struct ResolvedLocationContext
    {
        bool valid = false;
        std::string key;
        std::string title;
        std::string entryDescription;
    };

    enum class TerrainHazardType
    {
        None,
        VacuumLeak,
        RadiationLeak,
        ToxicSpill,
        ElectricalArc
    };

    struct TerrainHazardTile
    {
        int id = 0;
        TileCoord tile{};
        WorldRegionTagId region = WorldRegionTagId::CargoBay;
        TerrainHazardType type = TerrainHazardType::None;
        std::string label;
        int severity = 0;
        bool active = true;
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

        void clearRegionDescriptions();
        void setRegionDescription(
            WorldRegionTagId region,
            const std::string& key,
            const std::string& title,
            const std::string& entryDescription);
        [[nodiscard]] const WorldRegionDescription* regionDescription(WorldRegionTagId region) const;
        [[nodiscard]] ResolvedLocationContext resolveInteriorLocation(TileCoord tile) const;

        void clearAuthoringHotspots();
        void addAuthoringHotspot(const WorldAuthoringHotspot& hotspot);
        [[nodiscard]] const std::vector<WorldAuthoringHotspot>& authoringHotspots() const;
        [[nodiscard]] const WorldAuthoringHotspot* authoringHotspotAt(TileCoord tile) const;
        [[nodiscard]] WorldAuthoringHotspot* authoringHotspotAtMutable(TileCoord tile);

        void setAuthoringHotspotsVisible(bool visible);
        [[nodiscard]] bool authoringHotspotsVisible() const;

        void clearTerrainHazards();
        void addTerrainHazard(const TerrainHazardTile& hazard);
        [[nodiscard]] const std::vector<TerrainHazardTile>& terrainHazards() const;
        [[nodiscard]] const TerrainHazardTile* terrainHazardAt(TileCoord tile) const;

    private:
        void fillRegionRect(int minX, int minY, int maxX, int maxY, WorldRegionTagId tag);
        [[nodiscard]] size_t index(TileCoord tile) const;

        WorldGrid m_world;
        EntityManager m_entities;
        std::vector<WorldRegionTagId> m_regionTags;
        bool m_regionOverlayEnabled = true;
        BgfxThemePaletteMode m_paletteMode = BgfxThemePaletteMode::Default;
        std::vector<WorldRegionDescription> m_regionDescriptions;
        std::vector<WorldAuthoringHotspot> m_authoringHotspots;
        bool m_authoringHotspotsVisible = true;
        std::vector<TerrainHazardTile> m_terrainHazards;
    };
}
