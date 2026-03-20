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
    };

    struct AuthoritativeWorldSnapshot
    {
        bool valid = false;
        uint64_t simulationTicks = 0;
        uint64_t lastProcessedIntentSequence = 0;
        uint64_t publishedEpochMilliseconds = 0;
        Vec2 authoritativePlayerPosition{ 0.0f, 0.0f };
        bool movementTargetActive = false;
        TileCoord movementTargetTile{};
        std::vector<TileCoord> currentPath;
        size_t pathIndex = 0;
        std::vector<ReplicatedEntityState> entities;
        std::vector<std::string> eventLog;
    };

    struct SharedSimulationDiagnostics
    {
        bool localAuthorityActive = true;
        bool fixedStepEnabled = true;
        bool hostAuthorityActive = false;
        bool clientPredictionEnabled = false;
        bool latencyHarnessEnabled = false;
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
        size_t pendingIntentCount = 0;
        bool movementTargetActive = false;
        bool lastPathDivergence = false;
        bool lastEntityDivergence = false;
        bool lastSnapshotReadFailed = false;
        std::string lastSnapshotReadError;
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
}
