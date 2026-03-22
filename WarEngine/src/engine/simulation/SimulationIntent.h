#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "engine/gameplay/Entity.h"
#include "engine/math/Vec2.h"
#include "engine/world/WorldGrid.h"

namespace war
{
    enum class SimulationIntentType
    {
        MoveToTile,
        InspectTile,
        InteractTile
    };

    enum class SimulationIntentAckResult
    {
        None,
        Accepted,
        Rejected
    };

    enum class InventoryItemId : int
    {
        None = 0,
        RationPack = 1,
        ScrapBundle = 2,
        AccessBadge = 3,
        MedInjector = 4,
        MaintenanceKey = 5,
        SealantKit = 6,
        ShockPistol = 7,
        ContainmentMask = 8,
        SurveyScanner = 9,
        UtilitySuit = 10,
        PatchKit = 11
    };

    struct InventoryItemStack
    {
        InventoryItemId id = InventoryItemId::None;
        uint32_t quantity = 0;
    };

    struct PlayerEquipmentState
    {
        InventoryItemId weapon = InventoryItemId::None;
        InventoryItemId suit = InventoryItemId::None;
        InventoryItemId tool = InventoryItemId::None;
    };

    struct PlayerActorRuntimeState
    {
        int health = 100;
        int maxHealth = 100;
        int armor = 25;
        int suitIntegrity = 100;
        float oxygenSecondsRemaining = 300.0f;
        int radiationDose = 0;
        int toxicExposure = 0;

        uint64_t lootCollections = 0;
        uint64_t encounterWins = 0;
        uint64_t encountersSurvived = 0;
        uint64_t lastSaveEpochMilliseconds = 0;

        std::vector<InventoryItemStack> inventory{};
        PlayerEquipmentState equipment{};
    };

    struct CombatEncounterState
    {
        bool active = false;
        std::string encounterId;
        std::string label;
        TileCoord anchorTile{};
        uint32_t roundNumber = 0;
        uint32_t roundTicksRemaining = 0;
        std::string hostileLabel;
        int hostileHealth = 0;
        int hostileMaxHealth = 0;
        int hostileArmor = 0;
        int hostileMinDamage = 0;
        int hostileMaxDamage = 0;
        std::string lastOutcome;
    };

    enum class MissionId : int
    {
        None = 0,
        PlanetaryMissionSlice = 1
    };

    enum class MissionPhase : int
    {
        None = 0,
        RecoverTransitData = 1,
        DiagnoseMedlab = 2,
        SecureQuarantineGate = 3,
        RestoreQuarantineControl = 4,
        BoardResponderShuttle = 5,
        ClaimShuttleCommand = 6,
        EnterOrbitalLane = 7,
        ReachSurveyOrbit = 8,
        StabilizeRelayTrack = 9,
        DockRelayPlatform = 10,
        LandDustFrontier = 11,
        SecureFrontierRelay = 12,
        ReturnToShuttle = 13,
        ReturnToHomeDock = 14,
        MissionComplete = 15
    };

    enum class ShipId : int
    {
        None = 0,
        ResponderShuttleKhepri = 1
    };

    enum class ShipOwnershipState : int
    {
        None = 0,
        MissionAssigned = 1,
        PlayerCommand = 2
    };

    enum class ShipOccupancyState : int
    {
        PlanetSurface = 0,
        AboardDockedShip = 1,
        AboardOrbitalShip = 2
    };

    enum class OrbitalNodeId : int
    {
        DockedAnchor = 0,
        TrafficSeparationLane = 1,
        DebrisSurveyOrbit = 2,
        RelayHoldingTrack = 3,
        RelayPlatformDock = 4,
        ReturnTrafficLane = 5,
        HomeDockAnchor = 6
    };

    enum class OrbitalTravelPhase : int
    {
        Docked = 0,
        UndockingTransfer = 1,
        TrafficLaneHolding = 2,
        SurveyTransfer = 3,
        SurveyHolding = 4,
        RelayTransfer = 5,
        RelayHolding = 6,
        DockingTransfer = 7,
        RelayPlatformDocked = 8,
        ReturnLaunchTransfer = 9,
        HomeDockTransfer = 10,
        HomeDocked = 11
    };

