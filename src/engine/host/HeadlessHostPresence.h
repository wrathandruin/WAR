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
        bool persistenceSavePresent = false;
        bool persistenceLastSaveSucceeded = false;
        bool persistenceLastLoadSucceeded = false;
        bool persistenceMigrationApplied = false;
        bool persistenceRestoreVisible = false;

        uint64_t heartbeatAgeMilliseconds = 0;
        uint64_t advertisedSimulationTicks = 0;
        uint64_t pendingInboundIntentCount = 0;
        uint64_t pendingOutboundAcknowledgementCount = 0;
        uint64_t pendingSnapshotCount = 0;
        uint64_t persistenceSaveCount = 0;
        uint64_t persistenceLoadCount = 0;
        uint64_t lastPersistenceSaveEpochMilliseconds = 0;
        uint64_t lastPersistenceLoadEpochMilliseconds = 0;
        uint32_t hostTickMilliseconds = 0;
        uint32_t persistenceSchemaVersion = 0;
        uint32_t persistenceLoadedSchemaVersion = 0;
        uint32_t persistenceMigratedFromSchemaVersion = 0;
        uint32_t protocolVersion = 0;

        std::string hostMode = "offline";
        std::string hostState = "unseen";
        std::string hostPid = "none";
        std::string persistenceSlotName = "primary";
        std::string transportKind = "unknown";
        std::string connectTargetName = "unresolved";
        std::string connectLaneMode = "unresolved";
        std::string buildConfiguration = "unknown";
        std::string buildTimestamp = "unknown";
        std::string buildIdentity = "unknown";
        std::string buildChannel = "unknown";
        std::string hostInstanceId = "none";
        std::string sessionId = "none";
        std::string runtimeRootPath = "unresolved";
        std::string restoreState = "unseen";
        std::filesystem::path statusFilePath;
        std::filesystem::path persistentSavePath;
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
