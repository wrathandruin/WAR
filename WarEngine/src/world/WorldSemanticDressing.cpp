#include "engine/world/WorldSemanticDressing.h"

#include <string>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/gameplay/Entity.h"

namespace war
{
    namespace
    {
        void appendSemanticTrace(std::string_view line)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "simulation_init_trace.txt", line);
        }

        bool isEntityOccupied(const WorldState& worldState, TileCoord tile)
        {
            return worldState.entities().getAt(tile) != nullptr;
        }

        bool isHotspotOccupied(const WorldState& worldState, TileCoord tile)
        {
            return worldState.authoringHotspotAt(tile) != nullptr;
        }

        bool isHazardOccupied(const WorldState& worldState, TileCoord tile)
        {
            return worldState.terrainHazardAt(tile) != nullptr;
        }

        bool tryResolvePlacement(
            const WorldState& worldState,
            WorldRegionTagId expectedRegion,
            TileCoord desired,
            bool allowEntityOverlap,
            bool allowHotspotOverlap,
            bool allowHazardOverlap,
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

                        if (!allowHazardOverlap && isHazardOccupied(worldState, candidate))
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
            const char* lootProfileId,
            bool isOpen,
            bool isLocked,
            bool isPowered)
        {
            appendSemanticTrace(std::string("addEntity begin: ") + name);
            TileCoord resolved{};
            if (!tryResolvePlacement(worldState, expectedRegion, desired, false, true, true, resolved))
            {
                appendSemanticTrace(std::string("addEntity placement failed: ") + name);
                return;
            }
            appendSemanticTrace(std::string("addEntity placement resolved: ") + name);

            Entity entity{};
            entity.id = nextId++;
            entity.name = name;
            entity.type = type;
            entity.tile = resolved;
            entity.isOpen = isOpen;
            entity.isLocked = isLocked;
            entity.isPowered = isPowered;
            entity.lootProfileId = lootProfileId != nullptr ? lootProfileId : "";
            worldState.entities().add(entity);
            appendSemanticTrace(std::string("addEntity committed: ") + name);
        }

        void addHotspot(
            WorldState& worldState,
            int& nextId,
            WorldRegionTagId expectedRegion,
            TileCoord desired,
            WorldAuthoringHotspotType type,
            const char* label,
            const char* summary,
            const char* locationKey,
            const char* locationTitle,
            const char* locationDescription,
            bool encounterReady)
        {
            TileCoord resolved{};
            if (!tryResolvePlacement(worldState, expectedRegion, desired, false, false, true, resolved))
            {
                return;
            }

            WorldAuthoringHotspot hotspot{};
            hotspot.id = nextId++;
            hotspot.tile = resolved;
            hotspot.region = expectedRegion;
            hotspot.type = type;
            hotspot.label = label != nullptr ? label : "";
            hotspot.summary = summary != nullptr ? summary : "";
            hotspot.locationKey = locationKey != nullptr ? locationKey : "";
            hotspot.locationTitle = locationTitle != nullptr ? locationTitle : "";
            hotspot.locationDescription = locationDescription != nullptr ? locationDescription : "";
            hotspot.encounterReady = encounterReady;
            worldState.addAuthoringHotspot(hotspot);
        }

        void addHazard(
            WorldState& worldState,
            int& nextId,
            WorldRegionTagId expectedRegion,
            TileCoord desired,
            TerrainHazardType type,
            const char* label,
            int severity)
        {
            TileCoord resolved{};
            if (!tryResolvePlacement(worldState, expectedRegion, desired, true, true, false, resolved))
            {
                return;
            }

            TerrainHazardTile hazard{};
            hazard.id = nextId++;
            hazard.tile = resolved;
            hazard.region = expectedRegion;
            hazard.type = type;
            hazard.label = label;
            hazard.severity = severity;
            hazard.active = true;
            worldState.addTerrainHazard(hazard);
        }
    }

    void WorldSemanticDressing::populateAuthoredTestWorld(WorldState& worldState)
    {
        appendSemanticTrace("WorldSemanticDressing::populateAuthoredTestWorld entered");
        worldState.entities().clear();
        worldState.clearRegionDescriptions();
        worldState.clearAuthoringHotspots();
        worldState.clearTerrainHazards();

        worldState.setRegionDescription(
            WorldRegionTagId::CargoBay,
            "region.khepri-cargo-bay",
            "Khepri Dock Cargo Bay",
            "Floodlit pallet lanes, dock clamps, and stacked freight mark this bay as Khepri Dock's working intake floor.");
        worldState.setRegionDescription(
            WorldRegionTagId::TransitSpine,
            "region.transit-spine",
            "Transit Spine",
            "Narrow conduit lighting and maintenance striping pull personnel and freight through the station's hardened central spine.");
        worldState.setRegionDescription(
            WorldRegionTagId::MedLab,
            "region.medlab-diagnostics",
            "MedLab Diagnostics",
            "Sterile light, diagnostic trays, and sealed med stations give the lab a clinical chill that the rest of the dock never quite loses.");
        worldState.setRegionDescription(
            WorldRegionTagId::CommandDeck,
            "region.command-deck-approach",
            "Command Deck Approach",
            "Armored bulkheads, hardened consoles, and restricted sightlines tighten the command approach into a guarded operational lane.");
        worldState.setRegionDescription(
            WorldRegionTagId::HazardContainment,
            "region.hazard-containment",
            "Hazard Containment",
            "Scored containment walls, emergency foam stains, and patchwork sealant make this sector feel recently fought over.");

        int nextEntityId = 1;
        int nextHotspotId = 1;
        int nextHazardId = 1;

        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 6, 24 }, EntityType::Crate, "Cargo Pallet Crate", "cargo_supplies", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 10, 26 }, EntityType::Crate, "Freight Container", "cargo_supplies", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 4, 28 }, EntityType::Locker, "Dockside Locker", "dockside_personal", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 6, 20 }, EntityType::Ship, "Responder Shuttle Khepri", "", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 8, 20 }, EntityType::Terminal, "Shuttle Helm Terminal", "", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 9, 20 }, EntityType::Terminal, "Orbital Navigation Console", "", false, true, false);
        appendSemanticTrace("WorldSemanticDressing cargo bay entities placed");

        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::CargoBay, { 8, 22 },
            WorldAuthoringHotspotType::Loot,
            "Cargo Staging Pocket",
            "A serviceable loot pocket that feeds the early inventory loop.",
            "hotspot.cargo-staging-pocket",
            "Cargo Staging Pocket",
            "Cargo netting, forklift scars, and tagged supply stacks turn this recess into a compact staging pocket for scavenged supplies.",
            true);
        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::CargoBay, { 7, 21 },
            WorldAuthoringHotspotType::Transit,
            "Docked Boarding Collar",
            "A pressurized shuttle collar that becomes the first ship-runtime boarding lane.",
            "hotspot.docked-boarding-collar",
            "Docked Boarding Collar",
            "The shuttle collar hisses under pressure equalization, narrowing the world to one steel throat between dock and responder craft.",
            false);
        appendSemanticTrace("WorldSemanticDressing cargo bay hotspots placed");

        addEntity(worldState, nextEntityId, WorldRegionTagId::TransitSpine, { 21, 16 }, EntityType::Terminal, "Transit Service Terminal", "", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::TransitSpine, { 20, 14 }, EntityType::Locker, "Route Maintenance Locker", "maintenance_cache", false, false, false);
        appendSemanticTrace("WorldSemanticDressing transit spine entities placed");

        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::TransitSpine, { 21, 17 },
            WorldAuthoringHotspotType::Transit,
            "Spine Junction",
            "A routing anchor that later feeds mission and traversal flow.",
            "hotspot.spine-junction",
            "Spine Junction",
            "Maintenance arrows, scuffed deck plating, and overlapping service lines make the junction feel like the station's circulatory knot.",
            true);
        appendSemanticTrace("WorldSemanticDressing transit spine hotspot placed");

        addEntity(worldState, nextEntityId, WorldRegionTagId::MedLab, { 18, 7 }, EntityType::Terminal, "Diagnostic Station", "", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::MedLab, { 23, 9 }, EntityType::Locker, "Med Supply Locker", "medical_supplies", false, false, false);
        appendSemanticTrace("WorldSemanticDressing medlab entities placed");

        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::MedLab, { 19, 8 },
            WorldAuthoringHotspotType::Control,
            "Triage Convergence",
            "A controlled medical access point suited to mission and survival beats.",
            "hotspot.triage-convergence",
            "Triage Convergence",
            "Med trolleys, sealable partitions, and clipped diagnostic readouts crowd this triage convergence with controlled urgency.",
            true);
        appendSemanticTrace("WorldSemanticDressing medlab hotspot placed");

        addEntity(worldState, nextEntityId, WorldRegionTagId::CommandDeck, { 35, 8 }, EntityType::Terminal, "Command Console", "", true, false, true);
        addEntity(worldState, nextEntityId, WorldRegionTagId::CommandDeck, { 39, 10 }, EntityType::Locker, "Secure Command Locker", "command_secure", false, true, false);
        appendSemanticTrace("WorldSemanticDressing command deck entities placed");

        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::CommandDeck, { 41, 5 },
            WorldAuthoringHotspotType::Encounter,
            "Bridge Access Chokepoint",
            "A narrow, hostile crossing suited to the first six-second combat encounter.",
            "hotspot.bridge-access-chokepoint",
            "Bridge Access Chokepoint",
            "A disciplined chokepoint of armored shutters and control posts forces any approach into the command lane to feel deliberate and exposed.",
            true);
        appendSemanticTrace("WorldSemanticDressing command deck hotspot placed");

        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 33, 27 }, EntityType::Locker, "Containment Locker", "response_gear", false, true, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 29, 25 }, EntityType::Terminal, "Quarantine Control Terminal", "", false, false, false);
        addEntity(worldState, nextEntityId, WorldRegionTagId::HazardContainment, { 35, 23 }, EntityType::Crate, "Response Gear Case", "response_gear", false, false, false);
        appendSemanticTrace("WorldSemanticDressing hazard containment entities placed");

        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::HazardContainment, { 31, 26 },
            WorldAuthoringHotspotType::Hazard,
            "Containment Breach Lane",
            "A dangerous lane where suit, oxygen, and health pressure become visible.",
            "hotspot.containment-breach-lane",
            "Containment Breach Lane",
            "Emergency patches, scorched sealant, and warning strobes mark this lane as the heart of the current containment failure.",
            true);
        appendSemanticTrace("WorldSemanticDressing hazard hotspot placed");

        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::HazardContainment, { 24, 27 },
            WorldAuthoringHotspotType::Encounter,
            "Quarantine Access Gate",
            "A close-range hostile contact point for the first authoritative encounter loop.",
            "hotspot.quarantine-access-gate",
            "Quarantine Access Gate",
            "The quarantine gate stands half-reset, with hard-lock brackets and emergency overrides still bearing signs of forced access.",
            true);
        appendSemanticTrace("WorldSemanticDressing quarantine gate hotspot placed");

        addEntity(worldState, nextEntityId, WorldRegionTagId::CargoBay, { 44, 4 }, EntityType::Terminal, "Frontier Relay Beacon", "", false, true, false);
        addHotspot(
            worldState, nextHotspotId, WorldRegionTagId::CargoBay, { 43, 4 },
            WorldAuthoringHotspotType::Transit,
            "Dust Frontier Landing Pad",
            "An isolated frontier pad that becomes the second-destination landing and return-loop proof for M44.",
            "frontier.dust-frontier-landing-pad",
            "Dust Frontier Landing Pad",
            "Dust-scoured plating and relay scaffold shadows make the frontier pad feel temporary, exposed, and very far from home.",
            false);
        appendSemanticTrace("WorldSemanticDressing frontier destination anchors placed");

        addHazard(worldState, nextHazardId, WorldRegionTagId::HazardContainment, { 30, 26 }, TerrainHazardType::ToxicSpill, "Corrosive spill", 2);
        addHazard(worldState, nextHazardId, WorldRegionTagId::HazardContainment, { 32, 26 }, TerrainHazardType::RadiationLeak, "Containment radiation plume", 2);
        addHazard(worldState, nextHazardId, WorldRegionTagId::TransitSpine, { 23, 17 }, TerrainHazardType::ElectricalArc, "Transit conduit arcing", 1);
        addHazard(worldState, nextHazardId, WorldRegionTagId::MedLab, { 16, 8 }, TerrainHazardType::VacuumLeak, "Medlab pressure loss", 1);
        appendSemanticTrace("WorldSemanticDressing::populateAuthoredTestWorld completed");
    }
}