    struct MissionRuntimeState
    {
        MissionId activeMission = MissionId::PlanetaryMissionSlice;
        MissionPhase phase = MissionPhase::RecoverTransitData;
        uint64_t advancementCount = 0;
        bool transitDataRecovered = false;
        bool medlabDiagnosisComplete = false;
        bool quarantineGateUnlocked = false;
        bool quarantineEncounterResolved = false;
        bool quarantineControlRestored = false;
        bool shipRuntimePrepReady = false;
        bool orbitalDepartureComplete = false;
        bool surveyOrbitReached = false;
        bool relayTrackStabilized = false;
        bool relayPlatformDocked = false;
        bool dustFrontierLanded = false;
        bool frontierRelaySecured = false;
        bool returnLaunchComplete = false;
        bool homeDockRestored = false;
        bool returnLoopComplete = false;
        bool missionComplete = false;
        std::string lastBeat = "Mission initialized";
    };

    struct ShipRuntimeState
    {
        ShipId activeShip = ShipId::ResponderShuttleKhepri;
        ShipOwnershipState ownershipState = ShipOwnershipState::MissionAssigned;
        ShipOccupancyState occupancyState = ShipOccupancyState::PlanetSurface;
        uint64_t boardingCount = 0;
        bool boardingUnlocked = false;
        bool docked = true;
        bool playerBoarded = false;
        bool powerOnline = false;
        bool airlockPressurized = false;
        bool commandClaimed = false;
        bool launchPrepReady = false;
        bool frontierSurfaceAccessUnlocked = false;
        bool frontierSurfaceActive = false;
        std::string shipName = "Responder Shuttle Khepri";
        std::string locationLabel = "cargo-bay-dock";
        std::string lastBeat = "Ship runtime awaiting authorization.";
    };

    struct OrbitalRuntimeState
    {
        bool orbitalLayerUnlocked = false;
        bool orbitalLayerActive = false;
        bool departureAuthorized = false;
        bool surveyOrbitReached = false;
        bool relayTrackReached = false;
        bool relayPlatformDocked = false;
        bool returnRouteAuthorized = false;
        bool homeDockReached = false;
        bool travelInProgress = false;
        uint64_t transferCount = 0;
        uint32_t travelTicksRemaining = 0;
        OrbitalNodeId currentNode = OrbitalNodeId::DockedAnchor;
        OrbitalNodeId targetNode = OrbitalNodeId::DockedAnchor;
        OrbitalTravelPhase phase = OrbitalTravelPhase::Docked;
        std::string ruleText = "Local orbit remains locked until shuttle command is claimed.";
        std::string lastBeat = "Orbital layer inactive.";
    };

    struct SimulationIntent
    {
        uint64_t sequence = 0;
        SimulationIntentType type = SimulationIntentType::MoveToTile;
        TileCoord target{};
        std::string clientInstanceId;
        std::string sessionId;
    };

    struct SimulationIntentAck
    {
        uint64_t sequence = 0;
        SimulationIntentType type = SimulationIntentType::MoveToTile;
        TileCoord target{};
        SimulationIntentAckResult result = SimulationIntentAckResult::None;
        std::string reason;
        uint64_t hostSimulationTicks = 0;
        uint64_t publishedEpochMilliseconds = 0;
        std::string clientInstanceId;
        std::string sessionId;
    };

    struct ReplicatedEntityState
    {
        int id = 0;
        std::string name;
        EntityType type = EntityType::Crate;
        TileCoord tile{};
        bool isOpen = false;
        bool isLocked = false;
        bool isPowered = false;
        std::string lootProfileId;
        bool lootClaimed = false;
    };

    struct AuthoritativeWorldSnapshot
    {
        bool valid = false;
        uint64_t simulationTicks = 0;
        uint64_t lastProcessedIntentSequence = 0;
        uint64_t nextIntentSequence = 1;
        uint64_t publishedEpochMilliseconds = 0;

        Vec2 authoritativePlayerPosition{ 0.0f, 0.0f };
        bool movementTargetActive = false;
        TileCoord movementTargetTile{};
        std::vector<TileCoord> currentPath{};
        size_t pathIndex = 0;

        std::vector<ReplicatedEntityState> entities{};
        std::vector<std::string> eventLog{};

        PlayerActorRuntimeState playerActorState{};
        CombatEncounterState combatEncounterState{};
        MissionRuntimeState missionRuntimeState{};
        ShipRuntimeState shipRuntimeState{};
        OrbitalRuntimeState orbitalRuntimeState{};

