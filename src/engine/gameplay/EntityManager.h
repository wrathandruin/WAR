#pragma once

#include <cstddef>
#include <vector>

#include "engine/gameplay/Entity.h"

namespace war
{
    class EntityManager
    {
    public:
        void clear();
        void add(const Entity& entity);

        [[nodiscard]] Entity* getAt(TileCoord tile);
        [[nodiscard]] const Entity* getAt(TileCoord tile) const;

        [[nodiscard]] const std::vector<Entity>& all() const;
        [[nodiscard]] size_t count() const;

    private:
        std::vector<Entity> m_entities;
    };
}
