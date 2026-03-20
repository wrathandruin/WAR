#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"
#include "engine/simulation/SimulationIntent.h"

namespace war
{
    struct HeadlessHostPresenceReport
    {
        bool statusFilePresent = false;
        bool hostOnline = false;
        bool heartbeatFresh = false;
        bool authorityHostAdvertised = false;
        bool localBootstrapLaneReady = false;

        uint64_t heartbeatAgeMilliseconds = 0;
        uint64_t advertisedSimulationTicks = 0;
        uint32_t hostTickMilliseconds = 0;

        std::string hostMode = "offline";
        std::string hostState = "unseen";
        std::string hostPid = "none";
        std::filesystem::path statusFilePath;

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
            const std::string& hostState);

    private:
        [[nodiscard]] static uint64_t currentEpochMilliseconds();
    };
}