        uint64_t hazardTicks = 0;
        uint64_t terrainConsequenceEvents = 0;
        uint32_t persistenceSchemaVersion = 7;
        uint32_t persistenceLoadedSchemaVersion = 0;
        uint32_t persistenceMigratedFromSchemaVersion = 0;
        uint64_t persistenceEpochMilliseconds = 0;
        std::string persistenceSlotName = "primary";
        std::string currentHazardLabel;
        std::string currentTerrainConsequence;
    };

    struct SharedSimulationDiagnostics
    {
        bool localAuthorityActive = true;
        bool fixedStepEnabled = true;
        bool hostAuthorityActive = false;
        bool clientPredictionEnabled = false;
        bool latencyHarnessEnabled = false;
        bool playerInHazard = false;
        bool combatActive = false;
        bool persistenceActive = false;
        bool persistenceDataLoaded = false;
        bool persistenceMigrationApplied = false;
        bool lastPersistenceSaveSucceeded = false;
        bool lastPersistenceLoadSucceeded = false;
        bool missionActive = true;
        bool missionComplete = false;
        bool missionGateLocked = true;
        bool shipRuntimePrepReady = false;
        bool shipActive = true;
        bool shipBoarded = false;
        bool shipDocked = true;
        bool shipPowerOnline = false;
        bool shipAirlockPressurized = false;
        bool shipCommandClaimed = false;
        bool shipLaunchPrepReady = false;
        bool orbitalLayerActive = false;
        bool orbitalDepartureAuthorized = false;
        bool orbitalTravelInProgress = false;
        bool orbitalSurveyOrbitReached = false;
        bool orbitalRelayTrackReached = false;
        bool orbitalRelayPlatformDocked = false;
        bool orbitalReturnRouteAuthorized = false;
        bool orbitalHomeDockReached = false;
        bool frontierSurfaceActive = false;

        float fixedStepSeconds = 0.05f;
        float accumulatorSeconds = 0.0f;
        float presentationAlpha = 0.0f;
        float lastPositionDivergenceDistance = 0.0f;

        uint32_t intentLatencyMilliseconds = 0;
        uint32_t acknowledgementLatencyMilliseconds = 0;
        uint32_t snapshotLatencyMilliseconds = 0;
        uint32_t jitterMilliseconds = 0;

        uint64_t renderedFrames = 0;
        uint64_t simulationTicks = 0;
        uint64_t intentsQueued = 0;
        uint64_t intentsProcessed = 0;
        uint64_t intentsAcknowledged = 0;
        uint64_t intentsRejected = 0;
        uint64_t correctionsApplied = 0;
        uint64_t divergenceEvents = 0;
        uint64_t lastIntentSequence = 0;
        uint64_t lastAcknowledgedSequence = 0;
        uint64_t lastRejectedSequence = 0;
        uint64_t lastSnapshotSequence = 0;
        uint64_t lastSnapshotSimulationTicks = 0;
        uint64_t lastSnapshotAgeMilliseconds = 0;
        uint64_t snapshotReadFailures = 0;
        uint64_t hazardTicks = 0;
        uint64_t terrainConsequenceEvents = 0;
        uint64_t combatRoundsResolved = 0;
        uint64_t persistenceSaveCount = 0;
        uint64_t persistenceLoadCount = 0;
        uint64_t lastPersistenceSaveEpochMilliseconds = 0;
        uint64_t lastPersistenceLoadEpochMilliseconds = 0;
        uint64_t missionAdvancementCount = 0;
        uint64_t shipBoardingCount = 0;
        uint64_t orbitalTransferCount = 0;

        size_t pendingIntentCount = 0;
        bool movementTargetActive = false;
        bool lastPathDivergence = false;
        bool lastEntityDivergence = false;
        bool lastSnapshotReadFailed = false;
        uint32_t persistenceSchemaVersion = 7;
        uint32_t persistenceLoadedSchemaVersion = 0;
        uint32_t persistenceMigratedFromSchemaVersion = 0;

        int playerHealth = 100;
        int playerMaxHealth = 100;
        int playerArmor = 25;
        int suitIntegrity = 100;
        float oxygenSecondsRemaining = 300.0f;
        int radiationDose = 0;
        int toxicExposure = 0;
        int hostileHealth = 0;
        int hostileMaxHealth = 0;
        uint32_t combatRoundNumber = 0;
        uint32_t combatTicksRemaining = 0;
        uint32_t inventoryStackCount = 0;
        uint32_t inventoryItemCount = 0;
        uint32_t orbitalTravelTicksRemaining = 0;
        uint64_t lootCollections = 0;
        uint64_t encounterWins = 0;
        uint64_t encountersSurvived = 0;

