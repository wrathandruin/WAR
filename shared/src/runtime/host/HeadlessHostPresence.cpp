#include "engine/host/HeadlessHostPresence.h"

#include <chrono>
#include <fstream>
#include <string>
#include <system_error>
#include <unordered_map>

namespace war
{
    namespace
    {
        using StringMap = std::unordered_map<std::string, std::string>;

        StringMap parseKeyValueFile(const std::filesystem::path& path)
        {
            StringMap values{};
            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                return values;
            }

            std::string line;
            while (std::getline(input, line))
            {
                const size_t split = line.find('=');
                if (split == std::string::npos)
                {
                    continue;
                }

                values.emplace(line.substr(0, split), line.substr(split + 1));
            }

            return values;
        }

        bool fileExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_regular_file(path, error);
        }

        bool tryParseUnsigned(const StringMap& values, const char* key, uint64_t& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            try
            {
                outValue = static_cast<uint64_t>(std::stoull(it->second));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool tryParseString(const StringMap& values, const char* key, std::string& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end() || it->second.empty())
            {
                return false;
            }

            outValue = it->second;
            return true;
        }

        bool tryParseYesNo(const StringMap& values, const char* key, bool& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            if (it->second == "yes" || it->second == "true" || it->second == "1")
            {
                outValue = true;
                return true;
            }

            if (it->second == "no" || it->second == "false" || it->second == "0")
            {
                outValue = false;
                return true;
            }

            return false;
        }
    }

    HeadlessHostPresenceReport HeadlessHostPresence::buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        HeadlessHostPresenceReport report{};
        report.statusFilePath = runtimeBoundaryReport.hostDirectory / "headless_host_status.txt";
        report.persistentSavePath = runtimeBoundaryReport.savesDirectory / "authoritative_world_primary.txt";
        report.persistenceSavePresent = fileExists(report.persistentSavePath);

        if (!fileExists(report.statusFilePath))
        {
            return report;
        }

        report.statusFilePresent = true;
        const StringMap values = parseKeyValueFile(report.statusFilePath);
        if (values.empty())
        {
            report.issues.push_back("Host status file exists but could not be parsed.");
            return report;
        }

        bool malformed = false;
        uint64_t statusVersion = 0;
        uint64_t hostTickMilliseconds = 0;
        uint64_t advertisedSimulationTicks = 0;
        uint64_t pendingInboundIntentCount = 0;
        uint64_t pendingOutboundAcknowledgementCount = 0;
        uint64_t pendingSnapshotCount = 0;
        uint64_t heartbeatEpochMilliseconds = 0;
        bool authorityHostAdvertised = false;
        bool fixedStepEnabled = false;
        bool latencyHarnessEnabled = false;
        bool intentLatencyValid = false;
        bool ackLatencyValid = false;
        bool snapshotLatencyValid = false;
        bool jitterValid = false;
        uint64_t intentLatency = 0;
        uint64_t ackLatency = 0;
        uint64_t snapshotLatency = 0;
        uint64_t jitter = 0;

        malformed |= !tryParseUnsigned(values, "status_version", statusVersion);
        malformed |= !tryParseString(values, "mode", report.hostMode);
        malformed |= !tryParseString(values, "state", report.hostState);
        malformed |= !tryParseString(values, "pid", report.hostPid);
        malformed |= !tryParseUnsigned(values, "host_tick_ms", hostTickMilliseconds);
        malformed |= !tryParseUnsigned(values, "simulation_ticks", advertisedSimulationTicks);
        malformed |= !tryParseYesNo(values, "host_authority_active", authorityHostAdvertised);
        malformed |= !tryParseYesNo(values, "fixed_step_enabled", fixedStepEnabled);
        report.heartbeatFieldValid = tryParseUnsigned(values, "heartbeat_epoch_ms", heartbeatEpochMilliseconds);
        malformed |= !report.heartbeatFieldValid;
        malformed |= !tryParseYesNo(values, "latency_harness_enabled", latencyHarnessEnabled);
        intentLatencyValid = tryParseUnsigned(values, "intent_latency_ms", intentLatency);
        ackLatencyValid = tryParseUnsigned(values, "ack_latency_ms", ackLatency);
        snapshotLatencyValid = tryParseUnsigned(values, "snapshot_latency_ms", snapshotLatency);
        jitterValid = tryParseUnsigned(values, "jitter_ms", jitter);
        malformed |= !(intentLatencyValid && ackLatencyValid && snapshotLatencyValid && jitterValid);
        malformed |= !tryParseUnsigned(values, "pending_inbound_intents", pendingInboundIntentCount);
        malformed |= !tryParseUnsigned(values, "pending_outbound_acks", pendingOutboundAcknowledgementCount);
        malformed |= !tryParseUnsigned(values, "pending_snapshots", pendingSnapshotCount);

        report.hostTickMilliseconds = static_cast<uint32_t>(hostTickMilliseconds);
        report.advertisedSimulationTicks = advertisedSimulationTicks;
        report.authorityHostAdvertised = authorityHostAdvertised && fixedStepEnabled;
        report.latencyHarnessEnabled = latencyHarnessEnabled;
        report.pendingInboundIntentCount = pendingInboundIntentCount;
        report.pendingOutboundAcknowledgementCount = pendingOutboundAcknowledgementCount;
        report.pendingSnapshotCount = pendingSnapshotCount;
        report.harnessConfig.enabled = latencyHarnessEnabled;
        report.harnessConfig.intentLatencyMilliseconds = static_cast<uint32_t>(intentLatency);
        report.harnessConfig.acknowledgementLatencyMilliseconds = static_cast<uint32_t>(ackLatency);
        report.harnessConfig.snapshotLatencyMilliseconds = static_cast<uint32_t>(snapshotLatency);
        report.harnessConfig.jitterMilliseconds = static_cast<uint32_t>(jitter);

        uint64_t persistenceSchemaVersion = 0;
        uint64_t persistenceLoadedSchemaVersion = 0;
        uint64_t persistenceMigratedFromSchemaVersion = 0;
        uint64_t persistenceSaveCount = 0;
        uint64_t persistenceLoadCount = 0;
        uint64_t lastPersistenceSaveEpochMilliseconds = 0;
        uint64_t lastPersistenceLoadEpochMilliseconds = 0;
        bool persistenceLastSaveSucceeded = false;
        bool persistenceLastLoadSucceeded = false;
        bool persistenceMigrationApplied = false;
        bool persistenceActive = false;
        bool persistenceParseValid = true;

        if (values.contains("persistence_active"))
        {
            persistenceParseValid &= tryParseYesNo(values, "persistence_active", persistenceActive);
        }
        if (values.contains("persistence_schema_version"))
        {
            persistenceParseValid &= tryParseUnsigned(values, "persistence_schema_version", persistenceSchemaVersion);
        }
        if (values.contains("persistence_loaded_schema_version"))
        {
            persistenceParseValid &= tryParseUnsigned(values, "persistence_loaded_schema_version", persistenceLoadedSchemaVersion);
        }
        if (values.contains("persistence_migrated_from_version"))
        {
            persistenceParseValid &= tryParseUnsigned(values, "persistence_migrated_from_version", persistenceMigratedFromSchemaVersion);
        }
        if (values.contains("persistence_save_count"))
        {
            persistenceParseValid &= tryParseUnsigned(values, "persistence_save_count", persistenceSaveCount);
        }
        if (values.contains("persistence_load_count"))
        {
            persistenceParseValid &= tryParseUnsigned(values, "persistence_load_count", persistenceLoadCount);
        }
        if (values.contains("persistence_last_save_epoch_ms"))
        {
            persistenceParseValid &= tryParseUnsigned(values, "persistence_last_save_epoch_ms", lastPersistenceSaveEpochMilliseconds);
        }
        if (values.contains("persistence_last_load_epoch_ms"))
        {
            persistenceParseValid &= tryParseUnsigned(values, "persistence_last_load_epoch_ms", lastPersistenceLoadEpochMilliseconds);
        }
        if (values.contains("persistence_last_save_succeeded"))
        {
            persistenceParseValid &= tryParseYesNo(values, "persistence_last_save_succeeded", persistenceLastSaveSucceeded);
        }
        if (values.contains("persistence_last_load_succeeded"))
        {
            persistenceParseValid &= tryParseYesNo(values, "persistence_last_load_succeeded", persistenceLastLoadSucceeded);
        }
        if (values.contains("persistence_migration_applied"))
        {
            persistenceParseValid &= tryParseYesNo(values, "persistence_migration_applied", persistenceMigrationApplied);
        }
        if (values.contains("persistence_slot"))
        {
            persistenceParseValid &= tryParseString(values, "persistence_slot", report.persistenceSlotName);
        }

        report.persistenceSchemaVersion = static_cast<uint32_t>(persistenceSchemaVersion);
        report.persistenceLoadedSchemaVersion = static_cast<uint32_t>(persistenceLoadedSchemaVersion);
        report.persistenceMigratedFromSchemaVersion = static_cast<uint32_t>(persistenceMigratedFromSchemaVersion);
        report.persistenceSaveCount = persistenceSaveCount;
        report.persistenceLoadCount = persistenceLoadCount;
        report.lastPersistenceSaveEpochMilliseconds = lastPersistenceSaveEpochMilliseconds;
        report.lastPersistenceLoadEpochMilliseconds = lastPersistenceLoadEpochMilliseconds;
        report.persistenceLastSaveSucceeded = persistenceLastSaveSucceeded;
        report.persistenceLastLoadSucceeded = persistenceLastLoadSucceeded;
        report.persistenceMigrationApplied = persistenceMigrationApplied;

        report.statusParseValid = !malformed && persistenceParseValid && statusVersion >= 2 && report.hostTickMilliseconds > 0;

        const uint64_t now = currentEpochMilliseconds();
        if (report.heartbeatFieldValid && heartbeatEpochMilliseconds > 0 && now >= heartbeatEpochMilliseconds)
        {
            report.heartbeatAgeMilliseconds = now - heartbeatEpochMilliseconds;
            report.heartbeatFresh = report.statusParseValid && report.heartbeatAgeMilliseconds <= 2500ull;
        }
        else
        {
            report.heartbeatAgeMilliseconds = 0;
            report.heartbeatFresh = false;
        }

        report.hostOnline = report.statusParseValid
            && report.heartbeatFresh
            && report.hostState == "running"
            && report.authorityHostAdvertised;
        report.localBootstrapLaneReady = report.statusParseValid && report.authorityHostAdvertised && persistenceActive;

        if (!report.statusParseValid)
        {
            report.issues.push_back("Headless host status file is malformed or missing required fields.");
        }
        else if (!report.heartbeatFresh && report.hostState == "running")
        {
            report.issues.push_back("Headless host heartbeat is stale or missing.");
        }
        else if (!report.authorityHostAdvertised)
        {
            report.issues.push_back("Headless host status does not advertise authoritative fixed-step ownership.");
        }

        return report;
    }

    void HeadlessHostPresence::writeStatus(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const SharedSimulationDiagnostics& simulationDiagnostics,
        uint32_t hostTickMilliseconds,
        uint32_t processId,
        const std::string& hostState,
        const ReplicationHarnessConfig& harnessConfig,
        size_t pendingInboundIntentCount,
        size_t pendingOutboundAcknowledgementCount,
        size_t pendingSnapshotCount)
    {
        std::error_code error;
        std::filesystem::create_directories(runtimeBoundaryReport.hostDirectory, error);
        if (error)
        {
            return;
        }

        const std::filesystem::path finalPath = runtimeBoundaryReport.hostDirectory / "headless_host_status.txt";
        const std::filesystem::path tempPath = runtimeBoundaryReport.hostDirectory / "headless_host_status.tmp";
        const std::filesystem::path persistentSavePath = runtimeBoundaryReport.savesDirectory / "authoritative_world_primary.txt";
        const bool persistenceSavePresent = fileExists(persistentSavePath);

        std::ofstream output(tempPath, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            return;
        }

        output
            << "status_version=3\n"
            << "mode=headless-authoritative-host\n"
            << "state=" << hostState << "\n"
            << "pid=" << processId << "\n"
            << "host_tick_ms=" << hostTickMilliseconds << "\n"
            << "heartbeat_epoch_ms=" << currentEpochMilliseconds() << "\n"
            << "simulation_ticks=" << simulationDiagnostics.simulationTicks << "\n"
            << "host_authority_active=" << (simulationDiagnostics.hostAuthorityActive ? "yes" : "no") << "\n"
            << "fixed_step_enabled=" << (simulationDiagnostics.fixedStepEnabled ? "yes" : "no") << "\n"
            << "fixed_step_seconds=" << simulationDiagnostics.fixedStepSeconds << "\n"
            << "intents_processed=" << simulationDiagnostics.intentsProcessed << "\n"
            << "latency_harness_enabled=" << (harnessConfig.enabled ? "yes" : "no") << "\n"
            << "intent_latency_ms=" << harnessConfig.intentLatencyMilliseconds << "\n"
            << "ack_latency_ms=" << harnessConfig.acknowledgementLatencyMilliseconds << "\n"
            << "snapshot_latency_ms=" << harnessConfig.snapshotLatencyMilliseconds << "\n"
            << "jitter_ms=" << harnessConfig.jitterMilliseconds << "\n"
            << "pending_inbound_intents=" << pendingInboundIntentCount << "\n"
            << "pending_outbound_acks=" << pendingOutboundAcknowledgementCount << "\n"
            << "pending_snapshots=" << pendingSnapshotCount << "\n"
            << "persistence_active=" << (simulationDiagnostics.persistenceActive ? "yes" : "no") << "\n"
            << "persistence_slot=" << simulationDiagnostics.persistenceSlotName << "\n"
            << "persistence_save_present=" << (persistenceSavePresent ? "yes" : "no") << "\n"
            << "persistence_schema_version=" << simulationDiagnostics.persistenceSchemaVersion << "\n"
            << "persistence_loaded_schema_version=" << simulationDiagnostics.persistenceLoadedSchemaVersion << "\n"
            << "persistence_migrated_from_version=" << simulationDiagnostics.persistenceMigratedFromSchemaVersion << "\n"
            << "persistence_migration_applied=" << (simulationDiagnostics.persistenceMigrationApplied ? "yes" : "no") << "\n"
            << "persistence_last_save_succeeded=" << (simulationDiagnostics.lastPersistenceSaveSucceeded ? "yes" : "no") << "\n"
            << "persistence_last_load_succeeded=" << (simulationDiagnostics.lastPersistenceLoadSucceeded ? "yes" : "no") << "\n"
            << "persistence_save_count=" << simulationDiagnostics.persistenceSaveCount << "\n"
            << "persistence_load_count=" << simulationDiagnostics.persistenceLoadCount << "\n"
            << "persistence_last_save_epoch_ms=" << simulationDiagnostics.lastPersistenceSaveEpochMilliseconds << "\n"
            << "persistence_last_load_epoch_ms=" << simulationDiagnostics.lastPersistenceLoadEpochMilliseconds << "\n";
        output.close();
        if (!output)
        {
            return;
        }

        std::filesystem::remove(finalPath, error);
        error.clear();
        std::filesystem::rename(tempPath, finalPath, error);
        if (error)
        {
            std::filesystem::remove(tempPath, error);
        }
    }

    uint64_t HeadlessHostPresence::currentEpochMilliseconds()
    {
        using namespace std::chrono;
        return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    }
}
