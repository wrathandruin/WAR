#include "engine/world/WorldSemanticDressing.h"

#include "engine/gameplay/Entity.h"

namespace war
{
    namespace
    {
        bool isOccupied(const WorldState& worldState, TileCoord tile)
        {
            for (const Entity& entity : worldState.entities().all())
            {
                if (entity.tile.x == tile.x && entity.tile.y == tile.y)
                {
                    return true;
                }
            }

            return false;
        }

        bool tryResolvePlacement(
            const WorldState& worldState,
            WorldRegionTagId expectedRegion,
            TileCoord desired,
            TileCoord& outTile)
        {
            for (int radius = 0; radius <= 6; ++radius)
            {
                for (int dy = -radius; dy <= radius; ++dy)
                {
                    for (int dx = -radius; dx <= radius; ++dx)
                    {
                        const TileCoord candidate{ desired.x + dx, desired.y + dy };

                        if (!worldState.world().isInBounds(candidate))
                        {
                            continue;
                        }

                        if (worldState.regionTag(candidate) != expectedRegion)
                        {
                            continue;
                        }

                        if (worldState.world().isBlocked(candidate))
                        {
                            continue;
                        }

                        if (isOccupied(worldState, candidate))
                        {
                            continue;
                        }

                        outTile = candidate;
                        return true;
                    }
                }
            }

            return false;
        }

        void addEntity(
            WorldState& worldState,
            int& nextId,
            WorldRegionTagId expectedRegion,
            TileCoord desired,
            EntityType type,
            const char* name,
            bool isOpen,
            bool isLocked,
            bool isPowered)
        {
            TileCoord resolved{};
            if (!tryResolvePlacement(worldState, expectedRegion, desired, resolved))
            {
                return;
            }

            Entity entity{};
            entity.id = nextId++;
            entity.name = name;
            entity.type = type;
            entity.tile = resolved;
            entity.isOpen = isOpen;
            entity.isLocked = isLocked;
            entity.isPowered = isPowered;
            worldState.entities().add(entity);
        }
    }

    void WorldSemanticDressing::populateTestEntities(WorldState& worldState)
    {
        worldState.entities().clear();

        int nextId = 1;

        addEntity(worldState, nextId, WorldRegionTagId::CargoBay, { 6, 24 }, EntityType::Crate, "Cargo Pallet Crate", false, false, false);
        addEntity(worldState, nextId, WorldRegionTagId::CargoBay, { 10, 26 }, EntityType::Crate, "Freight Container", false, false, false);
        addEntity(worldState, nextId, WorldRegionTagId::CargoBay, { 12, 21 }, EntityType::Crate, "Supply Bin", false, false, false);

        addEntity(worldState, nextId, WorldRegionTagId::TransitSpine, { 21, 16 }, EntityType::Terminal, "Transit Service Terminal", true, false, true);
        addEntity(worldState, nextId, WorldRegionTagId::TransitSpine, { 20, 14 }, EntityType::Locker, "Route Maintenance Locker", false, false, false);

        addEntity(worldState, nextId, WorldRegionTagId::MedLab, { 18, 7 }, EntityType::Terminal, "Diagnostic Station", true, false, true);
        addEntity(worldState, nextId, WorldRegionTagId::MedLab, { 23, 9 }, EntityType::Locker, "Med Supply Locker", false, false, false);

        addEntity(worldState, nextId, WorldRegionTagId::CommandDeck, { 35, 8 }, EntityType::Terminal, "Command Console", true, false, true);
        addEntity(worldState, nextId, WorldRegionTagId::CommandDeck, { 39, 10 }, EntityType::Locker, "Secure Command Locker", false, true, false);

        addEntity(worldState, nextId, WorldRegionTagId::HazardContainment, { 33, 27 }, EntityType::Locker, "Containment Locker", false, true, false);
        addEntity(worldState, nextId, WorldRegionTagId::HazardContainment, { 29, 25 }, EntityType::Terminal, "Quarantine Control Terminal", false, false, false);
        addEntity(worldState, nextId, WorldRegionTagId::HazardContainment, { 38, 29 }, EntityType::Crate, "Sealed Sample Crate", false, true, false);
    }
}
