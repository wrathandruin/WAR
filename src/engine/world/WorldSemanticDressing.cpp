#include "engine/world/WorldSemanticDressing.h"

#include "engine/gameplay/Entity.h"

namespace war
{
    namespace
    {
        bool isEntityOccupied(const WorldState& worldState, TileCoord tile)
        {
            for (const Entity& entity : worldState.entities().all())
            {
                if (entity.tile == tile)
                {
                    return true;
                }
            }

            return false;
        }

        bool isHotspotOccupied(const WorldState& worldState, TileCoord tile)
        {
            for (const WorldAuthoringHotspot& hotspot : worldState.authoringHotspots())
            {
                if (hotspot.tile == tile)
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
            bool allowEntityOverlap,
            bool allowHotspotOverlap,
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

                        if (!allowEntityOverlap && isEntityOccupied(worldState, candidate))
                        {
                            continue;
                        }

                        if (!allowHotspotOverlap && isHotspotOccupied(worldState, candidate))
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
            if (!tryResolvePlacement(worldState, expectedRegion, desired, false, true, resolved))
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

        void addHotspot(
            WorldState& worldState,
            int& nextId,
            WorldRegionTagId expectedRegion,
            TileCoord desired,
            WorldAuthoringHotspotType type,
            const char* label,
            const char* summary,
            bool encounterReady)
        {
            TileCoord resolved{};
            if (!tryResolvePlacement(worldState, expectedRegion, desired, false, false, resolved))
            {
                return;
            }

            WorldAuthoringHotspot hotspot{};
            hotspot.id = nextId++;
            hotspot.tile = resolved;
            hotspot.region = expectedRegion;
            hotspot.type = type;
            hotspot.label = label;
            hotspot.summary = summary;
            hotspot.encounterReady = encounterReady;
            worldState.addAuthoringHotspot(hotspot);
        }
    }

    void WorldSemanticDressing::populateAuthoredTestWorld(WorldState& worldState)
    {
        worldState.entities().clear();
        worldState.clearAuthoringHotspots();

        int nextEntityId = 1;
        int nextHotspotId = 1;

        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 6, 24 }, EntityType::Crate, "Cargo Pallet Crate", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 10, 26 }, EntityType::Crate, "Freight Container", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 12, 21 }, EntityType::Crate, "Supply Bin", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 4, 28 }, EntityType::Locker, "Dockside Locker", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 15, 24 }, EntityType::Crate, "Stacked Cargo Cage", false, true, false);

        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::CargoBay,
            { 8, 22 },
            WorldAuthoringHotspotType::Loot,
            "Cargo Staging Pocket",
            "Freight is staged here before transfer. The lane reads as a loot and inspection anchor.",
            true);
        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::CargoBay,
            { 3, 19 },
            WorldAuthoringHotspotType::Transit,
            "Loading Route Cutthrough",
            "A clean access lane that can later support traversal, routing, or escort logic.",
            false);

        addEntity(worldState, nextEntityId, WorldRegionTagId::TransitSpine, { 21, 16 }, EntityType::Terminal, "Transit Service Terminal", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::TransitSpine, { 20, 14 }, EntityType::Locker, "Route Maintenance Locker", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::TransitSpine, { 24, 17 }, EntityType::Terminal, "Route Junction Panel", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::TransitSpine, { 18, 18 }, EntityType::Locker, "Service Cache", false, true, false);

        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::TransitSpine,
            { 21, 17 },
            WorldAuthoringHotspotType::Transit,
            "Spine Junction",
            "A high-traffic crossing that can later host encounter routing, patrol gating, or mission flow.",
            true);
        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::TransitSpine,
            { 20, 11 },
            WorldAuthoringHotspotType::Control,
            "Maintenance Hold Point",
            "A service hold point suited to authored control or inspection beats.",
            false);

        addEntity(worldState, nextEntityId, WorldRegionTagId::MedLab, { 18, 7 }, EntityType::Terminal, "Diagnostic Station", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::MedLab, { 23, 9 }, EntityType::Locker, "Med Supply Locker", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::MedLab, { 15, 6 }, EntityType::Terminal, "Sterile Intake Console", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::MedLab, { 24, 6 }, EntityType::Locker, "Sealed Recovery Cabinet", false, true, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::MedLab, { 17, 10 }, EntityType::Crate, "Clean Supply Case", false, false, false);

        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::MedLab,
            { 19, 8 },
            WorldAuthoringHotspotType::Control,
            "Triage Convergence",
            "A controlled treatment lane that already reads like a future objective or encounter checkpoint.",
            true);
        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::MedLab,
            { 14, 10 },
            WorldAuthoringHotspotType::Loot,
            "Recovery Intake Niche",
            "A small staging space suited to item transfer, medical recovery, or mission pickup logic.",
            false);

        addEntity(worldState, nextEntityId, WorldRegionTagId::CommandDeck, { 35, 8 }, EntityType::Terminal, "Command Console", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CommandDeck, { 39, 10 }, EntityType::Locker, "Secure Command Locker", false, true, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CommandDeck, { 31, 6 }, EntityType::Terminal, "Operations Readout", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CommandDeck, { 41, 9 }, EntityType::Locker, "Command Archive Cabinet", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CommandDeck, { 30, 11 }, EntityType::Crate, "Secured Briefing Cache", false, true, false);

        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::CommandDeck,
            { 32, 9 },
            WorldAuthoringHotspotType::Control,
            "Command Overwatch Position",
            "A strong authored anchor for security, mission escalation, or future authority-facing scene logic.",
            true);
        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::CommandDeck,
            { 41, 5 },
            WorldAuthoringHotspotType::Encounter,
            "Bridge Access Chokepoint",
            "A narrow high-value crossing that already reads like encounter space.",
            true);

        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 33, 27 }, EntityType::Locker, "Containment Locker", false, true, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 29, 25 }, EntityType::Terminal, "Quarantine Control Terminal", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 38, 29 }, EntityType::Crate, "Sealed Sample Crate", false, true, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 41, 27 }, EntityType::Terminal, "Containment Sensor Mast", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 24, 26 }, EntityType::Locker, "Emergency Washdown Cabinet", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 35, 23 }, EntityType::Crate, "Response Gear Case", false, false, false);

        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::HazardContainment,
            { 31, 26 },
            WorldAuthoringHotspotType::Hazard,
            "Containment Breach Lane",
            "The geometry and dressing mark this as a future hazard or combat-resolution space.",
            true);
        addHotspot(
            worldState,
            nextHotspotId,
            WorldRegionTagId::HazardContainment,
            { 24, 27 },
            WorldAuthoringHotspotType::Control,
            "Quarantine Access Gate",
            "A controllable threshold suited to future mission gating and containment state changes.",
            true);
    }
}
