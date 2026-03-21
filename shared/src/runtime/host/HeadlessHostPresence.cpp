#include "engine/host/HeadlessHostPresence.h"

#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <windows.h>

namespace war
{
    namespace
    {
        using StringMap = std::unordered_map<std::string, std::string>;
        constexpr uint32_t kCurrentProtocolVersion = 2u;

        bool readTextFileForAtomicReplace(const std::filesystem::path& path, std::string& outContents)
        {
            outContents.clear();
        #if defined(_WIN32)
            HANDLE handle = CreateFileW(
                path.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);
            if (handle == INVALID_HANDLE_VALUE)
            {
                return false;
            }

            LARGE_INTEGER fileSize{};
            if (!GetFileSizeEx(handle, &fileSize) || fileSize.QuadPart < 0)
            {
                CloseHandle(handle);
                return false;
            }

            const DWORD requestedBytes = static_cast<DWORD>(fileSize.QuadPart);
            std::vector<char> buffer(static_cast<size_t>(requestedBytes), '\0');
            DWORD bytesRead = 0;
            const BOOL readOk = requestedBytes == 0
                ? TRUE
                : ReadFile(handle, buffer.data(), requestedBytes, &bytesRead, nullptr);
            CloseHandle(handle);
            if (!readOk)
            {
                return false;
            }

            outContents.assign(buffer.data(), buffer.data() + bytesRead);
            return true;
        #else
            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                return false;
            }

            std::ostringstream stream;
            stream << input.rdbuf();
            outContents = stream.str();
            return static_cast<bool>(input) || input.eof();
        #endif
        }

        StringMap parseKeyValueText(const std::string& contents)
        {
            StringMap values{};
            std::istringstream input(contents);
            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                const size_t split = line.find('=');
                if (split == std::string::npos)
                {
                    continue;
                }

                values.emplace(line.substr(0, split), line.substr(split + 1));
            }

