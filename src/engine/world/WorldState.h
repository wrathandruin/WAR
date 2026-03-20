#pragma once

#include "engine/gameplay/EntityManager.h"
#include "engine/world/WorldGrid.h"

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

    private:
        WorldGrid m_world;
        EntityManager m_entities;
    };
}
