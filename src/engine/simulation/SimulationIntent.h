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

    struct SimulationIntent
    {
        uint64_t sequence = 0;
        SimulationIntentType type = SimulationIntentType::MoveToTile;
        TileCoord target{};
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

        uint64_t hazardTicks = 0;
        uint64_t terrainConsequenceEvents = 0;
        uint32_t persistenceSchemaVersion = 3;
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

        size_t pendingIntentCount = 0;
        bool movementTargetActive = false;
        bool lastPathDivergence = false;
        bool lastEntityDivergence = false;
        bool lastSnapshotReadFailed = false;
        uint32_t persistenceSchemaVersion = 3;
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
}