            return values;
        }

        StringMap parseKeyValueFile(const std::filesystem::path& path)
        {
            std::string contents;
            if (!readTextFileForAtomicReplace(path, contents))
            {
                return {};
            }

            return parseKeyValueText(contents);
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

        std::wstring readEnvironmentWide(const wchar_t* variableName)
        {
            const DWORD requiredLength = GetEnvironmentVariableW(variableName, nullptr, 0);
            if (requiredLength == 0)
            {
                return {};
            }

            std::wstring buffer(static_cast<size_t>(requiredLength), L'\0');
            const DWORD writtenLength = GetEnvironmentVariableW(variableName, buffer.data(), requiredLength);
            if (writtenLength == 0)
            {
                return {};
            }

            buffer.resize(static_cast<size_t>(writtenLength));
            return buffer;
        }

        std::string narrowText(const std::wstring& value)
        {
            if (value.empty())
            {
                return {};
            }

            const int requiredBytes = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.c_str(),
                static_cast<int>(value.size()),
                nullptr,
                0,
                nullptr,
                nullptr);
            if (requiredBytes <= 0)
            {
                return {};
            }

            std::string result(static_cast<size_t>(requiredBytes), '\0');
            const int writtenBytes = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.c_str(),
                static_cast<int>(value.size()),
                result.data(),
                requiredBytes,
                nullptr,
                nullptr);
            if (writtenBytes <= 0)
            {
                return {};
            }

            result.resize(static_cast<size_t>(writtenBytes));
            return result;
        }

        std::string environmentText(const wchar_t* variableName)
        {
            return narrowText(readEnvironmentWide(variableName));
        }

        std::string buildConfigurationText()
        {
        #if defined(_DEBUG)
            return "Debug";
        #else
            return "Release";
        #endif
        }

        std::string buildTimestampText()
        {
            return std::string(__DATE__) + " " + std::string(__TIME__);
        }

        uint64_t currentSystemEpochMilliseconds()
        {
            using namespace std::chrono;
            return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
        }

        std::string stableHostInstanceId(uint32_t processId)
        {
            static std::string cachedId{};
            if (cachedId.empty())
            {
                cachedId = std::string("host-")
                    + std::to_string(processId)
                    + "-"
                    + std::to_string(currentSystemEpochMilliseconds());
            }

            return cachedId;
        }

        std::string stableSessionId(uint32_t processId)
        {
            static std::string cachedId{};
            if (cachedId.empty())
            {
                cachedId = std::string("session-")
                    + std::to_string(processId)
                    + "-"
                    + std::to_string(currentSystemEpochMilliseconds());
            }

            return cachedId;
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
        uint64_t protocolVersion = 0;
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
        malformed |= !tryParseUnsigned(values, "protocol_version", protocolVersion);
        malformed |= !tryParseString(values, "transport_kind", report.transportKind);
        malformed |= !tryParseString(values, "connect_target_name", report.connectTargetName);
        malformed |= !tryParseString(values, "connect_lane_mode", report.connectLaneMode);
        malformed |= !tryParseString(values, "build_configuration", report.buildConfiguration);
        malformed |= !tryParseString(values, "build_timestamp", report.buildTimestamp);
        malformed |= !tryParseString(values, "build_identity", report.buildIdentity);
        malformed |= !tryParseString(values, "build_channel", report.buildChannel);
        malformed |= !tryParseString(values, "host_instance_id", report.hostInstanceId);
        malformed |= !tryParseString(values, "session_id", report.sessionId);
        malformed |= !tryParseString(values, "runtime_root_path", report.runtimeRootPath);
        malformed |= !tryParseString(values, "restore_state", report.restoreState);
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

        report.protocolVersion = static_cast<uint32_t>(protocolVersion);
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
        report.persistenceRestoreVisible = persistenceLastLoadSucceeded || persistenceLoadCount > 0 || report.restoreState == "restored";

        report.statusParseValid = !malformed
            && persistenceParseValid
            && statusVersion >= 5
            && report.hostTickMilliseconds > 0
            && report.protocolVersion == kCurrentProtocolVersion;

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

        const std::string buildConfiguration = buildConfigurationText();
        const std::string buildTimestamp = buildTimestampText();
        const std::string buildIdentity = buildConfiguration + "|" + buildTimestamp;
        const std::string buildChannel = environmentText(L"WAR_BUILD_CHANNEL").empty()
            ? std::string("internal-alpha")
            : environmentText(L"WAR_BUILD_CHANNEL");
        const std::string connectTargetName = environmentText(L"WAR_CONNECT_TARGET_NAME").empty()
            ? std::string("localhost-fallback")
            : environmentText(L"WAR_CONNECT_TARGET_NAME");
        const std::string transportKind = environmentText(L"WAR_CONNECT_TRANSPORT").empty()
            ? std::string("file-backed-localhost-fallback")
            : environmentText(L"WAR_CONNECT_TRANSPORT");
        const std::string connectLaneMode = environmentText(L"WAR_CONNECT_LANE_MODE").empty()
            ? std::string("localhost-fallback")
            : environmentText(L"WAR_CONNECT_LANE_MODE");
        const std::string hostInstanceId = stableHostInstanceId(processId);
        const std::string sessionId = stableSessionId(processId);
        const std::string restoreState = simulationDiagnostics.lastPersistenceLoadSucceeded
            ? std::string("restored")
            : (simulationDiagnostics.persistenceLoadCount > 0 ? std::string("restore-failed") : std::string("cold-start"));

        std::ofstream output(tempPath, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            return;
        }

        output
            << "status_version=5\n"
            << "protocol_version=" << kCurrentProtocolVersion << "\n"
            << "transport_kind=" << transportKind << "\n"
            << "connect_target_name=" << connectTargetName << "\n"
            << "connect_lane_mode=" << connectLaneMode << "\n"
            << "build_configuration=" << buildConfiguration << "\n"
            << "build_timestamp=" << buildTimestamp << "\n"
            << "build_identity=" << buildIdentity << "\n"
            << "build_channel=" << buildChannel << "\n"
            << "host_instance_id=" << hostInstanceId << "\n"
            << "session_id=" << sessionId << "\n"
            << "runtime_root_path=" << runtimeBoundaryReport.runtimeRoot.generic_string() << "\n"
            << "restore_state=" << restoreState << "\n"
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
        return currentSystemEpochMilliseconds();
    }
}
