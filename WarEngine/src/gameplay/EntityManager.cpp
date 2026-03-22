#include "engine/gameplay/EntityManager.h"

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"

namespace war
{
    namespace
    {
        void appendEntityTrace(std::string_view line)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "simulation_init_trace.txt", line);
        }
    }

    void EntityManager::clear()
    {
        m_entities.clear();
    }

    void EntityManager::add(const Entity& entity)
    {
        appendEntityTrace("EntityManager::add begin");
        if (m_entities.capacity() < 16)
        {
            m_entities.reserve(16);
        }
        appendEntityTrace("EntityManager::add capacity prepared");
        m_entities.push_back(entity);
        appendEntityTrace("EntityManager::add committed");
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
