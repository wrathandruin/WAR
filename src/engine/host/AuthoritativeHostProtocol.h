#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"
#include "engine/simulation/SimulationIntent.h"

namespace war
{
    struct AuthoritativeHostProtocolReport
    {
        bool intentQueueReady = false;
        bool acknowledgementQueueReady = false;
        bool snapshotPresent = false;
        bool authorityLaneReady = false;

        std::filesystem::path intentQueueDirectory;
        std::filesystem::path acknowledgementQueueDirectory;
        std::filesystem::path snapshotPath;

        std::vector<std::string> issues;
    };

    class AuthoritativeHostProtocol
    {
    public:
        [[nodiscard]] static AuthoritativeHostProtocolReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport);
        static void ensureDirectories(AuthoritativeHostProtocolReport& report);

        [[nodiscard]] static bool writeIntentRequest(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SimulationIntent& intent,
            std::string& outError);

        [[nodiscard]] static std::vector<SimulationIntent> collectPendingIntentRequestsForHost(
            const RuntimeBoundaryReport& runtimeBoundaryReport);

        [[nodiscard]] static bool writeAcknowledgement(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SimulationIntentAck& acknowledgement,
            std::string& outError);

        [[nodiscard]] static std::vector<SimulationIntentAck> collectAcknowledgementsForClient(
            const RuntimeBoundaryReport& runtimeBoundaryReport);

        [[nodiscard]] static bool writeAuthoritativeSnapshot(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const AuthoritativeWorldSnapshot& snapshot,
            std::string& outError);

        [[nodiscard]] static AuthoritativeWorldSnapshot readAuthoritativeSnapshot(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            std::string& outError);

    private:
        [[nodiscard]] static std::filesystem::path intentQueueDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport);
        [[nodiscard]] static std::filesystem::path acknowledgementQueueDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport);
        [[nodiscard]] static std::filesystem::path snapshotPath(const RuntimeBoundaryReport& runtimeBoundaryReport);
    };
}
