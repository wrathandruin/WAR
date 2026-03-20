#include "engine/world/WorldState.h"

#include "engine/gameplay/Entity.h"

namespace war
{
    WorldState::WorldState()
        : m_world(48, 36, 48)
    {
        m_regionTags.resize(
            static_cast<size_t>(m_world.getWidth()) * static_cast<size_t>(m_world.getHeight()),
            WorldRegionTagId::IndustrialZone);
    }

    void WorldState::initializeTestWorld()
    {
        m_world.generateTestMap();
        m_regionOverlayEnabled = true;
        m_paletteMode = BgfxThemePaletteMode::Default;

        for (int y = 0; y < m_world.getHeight(); ++y)
        {
            for (int x = 0; x < m_world.getWidth(); ++x)
            {
                const TileCoord tile{ x, y };

                if (x < 16)
                {
                    setRegionTag(tile, WorldRegionTagId::IndustrialZone);
                }
                else if (x < 32)
                {
                    setRegionTag(tile, WorldRegionTagId::SterileZone);
                }
                else
                {
                    setRegionTag(tile, WorldRegionTagId::EmergencyZone);
                }
            }
        }

        m_entities.clear();

        Entity crate{};
        crate.id = 1;
        crate.name = "Cargo Crate";
        crate.type = EntityType::Crate;
        crate.tile = { 10, 10 };
        crate.isOpen = false;
        crate.isLocked = false;
        crate.isPowered = false;
        m_entities.add(crate);

        Entity terminal{};
        terminal.id = 2;
        terminal.name = "Operations Terminal";
        terminal.type = EntityType::Terminal;
        terminal.tile = { 18, 8 };
        terminal.isOpen = false;
        terminal.isLocked = false;
        terminal.isPowered = false;
        m_entities.add(terminal);

        Entity locker{};
        locker.id = 3;
        locker.name = "Maintenance Locker";
        locker.type = EntityType::Locker;
        locker.tile = { 30, 24 };
        locker.isOpen = false;
        locker.isLocked = true;
        locker.isPowered = false;
        m_entities.add(locker);
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
            return WorldRegionTagId::IndustrialZone;
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

    size_t WorldState::index(TileCoord tile) const
    {
        return static_cast<size_t>(tile.y) * static_cast<size_t>(m_world.getWidth())
             + static_cast<size_t>(tile.x);
    }
}
