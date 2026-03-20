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

        m_entities.clear();
        m_entities.add({ 1, "Cargo Crate", EntityType::Crate, { 10, 10 } });
        m_entities.add({ 2, "Operations Terminal", EntityType::Terminal, { 18, 8 } });
        m_entities.add({ 3, "Maintenance Locker", EntityType::Locker, { 30, 24 } });
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
}
