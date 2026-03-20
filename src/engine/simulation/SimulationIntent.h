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

    enum class InventoryItemId
    {
        None,
        MedInjector,
        MaintenanceKey,
        ShockPistol,
        UtilityHarness,
        SurveyScanner,
        SealantKit,
        RationPack,
        ScrapBundle,
        ContainmentMask,
        AccessBadge
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
        int healthCurrent = 100;
        int healthMax = 100;
        int carryCapacity = 12;
        int armorRating = 0;
        std::vector<InventoryItemStack> inventory;
        PlayerEquipmentState equipment{};
        uint64_t lootCollections = 0;
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
        uint32_t schemaVersion = 1;
        uint32_t migratedFromSchemaVersion = 0;
        uint64_t simulationTicks = 0;
        uint64_t lastProcessedIntentSequence = 0;
        uint64_t nextIntentSequence = 1;
        uint64_t publishedEpochMilliseconds = 0;
        Vec2 authoritativePlayerPosition{ 0.0f, 0.0f };
        bool movementTargetActive = false;
        TileCoord movementTargetTile{};
        std::vector<TileCoord> currentPath;
        size_t pathIndex = 0;
        std::vector<ReplicatedEntityState> entities;
        std::vector<std::string> eventLog;
        PlayerActorRuntimeState playerActor{};
    };

    struct SharedSimulationDiagnostics
    {
        bool localAuthorityActive = true;
        bool fixedStepEnabled = true;
        bool hostAuthorityActive = false;
        bool clientPredictionEnabled = false;
        bool latencyHarnessEnabled = false;
        bool actorRuntimeActive = false;
        float fixedStepSeconds = 0.05f;
        float accumulatorSeconds = 0.0f;
        float presentationAlpha = 0.0f;
        float lastPositionDivergenceDistance = 0.0f;
        uint32_t intentLatencyMilliseconds = 0;
        uint32_t acknowledgementLatencyMilliseconds = 0;
        uint32_t snapshotLatencyMilliseconds = 0;
        uint32_t jitterMilliseconds = 0;
        uint32_t persistenceSchemaVersion = 3;
        uint32_t loadedPersistenceSchemaVersion = 0;
        uint32_t migratedFromPersistenceSchemaVersion = 0;
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
        uint64_t persistenceLoadCount = 0;
        uint64_t persistenceSaveCount = 0;
        uint64_t lastPersistenceEpochMilliseconds = 0;
        size_t pendingIntentCount = 0;
        bool movementTargetActive = false;
        bool lastPathDivergence = false;
        bool lastEntityDivergence = false;
        bool lastActorDivergence = false;
        bool lastSnapshotReadFailed = false;
        bool lastPersistenceSucceeded = false;
        std::string lastSnapshotReadError;
        std::string lastPersistenceError;
        int playerHealthCurrent = 0;
        int playerHealthMax = 0;
        int playerArmorRating = 0;
        size_t inventoryStackCount = 0;
        uint32_t inventoryItemCount = 0;
        uint64_t lootCollections = 0;
        std::string equippedWeaponName;
        std::string equippedSuitName;
        std::string equippedToolName;
    };

    [[nodiscard]] inline const char* simulationIntentTypeText(SimulationIntentType type)
    {
        switch (type)
        {
        case SimulationIntentType::MoveToTile:
            return "move";
        case SimulationIntentType::InspectTile:
            return "inspect";
        case SimulationIntentType::InteractTile:
            return "interact";
        default:
            return "unknown";
        }
    }

    [[nodiscard]] inline const char* simulationIntentAckResultText(SimulationIntentAckResult result)
    {
        switch (result)
        {
        case SimulationIntentAckResult::Accepted:
            return "accepted";
        case SimulationIntentAckResult::Rejected:
            return "rejected";
        default:
            return "none";
        }
    }

    [[nodiscard]] inline const char* inventoryItemText(InventoryItemId id)
    {
        switch (id)
        {
        case InventoryItemId::MedInjector:
            return "Med Injector";
        case InventoryItemId::MaintenanceKey:
            return "Maintenance Key";
        case InventoryItemId::ShockPistol:
            return "Shock Pistol";
        case InventoryItemId::UtilityHarness:
            return "Utility Harness";
        case InventoryItemId::SurveyScanner:
            return "Survey Scanner";
        case InventoryItemId::SealantKit:
            return "Sealant Kit";
        case InventoryItemId::RationPack:
            return "Ration Pack";
        case InventoryItemId::ScrapBundle:
            return "Scrap Bundle";
        case InventoryItemId::ContainmentMask:
            return "Containment Mask";
        case InventoryItemId::AccessBadge:
            return "Access Badge";
        default:
            return "None";
        }
    }

    [[nodiscard]] inline const char* inventoryItemCategoryText(InventoryItemId id)
    {
        switch (id)
        {
        case InventoryItemId::ShockPistol:
            return "weapon";
        case InventoryItemId::UtilityHarness:
        case InventoryItemId::ContainmentMask:
            return "suit";
        case InventoryItemId::SurveyScanner:
        case InventoryItemId::MaintenanceKey:
            return "tool";
        case InventoryItemId::MedInjector:
        case InventoryItemId::SealantKit:
        case InventoryItemId::RationPack:
            return "consumable";
        case InventoryItemId::ScrapBundle:
            return "material";
        case InventoryItemId::AccessBadge:
            return "key-item";
        default:
            return "none";
        }
    }

    [[nodiscard]] inline bool inventoryItemIsWeapon(InventoryItemId id)
    {
        return id == InventoryItemId::ShockPistol;
    }

    [[nodiscard]] inline bool inventoryItemIsSuit(InventoryItemId id)
    {
        return id == InventoryItemId::UtilityHarness || id == InventoryItemId::ContainmentMask;
    }

    [[nodiscard]] inline bool inventoryItemIsTool(InventoryItemId id)
    {
        return id == InventoryItemId::SurveyScanner || id == InventoryItemId::MaintenanceKey;
    }

    [[nodiscard]] inline int inventoryItemArmorBonus(InventoryItemId id)
    {
        switch (id)
        {
        case InventoryItemId::UtilityHarness:
            return 2;
        case InventoryItemId::ContainmentMask:
            return 3;
        default:
            return 0;
        }
    }
}
