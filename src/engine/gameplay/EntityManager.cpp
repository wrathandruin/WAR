#include "engine/gameplay/EntityManager.h"

namespace war
{
    void EntityManager::clear()
    {
        m_entities.clear();
    }

    void EntityManager::add(const Entity& entity)
    {
        m_entities.push_back(entity);
    }

    Entity* EntityManager::getAt(TileCoord tile)
    {
        for (Entity& entity : m_entities)
        {
            if (entity.tile == tile)
            {
                return &entity;
            }
        }

        return nullptr;
    }

    const Entity* EntityManager::getAt(TileCoord tile) const
    {
        for (const Entity& entity : m_entities)
        {
            if (entity.tile == tile)
            {
                return &entity;
            }
        }

        return nullptr;
    }

    const std::vector<Entity>& EntityManager::all() const
    {
        return m_entities;
    }

    size_t EntityManager::count() const
    {
        return m_entities.size();
    }
}
