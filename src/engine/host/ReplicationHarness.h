#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "engine/core/RuntimePaths.h"

namespace war
{
    struct ReplicationHarnessConfig
    {
        bool enabled = false;
        uint32_t intentLatencyMilliseconds = 0;
        uint32_t acknowledgementLatencyMilliseconds = 0;
        uint32_t snapshotLatencyMilliseconds = 0;
        uint32_t jitterMilliseconds = 0;
    };

    class ReplicationHarness
    {
    public:
        [[nodiscard]] static std::filesystem::path configPath(const RuntimeBoundaryReport& runtimeBoundaryReport);
        [[nodiscard]] static ReplicationHarnessConfig loadConfig(const RuntimeBoundaryReport& runtimeBoundaryReport);
        [[nodiscard]] static bool saveConfig(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const ReplicationHarnessConfig& config,
            std::string& outError);

        [[nodiscard]] static uint64_t currentEpochMilliseconds();
        [[nodiscard]] static uint64_t computeArrivalEpochMilliseconds(
            uint64_t nowEpochMilliseconds,
            uint64_t sequenceBasis,
            uint32_t baseLatencyMilliseconds,
            uint32_t jitterMilliseconds);
    };
}
