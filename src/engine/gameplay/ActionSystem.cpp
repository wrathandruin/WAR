#include "engine/gameplay/ActionSystem.h"

#include <cstdio>
#include <utility>

#include "engine/gameplay/Action.h"
#include "engine/world/Pathfinding.h"

namespace war
{
    namespace
    {
        const char* boolText(bool value)
        {
            return value ? "yes" : "no";
        }

        const char* stateText(const Entity& entity)
        {
            switch (entity.type)
            {
            case EntityType::Crate:
                return entity.isOpen ? "open" : "closed";
            case EntityType::Terminal:
                return entity.isPowered ? "powered" : "offline";
            case EntityType::Locker:
                if (entity.isLocked)
                {
                    return "locked";
                }
                return entity.isOpen ? "open" : "closed";
            default:
                return "unknown";
            }
        }
    }

    const char* ActionSystem::entityTypeToText(EntityType type)
    {
        switch (type)
        {
        case EntityType::Crate:
            return "crate";
        case EntityType::Terminal:
            return "terminal";
        case EntityType::Locker:
            return "locker";
        default:
            return "unknown";
        }
    }

    void ActionSystem::pushEvent(std::vector<std::string>& eventLog, const std::string& message)
    {
        eventLog.push_back(message);
        constexpr size_t kMaxEvents = 10;
        if (eventLog.size() > kMaxEvents)
        {
            eventLog.erase(
                eventLog.begin(),
                eventLog.begin() + static_cast<std::ptrdiff_t>(eventLog.size() - kMaxEvents));
        }
    }

    void ActionSystem::rebuildPathTo(
        WorldState& worldState,
        const Vec2& playerPosition,
        TileCoord targetTile,
        std::vector<TileCoord>& currentPath,
        size_t& pathIndex,
        std::vector<std::string>& eventLog)
    {
        const TileCoord startTile = worldState.world().worldToTile(playerPosition);
        std::vector<TileCoord> path = Pathfinding::findPath(worldState.world(), startTile, targetTile);

        if (path.empty())
        {
            pushEvent(eventLog, "Move failed: no path found");
            currentPath.clear();
            pathIndex = 0;
            return;
        }

        currentPath = std::move(path);
        pathIndex = currentPath.size() > 1 ? 1 : 0;

        char buffer[128]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "Move queued: %zu nodes to (%d, %d)",
            currentPath.size(),
            targetTile.x,
            targetTile.y);
        pushEvent(eventLog, buffer);
    }

    void ActionSystem::processPending(
        WorldState& worldState,
        ActionQueue& actions,
        const Vec2& playerPosition,
        std::vector<TileCoord>& currentPath,
        size_t& pathIndex,
        std::vector<std::string>& eventLog)
    {
        while (actions.hasActions())
        {
            const Action action = actions.pop();

            if (action.type == ActionType::Move)
            {
                if (!worldState.world().isInBounds(action.target))
                {
                    pushEvent(eventLog, "Move rejected: target out of bounds");
                }
                else if (worldState.world().isBlocked(action.target))
                {
                    pushEvent(eventLog, "Move rejected: target tile blocked");
                }
                else
                {
                    rebuildPathTo(worldState, playerPosition, action.target, currentPath, pathIndex, eventLog);
                }
            }
            else if (action.type == ActionType::Inspect)
            {
                if (!worldState.world().isInBounds(action.target))
                {
                    pushEvent(eventLog, "Inspect rejected: target out of bounds");
                    continue;
                }

                char buffer[256]{};
                const Entity* entity = worldState.entities().getAt(action.target);

                if (entity != nullptr)
                {
                    std::snprintf(
                        buffer,
                        sizeof(buffer),
                        "Inspect (%d, %d): blocked=%s, entity=%s [%s], state=%s",
                        action.target.x,
                        action.target.y,
                        boolText(worldState.world().isBlocked(action.target)),
                        entity->name.c_str(),
                        entityTypeToText(entity->type),
                        stateText(*entity));
                }
                else
                {
                    std::snprintf(
                        buffer,
                        sizeof(buffer),
                        "Inspect (%d, %d): blocked=%s, entity=none",
                        action.target.x,
                        action.target.y,
                        boolText(worldState.world().isBlocked(action.target)));
                }

                pushEvent(eventLog, buffer);
            }
            else if (action.type == ActionType::Interact)
            {
                if (!worldState.world().isInBounds(action.target))
                {
                    pushEvent(eventLog, "Nothing to interact with.");
                    continue;
                }

                Entity* entity = worldState.entities().getAt(action.target);
                if (entity == nullptr)
                {
                    pushEvent(eventLog, "Nothing to interact with.");
                    continue;
                }

                switch (entity->type)
                {
                case EntityType::Crate:
                    if (!entity->isOpen)
                    {
                        entity->isOpen = true;
                        pushEvent(eventLog, "You open the cargo crate.");
                    }
                    else
                    {
                        pushEvent(eventLog, "The cargo crate is already open.");
                    }
                    break;

                case EntityType::Terminal:
                    entity->isPowered = !entity->isPowered;
                    if (entity->isPowered)
                    {
                        pushEvent(eventLog, "The operations terminal powers on.");
                    }
                    else
                    {
                        pushEvent(eventLog, "The operations terminal powers down.");
                    }
                    break;

                case EntityType::Locker:
                    if (entity->isLocked)
                    {
                        pushEvent(eventLog, "The maintenance locker is locked.");
                    }
                    else if (!entity->isOpen)
                    {
                        entity->isOpen = true;
                        pushEvent(eventLog, "You open the maintenance locker.");
                    }
                    else
                    {
                        pushEvent(eventLog, "The maintenance locker is already open.");
                    }
                    break;

                default:
                    pushEvent(eventLog, "You are not sure how to interact with that.");
                    break;
                }
            }
        }
    }
}
