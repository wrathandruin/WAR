#include "engine/gameplay/ActionSystem.h"

#include <cstdio>
#include <string>
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

        const char* hotspotTypeToText(WorldAuthoringHotspotType type)
        {
            switch (type)
            {
            case WorldAuthoringHotspotType::Encounter:
                return "encounter";
            case WorldAuthoringHotspotType::Control:
                return "control";
            case WorldAuthoringHotspotType::Transit:
                return "transit";
            case WorldAuthoringHotspotType::Loot:
                return "loot";
            case WorldAuthoringHotspotType::Hazard:
                return "hazard";
            default:
                return "unknown";
            }
        }

        const char* hotspotStateToText(const WorldAuthoringHotspot& hotspot)
        {
            return hotspot.encounterReady ? "encounter-ready" : "staged";
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

                const Entity* entity = worldState.entities().getAt(action.target);
                const WorldAuthoringHotspot* hotspot = worldState.authoringHotspotAt(action.target);

                std::string message = "Inspect [";
                message += std::to_string(action.target.x);
                message += ", ";
                message += std::to_string(action.target.y);
                message += "] region=";
                message += WorldRegionTags::debugName(worldState.regionTag(action.target));
                message += ", blocked=";
                message += boolText(worldState.world().isBlocked(action.target));

                if (entity != nullptr)
                {
                    message += ", entity=";
                    message += entity->name;
                    message += " [";
                    message += entityTypeToText(entity->type);
                    message += ", ";
                    message += stateText(*entity);
                    message += "]";
                }
                else
                {
                    message += ", entity=none";
                }

                if (hotspot != nullptr)
                {
                    message += ", hotspot=";
                    message += hotspot->label;
                    message += " [";
                    message += hotspotTypeToText(hotspot->type);
                    message += ", ";
                    message += hotspotStateToText(*hotspot);
                    message += "]";
                    message += ", detail=";
                    message += hotspot->summary;
                }
                else
                {
                    message += ", hotspot=none";
                }

                pushEvent(eventLog, message);
            }
            else if (action.type == ActionType::Interact)
            {
                if (!worldState.world().isInBounds(action.target))
                {
                    pushEvent(eventLog, "Nothing to interact with.");
                    continue;
                }

                Entity* entity = worldState.entities().getAt(action.target);
                if (entity != nullptr)
                {
                    switch (entity->type)
                    {
                    case EntityType::Crate:
                        if (!entity->isOpen)
                        {
                            entity->isOpen = true;
                            pushEvent(eventLog, std::string("You open ") + entity->name + ".");
                        }
                        else
                        {
                            pushEvent(eventLog, entity->name + std::string(" is already open."));
                        }
                        break;

                    case EntityType::Terminal:
                        entity->isPowered = !entity->isPowered;
                        if (entity->isPowered)
                        {
                            pushEvent(eventLog, std::string("You power on ") + entity->name + ".");
                        }
                        else
                        {
                            pushEvent(eventLog, std::string("You power down ") + entity->name + ".");
                        }
                        break;

                    case EntityType::Locker:
                        if (entity->isLocked)
                        {
                            pushEvent(eventLog, entity->name + std::string(" is locked."));
                        }
                        else if (!entity->isOpen)
                        {
                            entity->isOpen = true;
                            pushEvent(eventLog, std::string("You open ") + entity->name + ".");
                        }
                        else
                        {
                            pushEvent(eventLog, entity->name + std::string(" is already open."));
                        }
                        break;

                    default:
                        pushEvent(eventLog, std::string("You are not sure how to use ") + entity->name + ".");
                        break;
                    }

                    continue;
                }

                const WorldAuthoringHotspot* hotspot = worldState.authoringHotspotAt(action.target);
                if (hotspot == nullptr)
                {
                    pushEvent(eventLog, "Nothing to interact with.");
                    continue;
                }

                std::string message = "You use ";
                message += hotspot->label;
                message += " [";
                message += hotspotTypeToText(hotspot->type);
                message += ", ";
                message += hotspotStateToText(*hotspot);
                message += "]";

                switch (hotspot->type)
                {
                case WorldAuthoringHotspotType::Encounter:
                    message += ". This is a future combat-resolution anchor.";
                    break;
                case WorldAuthoringHotspotType::Control:
                    message += ". This is a future mission, gate, or authority control point.";
                    break;
                case WorldAuthoringHotspotType::Transit:
                    message += ". This is a future routing and traversal anchor.";
                    break;
                case WorldAuthoringHotspotType::Loot:
                    message += ". This is a future inventory and recovery anchor.";
                    break;
                case WorldAuthoringHotspotType::Hazard:
                    message += ". This is a future environmental hazard and consequence anchor.";
                    break;
                default:
                    message += ".";
                    break;
                }

                pushEvent(eventLog, message);
            }
        }
    }
}
