#include "engine/world/WorldState.h"

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/world/WorldSemanticDressing.h"

namespace war
{
    namespace
    {
        void appendWorldInitTrace(std::string_view line)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "simulation_init_trace.txt", line);
        }
    }

    WorldState::WorldState()
        : m_world(48, 36, 48)
    {
        m_regionTags.resize(
            static_cast<size_t>(m_world.getWidth()) * static_cast<size_t>(m_world.getHeight()),
            WorldRegionTagId::CargoBay);
    }

    void WorldState::initializeTestWorld()
    {
        appendWorldInitTrace("WorldState::initializeTestWorld entered");
        m_world.generateTestMap();
        appendWorldInitTrace("WorldState::initializeTestWorld map generated");
        m_regionOverlayEnabled = true;
        m_paletteMode = BgfxThemePaletteMode::Default;
        m_authoringHotspotsVisible = true;
        clearAuthoringHotspots();
        clearTerrainHazards();

        for (int y = 0; y < m_world.getHeight(); ++y)
        {
            for (int x = 0; x < m_world.getWidth(); ++x)
            {
                setRegionTag({ x, y }, WorldRegionTagId::CargoBay);
            }
        }

        fillRegionRect(0, 14, 47, 18, WorldRegionTagId::TransitSpine);
        fillRegionRect(20, 8, 23, 28, WorldRegionTagId::TransitSpine);
        fillRegionRect(13, 4, 25, 11, WorldRegionTagId::MedLab);
        fillRegionRect(30, 4, 42, 12, WorldRegionTagId::CommandDeck);
        fillRegionRect(27, 22, 43, 32, WorldRegionTagId::HazardContainment);
        fillRegionRect(19, 24, 26, 29, WorldRegionTagId::HazardContainment);
        appendWorldInitTrace("WorldState::initializeTestWorld regions assigned");

        WorldSemanticDressing::populateAuthoredTestWorld(*this);
        appendWorldInitTrace("WorldState::initializeTestWorld semantic dressing populated");
    }

    WorldGrid& WorldState::world()
    {
        return m_world;
    }

    const WorldGrid& WorldState::world() const
    {
        return m_world;
    }

    EntityManager& WorldState::entities()
    {
        return m_entities;
    }

    const EntityManager& WorldState::entities() const
    {
        return m_entities;
    }

    void WorldState::setRegionTag(TileCoord tile, WorldRegionTagId tag)
    {
        if (!m_world.isInBounds(tile))
        {
            return;
        }

        m_regionTags[index(tile)] = tag;
    }

    WorldRegionTagId WorldState::regionTag(TileCoord tile) const
    {
        if (!m_world.isInBounds(tile))
        {
            return WorldRegionTagId::CargoBay;
        }

        return m_regionTags[index(tile)];
    }

    BgfxWorldThemeId WorldState::visualThemeForTile(TileCoord tile) const
    {
        return WorldRegionTags::themeFor(regionTag(tile));
    }

    void WorldState::setRegionOverlayEnabled(bool enabled)
    {
        m_regionOverlayEnabled = enabled;
    }

    bool WorldState::regionOverlayEnabled() const
    {
        return m_regionOverlayEnabled;
    }

    void WorldState::setPaletteMode(BgfxThemePaletteMode paletteMode)
    {
        m_paletteMode = paletteMode;
    }

    BgfxThemePaletteMode WorldState::paletteMode() const
    {
        return m_paletteMode;
    }

    void WorldState::clearAuthoringHotspots()
    {
        m_authoringHotspots.clear();
    }

    void WorldState::addAuthoringHotspot(const WorldAuthoringHotspot& hotspot)
    {
        m_authoringHotspots.push_back(hotspot);
    }

    const std::vector<WorldAuthoringHotspot>& WorldState::authoringHotspots() const
    {
        return m_authoringHotspots;
    }

    const WorldAuthoringHotspot* WorldState::authoringHotspotAt(TileCoord tile) const
    {
        for (const WorldAuthoringHotspot& hotspot : m_authoringHotspots)
        {
            if (hotspot.tile == tile)
            {
                return &hotspot;
            }
        }

        return nullptr;
    }

    WorldAuthoringHotspot* WorldState::authoringHotspotAtMutable(TileCoord tile)
    {
        for (WorldAuthoringHotspot& hotspot : m_authoringHotspots)
        {
            if (hotspot.tile == tile)
            {
                return &hotspot;
            }
        }

        return nullptr;
    }

    void WorldState::setAuthoringHotspotsVisible(bool visible)
    {
        m_authoringHotspotsVisible = visible;
    }

    bool WorldState::authoringHotspotsVisible() const
    {
        return m_authoringHotspotsVisible;
    }

    void WorldState::clearTerrainHazards()
    {
        m_terrainHazards.clear();
    }

    void WorldState::addTerrainHazard(const TerrainHazardTile& hazard)
    {
        m_terrainHazards.push_back(hazard);
    }

    const std::vector<TerrainHazardTile>& WorldState::terrainHazards() const
    {
        return m_terrainHazards;
    }

    const TerrainHazardTile* WorldState::terrainHazardAt(TileCoord tile) const
    {
        for (const TerrainHazardTile& hazard : m_terrainHazards)
        {
            if (hazard.active && hazard.tile == tile)
            {
                return &hazard;
            }
        }

        return nullptr;
    }

    void WorldState::fillRegionRect(int minX, int minY, int maxX, int maxY, WorldRegionTagId tag)
    {
        if (minX > maxX || minY > maxY)
        {
            return;
        }

        for (int y = minY; y <= maxY; ++y)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                setRegionTag({ x, y }, tag);
            }
        }
    }

    size_t WorldState::index(TileCoord tile) const
    {
        return static_cast<size_t>(tile.y) * static_cast<size_t>(m_world.getWidth())
             + static_cast<size_t>(tile.x);
    }
}
