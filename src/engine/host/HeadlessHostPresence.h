#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"
#include "engine/host/ReplicationHarness.h"
#include "engine/simulation/SimulationIntent.h"

namespace war
{
    struct HeadlessHostPresenceReport
    {
        bool statusFilePresent = false;
        bool statusParseValid = false;
        bool heartbeatFieldValid = false;
        bool hostOnline = false;
        bool heartbeatFresh = false;
        bool authorityHostAdvertised = false;
        bool localBootstrapLaneReady = false;
        bool latencyHarnessEnabled = false;

        uint64_t heartbeatAgeMilliseconds = 0;
        uint64_t advertisedSimulationTicks = 0;
        uint64_t pendingInboundIntentCount = 0;
        uint64_t pendingOutboundAcknowledgementCount = 0;
        uint64_t pendingSnapshotCount = 0;
        uint32_t hostTickMilliseconds = 0;

        std::string hostMode = "offline";
        std::string hostState = "unseen";
        std::string hostPid = "none";
        std::filesystem::path statusFilePath;
        ReplicationHarnessConfig harnessConfig{};

        std::vector<std::string> issues;
    };

    class HeadlessHostPresence
    {
    public:
        [[nodiscard]] static HeadlessHostPresenceReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport);
        static void writeStatus(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SharedSimulationDiagnostics& simulationDiagnostics,
            uint32_t hostTickMilliseconds,
            uint32_t processId,
            const std::string& hostState,
            const ReplicationHarnessConfig& harnessConfig,
            size_t pendingInboundIntentCount,
            size_t pendingOutboundAcknowledgementCount,
            size_t pendingSnapshotCount);

    private:
        [[nodiscard]] static uint64_t currentEpochMilliseconds();
    };
}
