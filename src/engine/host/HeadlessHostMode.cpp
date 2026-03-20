#include "engine/host/HeadlessHostMode.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <optional>
#include <string>
#include <thread>

#include <windows.h>

#include "engine/host/AuthoritativeHostProtocol.h"
#include "engine/host/HeadlessHostPresence.h"
#include "engine/host/ReplicationHarness.h"
#include "engine/simulation/SimulationRuntime.h"

namespace war
{
    namespace
    {
        struct PendingIntentArrival
        {
            SimulationIntent intent{};
            uint64_t dueEpochMilliseconds = 0;
        };

        struct PendingAcknowledgementDelivery
        {
            SimulationIntentAck acknowledgement{};
            uint64_t dueEpochMilliseconds = 0;
        };

        struct PendingSnapshotDelivery
        {
            AuthoritativeWorldSnapshot snapshot{};
            uint64_t dueEpochMilliseconds = 0;
        };

        constexpr uint64_t kPersistenceSchemaVersion = 2;
        constexpr uint64_t kAutoSaveIntervalMilliseconds = 5000ull;

        uint32_t parseOptionValue(const std::wstring& commandLine, const std::wstring& key, uint32_t fallback)
        {
            const size_t start = commandLine.find(key);
            if (start == std::wstring::npos)
            {
                return fallback;
            }

            size_t valueStart = start + key.size();
            size_t valueEnd = valueStart;
            while (valueEnd < commandLine.size() && commandLine[valueEnd] != L' ' && commandLine[valueEnd] != L'\t')
            {
                ++valueEnd;
            }

            try
            {
                return static_cast<uint32_t>(std::stoul(commandLine.substr(valueStart, valueEnd - valueStart)));
            }
            catch (...)
            {
                return fallback;
            }
        }

        void appendLogLine(const std::filesystem::path& path, const std::string& line)
        {
            std::ofstream output(path, std::ios::out | std::ios::app);
            if (!output.is_open())
            {
                return;
            }

            output << line << "\n";
        }

        uint32_t effectiveLatency(bool enabled, uint32_t value)
        {
            return enabled ? value : 0u;
        }
    }

    HeadlessHostOptions HeadlessHostMode::parseOptions(const std::wstring& commandLine)
    {
        HeadlessHostOptions options{};
        options.tickMilliseconds = std::max<uint32_t>(1u, parseOptionValue(commandLine, L"--host-tick-ms=", 50u));
        options.heartbeatMilliseconds = std::max<uint32_t>(50u, parseOptionValue(commandLine, L"--host-heartbeat-ms=", 500u));
        options.runSeconds = parseOptionValue(commandLine, L"--host-run-seconds=", 0u);
        return options;
    }