        std::string currentHazardLabel;
        std::string currentTerrainConsequence;
        std::string equippedWeaponText = "none";
        std::string equippedSuitText = "none";
        std::string equippedToolText = "none";
        std::string inventorySummary = "empty";
        std::string currentCombatLabel = "none";
        std::string hostileLabel = "none";
        std::string persistenceSlotName = "primary";
        std::string lastPersistenceError;
        std::string lastSnapshotReadError;
        std::string activeMissionId = "planetary-mission-slice";
        std::string missionPhaseText = "recover-transit-data";
        std::string missionObjectiveText = "Reach the Transit Service Terminal and recover route data.";
        std::string missionLastBeat = "Mission initialized";
        std::string activeShipId = "responder-shuttle-khepri";
        std::string shipName = "Responder Shuttle Khepri";
        std::string shipOwnershipText = "mission-assigned";
        std::string shipOccupancyText = "planet-surface";
        std::string shipLocationText = "cargo-bay-dock";
        std::string shipLastBeat = "Ship runtime awaiting authorization.";
        std::string orbitalPhaseText = "docked";
        std::string orbitalCurrentNodeText = "docked-anchor";
        std::string orbitalTargetNodeText = "docked-anchor";
        std::string orbitalRuleText = "Local orbit remains locked until shuttle command is claimed.";
        std::string orbitalLastBeat = "Orbital layer inactive.";
        std::string frontierSiteText = "khepri-dock";
        std::string playerRuntimeContextText = "planet-surface";
    };

    [[nodiscard]] inline const char* simulationIntentTypeText(SimulationIntentType type)
    {
        switch (type)
        {
        case SimulationIntentType::MoveToTile: return "move";
        case SimulationIntentType::InspectTile: return "inspect";
        case SimulationIntentType::InteractTile: return "interact";
        default: return "unknown";
        }
    }

    [[nodiscard]] inline const char* simulationIntentAckResultText(SimulationIntentAckResult result)
    {
        switch (result)
        {
        case SimulationIntentAckResult::Accepted: return "accepted";
        case SimulationIntentAckResult::Rejected: return "rejected";
        default: return "none";
        }
    }

    [[nodiscard]] inline const char* inventoryItemText(InventoryItemId id)
    {
        switch (id)
        {
        case InventoryItemId::RationPack: return "Ration Pack";
        case InventoryItemId::ScrapBundle: return "Scrap Bundle";
        case InventoryItemId::AccessBadge: return "Access Badge";
        case InventoryItemId::MedInjector: return "Med Injector";
        case InventoryItemId::MaintenanceKey: return "Maintenance Key";
        case InventoryItemId::SealantKit: return "Sealant Kit";
        case InventoryItemId::ShockPistol: return "Shock Pistol";
        case InventoryItemId::ContainmentMask: return "Containment Mask";
        case InventoryItemId::SurveyScanner: return "Survey Scanner";
        case InventoryItemId::UtilitySuit: return "Utility Suit";
        case InventoryItemId::PatchKit: return "Patch Kit";
        default: return "None";
        }
    }

    [[nodiscard]] inline bool inventoryItemIsWeapon(InventoryItemId id)
    {
        return id == InventoryItemId::ShockPistol;
    }

    [[nodiscard]] inline bool inventoryItemIsSuit(InventoryItemId id)
    {
        return id == InventoryItemId::ContainmentMask || id == InventoryItemId::UtilitySuit;
    }

    [[nodiscard]] inline bool inventoryItemIsTool(InventoryItemId id)
    {
        return id == InventoryItemId::MaintenanceKey
            || id == InventoryItemId::SealantKit
            || id == InventoryItemId::SurveyScanner
            || id == InventoryItemId::PatchKit;
    }

    [[nodiscard]] inline int inventoryItemArmorBonus(InventoryItemId id)
    {
        switch (id)
        {
        case InventoryItemId::ContainmentMask: return 4;
        case InventoryItemId::UtilitySuit: return 8;
        default: return 0;
        }
    }

    [[nodiscard]] inline int inventoryItemWeaponMinDamage(InventoryItemId id)
    {
        switch (id)
        {
        case InventoryItemId::ShockPistol: return 9;
        default: return 4;
        }
    }

    [[nodiscard]] inline int inventoryItemWeaponMaxDamage(InventoryItemId id)
    {
        switch (id)
        {
        case InventoryItemId::ShockPistol: return 14;
        default: return 7;
        }
    }

