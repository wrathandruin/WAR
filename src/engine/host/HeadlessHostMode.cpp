#include "engine/host/HeadlessHostMode.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <filesystem>
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
        simulationRuntime.appendEvent("Milestone 38 host runtime active");
        simulationRuntime.appendEvent("Actor runtime / inventory / equipment / loot active on authoritative host");
        simulationRuntime.appendEvent("Persistence schema / migration / autosave active for authoritative world state");

        const std::filesystem::path savePath = runtimeBoundaryReport.savesDirectory / "authoritative_world_primary.txt";
        const std::filesystem::path logPath = runtimeBoundaryReport.logsDirectory / "headless_host_log.txt";
        const uint32_t processId = GetCurrentProcessId();

        if (std::filesystem::exists(savePath))
        {
            std::string loadError;
            const AuthoritativeWorldSnapshot loadedSnapshot = AuthoritativeHostProtocol::readSnapshotFile(savePath, loadError);
            if (loadError.empty())
            {
                std::string loadMessage;
                if (simulationRuntime.applyPersistedSnapshot(loadedSnapshot, loadMessage))
                {
                    simulationRuntime.appendEvent(loadMessage);
                    appendLogLine(logPath, loadMessage);
                }
            }
            else
            {
                simulationRuntime.notePersistenceLoad(0u, 0u, false, loadError);
                simulationRuntime.appendEvent(std::string("Persistence load failed: ") + loadError);
                appendLogLine(logPath, std::string("Persistence load failed: ") + loadError);
            }
        }
        else
        {
            simulationRuntime.notePersistenceLoad(3u, 0u, true, "");
            simulationRuntime.appendEvent("Persistence: no prior authoritative save found; starting fresh.");
        }

        AuthoritativeHostProtocolReport protocolReport = AuthoritativeHostProtocol::buildReport(runtimeBoundaryReport);
        AuthoritativeHostProtocol::ensureDirectories(protocolReport);

        ReplicationHarnessConfig harnessConfig = ReplicationHarness::loadConfig(runtimeBoundaryReport);
        std::string harnessError;
        (void)ReplicationHarness::saveConfig(runtimeBoundaryReport, harnessConfig, harnessError);

        appendLogLine(logPath, "WARServer M38 Authoritative Host");
        appendLogLine(logPath, std::string("PID: ") + std::to_string(processId));
        appendLogLine(logPath, std::string("Tick ms: ") + std::to_string(options.tickMilliseconds));
        appendLogLine(logPath, std::string("Runtime root: ") + RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot));
        appendLogLine(logPath, std::string("Startup report: ") + RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath));
        appendLogLine(logPath, std::string("Intent queue: ") + protocolReport.intentQueueDirectory.generic_string());
        appendLogLine(logPath, std::string("Ack queue: ") + protocolReport.acknowledgementQueueDirectory.generic_string());
        appendLogLine(logPath, std::string("Snapshot path: ") + protocolReport.snapshotPath.generic_string());
        appendLogLine(logPath, std::string("Persistence save path: ") + savePath.generic_string());
        appendLogLine(logPath, std::string("Harness enabled: ") + (harnessConfig.enabled ? "yes" : "no"));
        appendLogLine(logPath, "State: running");

        std::deque<PendingIntentArrival> pendingIntentArrivals{};
        std::deque<PendingAcknowledgementDelivery> pendingAcknowledgementDeliveries{};
        std::optional<PendingSnapshotDelivery> pendingSnapshotDelivery{};

        const auto tickDuration = std::chrono::milliseconds(options.tickMilliseconds);
        const auto heartbeatDuration = std::chrono::milliseconds(options.heartbeatMilliseconds);
        const auto startedAt = std::chrono::steady_clock::now();
        auto nextTick = startedAt;
        auto nextHeartbeat = startedAt;
        uint64_t lastAutosaveTicks = simulationRuntime.diagnostics().simulationTicks;

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

            if (simulationRuntime.diagnostics().simulationTicks >= lastAutosaveTicks + 40u)
            {
                AuthoritativeWorldSnapshot saveSnapshot =
                    simulationRuntime.buildAuthoritativeSnapshot(simulationRuntime.diagnostics().lastIntentSequence);
                saveSnapshot.publishedEpochMilliseconds = nowEpochMilliseconds;
                std::string saveError;
                const bool saved = AuthoritativeHostProtocol::writeSnapshotFile(savePath, saveSnapshot, saveError);
                simulationRuntime.notePersistenceSave(saveSnapshot.schemaVersion, saved, saveError, nowEpochMilliseconds);
                if (!saved)
                {
                    appendLogLine(logPath, std::string("Autosave failed: ") + saveError);
                }
                lastAutosaveTicks = simulationRuntime.diagnostics().simulationTicks;
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

        const uint64_t shutdownEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        AuthoritativeWorldSnapshot finalSnapshot =
            simulationRuntime.buildAuthoritativeSnapshot(simulationRuntime.diagnostics().lastIntentSequence);
        finalSnapshot.publishedEpochMilliseconds = shutdownEpochMilliseconds;

        std::string snapshotError;
        (void)AuthoritativeHostProtocol::writeAuthoritativeSnapshot(runtimeBoundaryReport, finalSnapshot, snapshotError);

        std::string saveError;
        const bool finalSaveSucceeded = AuthoritativeHostProtocol::writeSnapshotFile(savePath, finalSnapshot, saveError);
        simulationRuntime.notePersistenceSave(finalSnapshot.schemaVersion, finalSaveSucceeded, saveError, shutdownEpochMilliseconds);
        if (!finalSaveSucceeded)
        {
            appendLogLine(logPath, std::string("Final save failed: ") + saveError);
        }

        appendLogLine(logPath, std::string("Simulation ticks: ") + std::to_string(simulationRuntime.diagnostics().simulationTicks));
        appendLogLine(logPath, std::string("Last processed intent: ") + std::to_string(simulationRuntime.diagnostics().lastIntentSequence));
        appendLogLine(logPath, std::string("Persistence saves: ") + std::to_string(simulationRuntime.diagnostics().persistenceSaveCount));
        appendLogLine(logPath, std::string("Loot collections: ") + std::to_string(simulationRuntime.diagnostics().lootCollections));
        appendLogLine(logPath, "State: stopped");
        return 0;
    }
}
