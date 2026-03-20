#pragma once

#include <cstdint>
#include <cstddef>

#include "engine/world/WorldGrid.h"

namespace war
{
    enum class SimulationIntentType
    {
        MoveToTile,
        InspectTile,
        InteractTile
    };

    struct SimulationIntent
    {
        uint64_t sequence = 0;
        SimulationIntentType type = SimulationIntentType::MoveToTile;
        TileCoord target{};
    };

    struct SharedSimulationDiagnostics
    {
        bool localAuthorityActive = true;
        bool fixedStepEnabled = true;
        float fixedStepSeconds = 0.05f;
        float accumulatorSeconds = 0.0f;
        float presentationAlpha = 0.0f;
        uint64_t renderedFrames = 0;
        uint64_t simulationTicks = 0;
        uint64_t intentsQueued = 0;
        uint64_t intentsProcessed = 0;
        uint64_t lastIntentSequence = 0;
        size_t pendingIntentCount = 0;
        bool movementTargetActive = false;
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
}