    [[nodiscard]] inline const char* missionIdText(MissionId id)
    {
        switch (id)
        {
        case MissionId::PlanetaryMissionSlice: return "planetary-mission-slice";
        default: return "none";
        }
    }

    [[nodiscard]] inline const char* missionPhaseText(MissionPhase phase)
    {
        switch (phase)
        {
        case MissionPhase::RecoverTransitData: return "recover-transit-data";
        case MissionPhase::DiagnoseMedlab: return "diagnose-medlab";
        case MissionPhase::SecureQuarantineGate: return "secure-quarantine-gate";
        case MissionPhase::RestoreQuarantineControl: return "restore-quarantine-control";
        case MissionPhase::BoardResponderShuttle: return "board-responder-shuttle";
        case MissionPhase::ClaimShuttleCommand: return "claim-shuttle-command";
        case MissionPhase::EnterOrbitalLane: return "enter-orbital-lane";
        case MissionPhase::ReachSurveyOrbit: return "reach-survey-orbit";
        case MissionPhase::StabilizeRelayTrack: return "stabilize-relay-track";
        case MissionPhase::DockRelayPlatform: return "dock-relay-platform";
        case MissionPhase::LandDustFrontier: return "land-dust-frontier";
        case MissionPhase::SecureFrontierRelay: return "secure-frontier-relay";
        case MissionPhase::ReturnToShuttle: return "return-to-shuttle";
        case MissionPhase::ReturnToHomeDock: return "return-to-home-dock";
        case MissionPhase::MissionComplete: return "mission-complete";
        default: return "none";
        }
    }

    [[nodiscard]] inline const char* shipIdText(ShipId id)
    {
        switch (id)
        {
        case ShipId::ResponderShuttleKhepri: return "responder-shuttle-khepri";
        default: return "none";
        }
    }

    [[nodiscard]] inline const char* shipOwnershipStateText(ShipOwnershipState state)
    {
        switch (state)
        {
        case ShipOwnershipState::MissionAssigned: return "mission-assigned";
        case ShipOwnershipState::PlayerCommand: return "player-command";
        default: return "none";
        }
    }

    [[nodiscard]] inline const char* shipOccupancyStateText(ShipOccupancyState state)
    {
        switch (state)
        {
        case ShipOccupancyState::PlanetSurface: return "planet-surface";
        case ShipOccupancyState::AboardDockedShip: return "aboard-docked-ship";
        case ShipOccupancyState::AboardOrbitalShip: return "aboard-orbital-ship";
        default: return "none";
        }
    }

    [[nodiscard]] inline const char* orbitalNodeText(OrbitalNodeId node)
    {
        switch (node)
        {
        case OrbitalNodeId::DockedAnchor: return "docked-anchor";
        case OrbitalNodeId::TrafficSeparationLane: return "traffic-separation-lane";
        case OrbitalNodeId::DebrisSurveyOrbit: return "debris-survey-orbit";
        case OrbitalNodeId::RelayHoldingTrack: return "relay-holding-track";
        case OrbitalNodeId::RelayPlatformDock: return "relay-platform-dock";
        case OrbitalNodeId::ReturnTrafficLane: return "return-traffic-lane";
        case OrbitalNodeId::HomeDockAnchor: return "home-dock-anchor";
        default: return "none";
        }
    }

    [[nodiscard]] inline const char* orbitalTravelPhaseText(OrbitalTravelPhase phase)
    {
        switch (phase)
        {
        case OrbitalTravelPhase::Docked: return "docked";
        case OrbitalTravelPhase::UndockingTransfer: return "undocking-transfer";
        case OrbitalTravelPhase::TrafficLaneHolding: return "traffic-lane-holding";
        case OrbitalTravelPhase::SurveyTransfer: return "survey-transfer";
        case OrbitalTravelPhase::SurveyHolding: return "survey-holding";
        case OrbitalTravelPhase::RelayTransfer: return "relay-transfer";
        case OrbitalTravelPhase::RelayHolding: return "relay-holding";
        case OrbitalTravelPhase::DockingTransfer: return "docking-transfer";
        case OrbitalTravelPhase::RelayPlatformDocked: return "relay-platform-docked";
        case OrbitalTravelPhase::ReturnLaunchTransfer: return "return-launch-transfer";
        case OrbitalTravelPhase::HomeDockTransfer: return "home-dock-transfer";
        case OrbitalTravelPhase::HomeDocked: return "home-docked";
        default: return "none";
        }
    }
}
