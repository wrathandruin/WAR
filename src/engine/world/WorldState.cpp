#include "engine/world/WorldState.h"

#include "engine/gameplay/Entity.h"

namespace war
{
    WorldState::WorldState()
        : m_world(48, 36, 48)
    {
    }

    void WorldState::initializeTestWorld()
    {
        m_world.generateTestMap();
        m_visualTheme = BgfxWorldThemeId::Industrial;

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

    void WorldState::setVisualTheme(BgfxWorldThemeId theme)
    {
        m_visualTheme = theme;
    }

    BgfxWorldThemeId WorldState::visualTheme() const
    {
        return m_visualTheme;
    }
}