    int HeadlessHostMode::run(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
        const HeadlessHostOptions& options)
    {
        SimulationRuntime simulationRuntime{};
        simulationRuntime.initializeForLocalAuthority();
        simulationRuntime.setAuthorityMode(false, true, false);
        simulationRuntime.setPersistenceState(true, "primary");
        simulationRuntime.appendEvent("Milestone 37 authoritative persistence lane active");
        simulationRuntime.appendEvent("Versioned persistence schema and migration-safe load/save flow active");
        simulationRuntime.appendEvent("Client movement and interaction requests continue to resolve here");

        AuthoritativeHostProtocolReport protocolReport = AuthoritativeHostProtocol::buildReport(runtimeBoundaryReport);
        AuthoritativeHostProtocol::ensureDirectories(protocolReport);

        ReplicationHarnessConfig harnessConfig = ReplicationHarness::loadConfig(runtimeBoundaryReport);
        std::string harnessError;
        (void)ReplicationHarness::saveConfig(runtimeBoundaryReport, harnessConfig, harnessError);

        const std::filesystem::path logPath = runtimeBoundaryReport.logsDirectory / "headless_host_log.txt";
        const uint32_t processId = GetCurrentProcessId();

        appendLogLine(logPath, "WAR Headless Authoritative Host");
        appendLogLine(logPath, std::string("PID: ") + std::to_string(processId));
        appendLogLine(logPath, std::string("Tick ms: ") + std::to_string(options.tickMilliseconds));
        appendLogLine(logPath, std::string("Runtime root: ") + RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot));
        appendLogLine(logPath, std::string("Startup report: ") + RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath));
        appendLogLine(logPath, std::string("Intent queue: ") + protocolReport.intentQueueDirectory.generic_string());
        appendLogLine(logPath, std::string("Ack queue: ") + protocolReport.acknowledgementQueueDirectory.generic_string());
        appendLogLine(logPath, std::string("Snapshot path: ") + protocolReport.snapshotPath.generic_string());
        appendLogLine(logPath, std::string("Persistent save path: ") + protocolReport.persistentSavePath.generic_string());
        appendLogLine(logPath, std::string("Harness enabled: ") + (harnessConfig.enabled ? "yes" : "no"));
        appendLogLine(logPath, "State: running");

        const uint64_t startupEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        uint32_t loadedSchemaVersion = 0;
        uint32_t migratedFromSchemaVersion = 0;
        std::string persistenceLoadError;
        const AuthoritativeWorldSnapshot persistedSnapshot =
            AuthoritativeHostProtocol::readPersistentWorldSave(
                runtimeBoundaryReport,
                loadedSchemaVersion,
                migratedFromSchemaVersion,
                persistenceLoadError);

        if (persistenceLoadError.empty() && persistedSnapshot.valid)
        {
            simulationRuntime.applyPersistedSnapshot(
                persistedSnapshot,
                loadedSchemaVersion,
                migratedFromSchemaVersion,
                startupEpochMilliseconds);

            appendLogLine(
                logPath,
                std::string("Loaded authoritative persistence save. schema=")
                + std::to_string(loadedSchemaVersion)
                + " migrated_from="
                + std::to_string(migratedFromSchemaVersion));
            simulationRuntime.appendEvent(
                std::string("Authoritative save loaded. schema=")
                + std::to_string(loadedSchemaVersion)
                + (migratedFromSchemaVersion > 0
                    ? std::string(" migrated-from=") + std::to_string(migratedFromSchemaVersion)
                    : std::string{}));
        }
        else if (persistenceLoadError == "Persistent save not present.")
        {
            appendLogLine(logPath, "No authoritative persistent save present. Starting fresh world.");
            simulationRuntime.appendEvent("No authoritative persistent save present. Starting fresh world.");
        }
        else
        {
            simulationRuntime.notePersistenceFailure(persistenceLoadError, true);
            appendLogLine(logPath, std::string("Persistence load failed: ") + persistenceLoadError);
            simulationRuntime.appendEvent(std::string("Persistence load failed: ") + persistenceLoadError);
        }

        auto persistAuthoritativeState = [&](const std::string& reason, uint64_t epochMilliseconds)
        {
            AuthoritativeWorldSnapshot saveSnapshot =
                simulationRuntime.buildAuthoritativeSnapshot(simulationRuntime.diagnostics().lastIntentSequence);
            saveSnapshot.persistenceSchemaVersion = static_cast<uint32_t>(kPersistenceSchemaVersion);
            saveSnapshot.persistenceMigratedFromSchemaVersion = 0;
            saveSnapshot.persistenceEpochMilliseconds = epochMilliseconds;
            saveSnapshot.persistenceSlotName = simulationRuntime.diagnostics().persistenceSlotName;

            std::string persistenceSaveError;
            if (AuthoritativeHostProtocol::writePersistentWorldSave(runtimeBoundaryReport, saveSnapshot, persistenceSaveError))
            {
                simulationRuntime.notePersistenceSave(static_cast<uint32_t>(kPersistenceSchemaVersion), epochMilliseconds);
                appendLogLine(
                    logPath,
                    std::string("Persistence save published [") + reason + "] epoch_ms=" + std::to_string(epochMilliseconds));
                return true;
            }

            simulationRuntime.notePersistenceFailure(persistenceSaveError, false);
            appendLogLine(
                logPath,
                std::string("Persistence save failed [") + reason + "]: " + persistenceSaveError);
            simulationRuntime.appendEvent(
                std::string("Persistence save failed [") + reason + "]: " + persistenceSaveError);
            return false;
        };

        if (migratedFromSchemaVersion > 0)
        {
            (void)persistAuthoritativeState("post-migration", startupEpochMilliseconds);
        }

        std::deque<PendingIntentArrival> pendingIntentArrivals{};
        std::deque<PendingAcknowledgementDelivery> pendingAcknowledgementDeliveries{};
        std::optional<PendingSnapshotDelivery> pendingSnapshotDelivery{};

        const auto tickDuration = std::chrono::milliseconds(options.tickMilliseconds);
        const auto heartbeatDuration = std::chrono::milliseconds(options.heartbeatMilliseconds);
        const auto startedAt = std::chrono::steady_clock::now();
        auto nextTick = startedAt;
        auto nextHeartbeat = startedAt;
        uint64_t lastPersistenceSaveEpochMilliseconds = simulationRuntime.diagnostics().lastPersistenceSaveEpochMilliseconds;

        for (;;)
        {
            const auto nowSteady = std::chrono::steady_clock::now();
            const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
            harnessConfig = ReplicationHarness::loadConfig(runtimeBoundaryReport);

            if (options.runSeconds > 0)
            {
                const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(nowSteady - startedAt).count();
                if (elapsedSeconds >= static_cast<long long>(options.runSeconds))
                {
                    break;
                }
            }

            if (nowSteady < nextTick)
            {
                std::this_thread::sleep_until(nextTick);
                continue;
            }

            const std::vector<SimulationIntent> incomingRequests =
                AuthoritativeHostProtocol::collectPendingIntentRequestsForHost(runtimeBoundaryReport);
            for (const SimulationIntent& intent : incomingRequests)
            {
                PendingIntentArrival arrival{};
                arrival.intent = intent;
                arrival.dueEpochMilliseconds = ReplicationHarness::computeArrivalEpochMilliseconds(
                    nowEpochMilliseconds,
                    intent.sequence,
                    effectiveLatency(harnessConfig.enabled, harnessConfig.intentLatencyMilliseconds),
                    effectiveLatency(harnessConfig.enabled, harnessConfig.jitterMilliseconds));
                pendingIntentArrivals.push_back(arrival);
            }

            while (!pendingIntentArrivals.empty() && pendingIntentArrivals.front().dueEpochMilliseconds <= nowEpochMilliseconds)
            {
                const SimulationIntent intent = pendingIntentArrivals.front().intent;
                pendingIntentArrivals.pop_front();

                SimulationIntentAck acknowledgement = simulationRuntime.submitAuthoritativeIntent(intent);
                acknowledgement.publishedEpochMilliseconds = nowEpochMilliseconds;

                PendingAcknowledgementDelivery delivery{};
                delivery.acknowledgement = acknowledgement;
                delivery.dueEpochMilliseconds = ReplicationHarness::computeArrivalEpochMilliseconds(
                    nowEpochMilliseconds,
                    acknowledgement.sequence,
                    effectiveLatency(harnessConfig.enabled, harnessConfig.acknowledgementLatencyMilliseconds),
                    effectiveLatency(harnessConfig.enabled, harnessConfig.jitterMilliseconds));
                pendingAcknowledgementDeliveries.push_back(delivery);
            }

            simulationRuntime.advanceFrame(static_cast<float>(options.tickMilliseconds) / 1000.0f);
            nextTick += tickDuration;

            while (!pendingAcknowledgementDeliveries.empty()
                && pendingAcknowledgementDeliveries.front().dueEpochMilliseconds <= nowEpochMilliseconds)
            {
                const SimulationIntentAck acknowledgement = pendingAcknowledgementDeliveries.front().acknowledgement;
                pendingAcknowledgementDeliveries.pop_front();

                std::string ackError;
                if (!AuthoritativeHostProtocol::writeAcknowledgement(runtimeBoundaryReport, acknowledgement, ackError))
                {
                    appendLogLine(logPath, std::string("Ack write failed: ") + ackError);
                }
            }

            if (!pendingSnapshotDelivery.has_value())
            {
                PendingSnapshotDelivery delivery{};
                delivery.snapshot = simulationRuntime.buildAuthoritativeSnapshot(simulationRuntime.diagnostics().lastIntentSequence);
                delivery.snapshot.publishedEpochMilliseconds = nowEpochMilliseconds;
                delivery.dueEpochMilliseconds = ReplicationHarness::computeArrivalEpochMilliseconds(
                    nowEpochMilliseconds,
                    delivery.snapshot.simulationTicks + delivery.snapshot.lastProcessedIntentSequence,
                    effectiveLatency(harnessConfig.enabled, harnessConfig.snapshotLatencyMilliseconds),
                    effectiveLatency(harnessConfig.enabled, harnessConfig.jitterMilliseconds));
                pendingSnapshotDelivery = delivery;
            }

            if (pendingSnapshotDelivery.has_value() && pendingSnapshotDelivery->dueEpochMilliseconds <= nowEpochMilliseconds)
            {
                std::string snapshotError;
                if (!AuthoritativeHostProtocol::writeAuthoritativeSnapshot(
                        runtimeBoundaryReport,
                        pendingSnapshotDelivery->snapshot,
                        snapshotError))
                {
                    appendLogLine(logPath, std::string("Snapshot write failed: ") + snapshotError);
                }
                pendingSnapshotDelivery.reset();
            }

            if (lastPersistenceSaveEpochMilliseconds == 0
                || nowEpochMilliseconds - lastPersistenceSaveEpochMilliseconds >= kAutoSaveIntervalMilliseconds)
            {
                if (persistAuthoritativeState("autosave", nowEpochMilliseconds))
                {
                    lastPersistenceSaveEpochMilliseconds = nowEpochMilliseconds;
                }
            }

            if (nowSteady >= nextHeartbeat)
            {
                HeadlessHostPresence::writeStatus(
                    runtimeBoundaryReport,
                    simulationRuntime.diagnostics(),
                    options.tickMilliseconds,
                    processId,
                    "running",
                    harnessConfig,
                    pendingIntentArrivals.size(),
                    pendingAcknowledgementDeliveries.size(),
                    pendingSnapshotDelivery.has_value() ? 1u : 0u);
                nextHeartbeat = nowSteady + heartbeatDuration;
            }
        }

        const uint64_t shutdownEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        (void)persistAuthoritativeState("shutdown", shutdownEpochMilliseconds);

        HeadlessHostPresence::writeStatus(
            runtimeBoundaryReport,
            simulationRuntime.diagnostics(),
            options.tickMilliseconds,
            processId,
            "stopped",
            harnessConfig,
            0u,
            0u,
            0u);

        AuthoritativeWorldSnapshot finalSnapshot =
            simulationRuntime.buildAuthoritativeSnapshot(simulationRuntime.diagnostics().lastIntentSequence);
        finalSnapshot.publishedEpochMilliseconds = shutdownEpochMilliseconds;
        std::string snapshotError;
        (void)AuthoritativeHostProtocol::writeAuthoritativeSnapshot(runtimeBoundaryReport, finalSnapshot, snapshotError);

        appendLogLine(logPath, std::string("Simulation ticks: ") + std::to_string(simulationRuntime.diagnostics().simulationTicks));
        appendLogLine(logPath, std::string("Last processed intent: ") + std::to_string(simulationRuntime.diagnostics().lastIntentSequence));
        appendLogLine(logPath, std::string("Divergence corrections observed: ") + std::to_string(simulationRuntime.diagnostics().correctionsApplied));
        appendLogLine(logPath, std::string("Persistence saves: ") + std::to_string(simulationRuntime.diagnostics().persistenceSaveCount));
        appendLogLine(logPath, std::string("Persistence loads: ") + std::to_string(simulationRuntime.diagnostics().persistenceLoadCount));
        appendLogLine(logPath, "State: stopped");
        return 0;
    }
}
