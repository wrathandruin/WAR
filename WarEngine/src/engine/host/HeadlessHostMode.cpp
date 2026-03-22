#include "engine/host/HeadlessHostMode.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <exception>
#include <fstream>
#include <optional>
#include <string>
#include <thread>

#include <windows.h>

#include "engine/core/FailureBundleProtocol.h"
#include "engine/core/FailureBundleWriter.h"
#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimeOwnership.h"
#include "engine/host/AuthoritativeHostProtocol.h"
#include "engine/host/HeadlessHostPresence.h"
#include "engine/host/ReplicationHarness.h"
#include "engine/host/SessionEntryProtocol.h"
#include "engine/simulation/SimulationRuntime.h"

namespace war
{
    namespace
    {
        constexpr uint32_t kCurrentPersistenceSchemaVersion = 7u;

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
        try
        {
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostMode::run entered");

            FailureBundleProtocolReport failureBundleProtocolReport = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(failureBundleProtocolReport);

            const RuntimeOwnershipReport runtimeOwnershipReport = RuntimeOwnership::analyze(runtimeBoundaryReport);
            if (!runtimeOwnershipReport.ownershipValid)
            {
                const std::string failureReason =
                    std::string("Headless host runtime ownership invalid: ")
                    + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport);

                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "headless_host_trace.txt",
                    std::string("HeadlessHostMode fail-fast ownership: ")
                        + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport));

                std::string bundleDirectory;
                std::string bundleError;
                FailureBundleWriter::capture(
                    runtimeBoundaryReport,
                    FailureBundleKind::Runtime,
                    "headless-host",
                    "runtime",
                    "runtime-ownership-invalid",
                    failureReason,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    5,
                    FailureBundleWriter::headlessHostFailureAttachments(runtimeBoundaryReport),
                    bundleDirectory,
                    bundleError);

                return 5;
            }

            SessionEntryProtocolReport sessionEntryProtocolReport = SessionEntryProtocol::buildReport(runtimeBoundaryReport);
            SessionEntryProtocol::ensureDirectories(sessionEntryProtocolReport);
            if (!sessionEntryProtocolReport.issues.empty() || !sessionEntryProtocolReport.sessionEntryLaneReady)
            {
                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "headless_host_trace.txt",
                    "HeadlessHostMode fail-fast session-entry-lane");

                std::string bundleDirectory;
                std::string bundleError;
                FailureBundleWriter::capture(
                    runtimeBoundaryReport,
                    FailureBundleKind::Runtime,
                    "headless-host",
                    "runtime",
                    "session-entry-lane-invalid",
                    "Headless host session-entry lane is not ready.",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    6,
                    FailureBundleWriter::headlessHostFailureAttachments(runtimeBoundaryReport),
                    bundleDirectory,
                    bundleError);

                return 6;
            }

            SimulationRuntime simulationRuntime{};
            simulationRuntime.initializeForLocalAuthority();
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostMode initialized simulation");
            simulationRuntime.setAuthorityMode(false, true, false);
            simulationRuntime.setPersistenceState(true, runtimeOwnershipReport.primarySaveSlotName);

            const std::filesystem::path savePath = runtimeOwnershipReport.primarySavePath;
            std::string persistenceError;
            const AuthoritativeWorldSnapshot persistedSnapshot =
                AuthoritativeHostProtocol::readAuthoritativeSnapshotFromPath(savePath, persistenceError);
            if (persistenceError.empty() && persistedSnapshot.valid)
            {
                std::string loadError;
                if (simulationRuntime.loadPersistedState(persistedSnapshot, loadError))
                {
                    simulationRuntime.notePersistenceLoad(
                        persistedSnapshot.persistenceLoadedSchemaVersion,
                        persistedSnapshot.persistenceMigratedFromSchemaVersion,
                        ReplicationHarness::currentEpochMilliseconds());
                }
                else
                {
                    simulationRuntime.notePersistenceFailure(loadError, true);
                }
            }

            AuthoritativeHostProtocolReport protocolReport = AuthoritativeHostProtocol::buildReport(runtimeBoundaryReport);
            AuthoritativeHostProtocol::ensureDirectories(protocolReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostMode protocol directories ready");

            ReplicationHarnessConfig harnessConfig = ReplicationHarness::loadConfig(runtimeBoundaryReport);
            std::string harnessError;
            (void)ReplicationHarness::saveConfig(runtimeBoundaryReport, harnessConfig, harnessError);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostMode harness config loaded");

            const std::filesystem::path logPath = runtimeBoundaryReport.logsDirectory / "headless_host_log.txt";
            const uint32_t processId = GetCurrentProcessId();

            appendLogLine(logPath, "WARServer Headless Authoritative Host");
            appendLogLine(logPath, std::string("Build identity: ") + localDemoDiagnosticsReport.buildIdentity);
            appendLogLine(logPath, std::string("Build channel: ") + localDemoDiagnosticsReport.buildChannel);
            appendLogLine(logPath, std::string("Environment: ") + localDemoDiagnosticsReport.environmentName);
            appendLogLine(logPath, std::string("Environment profile: ") + localDemoDiagnosticsReport.environmentProfileName);
            appendLogLine(logPath, std::string("Connect target: ") + localDemoDiagnosticsReport.connectTargetName);
            appendLogLine(logPath, std::string("Transport: ") + localDemoDiagnosticsReport.connectTransport);
            appendLogLine(logPath, std::string("Lane mode: ") + localDemoDiagnosticsReport.connectLaneMode);
            appendLogLine(logPath, std::string("Runtime root: ") + RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot));
            appendLogLine(logPath, std::string("Save slot: ") + runtimeOwnershipReport.primarySaveSlotName);
            appendLogLine(logPath, std::string("Save path: ") + RuntimePaths::displayPath(savePath));
            appendLogLine(logPath, std::string("Runtime ownership: ") + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport));
            appendLogLine(logPath, std::string("Session entry root: ") + RuntimePaths::displayPath(sessionEntryProtocolReport.sessionEntryRootDirectory));
            appendLogLine(logPath, std::string("Failure bundle root: ") + RuntimePaths::displayPath(runtimeBoundaryReport.failureBundleRootDirectory));
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostMode initial log lines written");

            std::deque<PendingIntentArrival> pendingIntentArrivals{};
            std::deque<PendingAcknowledgementDelivery> pendingAcknowledgementDeliveries{};
            std::optional<PendingSnapshotDelivery> pendingSnapshotDelivery{};

            const auto tickDuration = std::chrono::milliseconds(options.tickMilliseconds);
            const auto heartbeatDuration = std::chrono::milliseconds(options.heartbeatMilliseconds);
            const auto autosaveDuration = std::chrono::seconds(3);
            const auto startedAt = std::chrono::steady_clock::now();
            auto nextTick = startedAt;
            auto nextHeartbeat = startedAt;
            auto nextAutosave = startedAt + autosaveDuration;

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

                const std::vector<SessionEntryRequest> entryRequests =
                    SessionEntryProtocol::collectPendingEntryRequestsForHost(runtimeBoundaryReport);
                for (const SessionEntryRequest& entryRequest : entryRequests)
                {
                    const SessionEntryValidationResult validationResult =
                        SessionEntryProtocol::validateRequest(
                            runtimeBoundaryReport,
                            entryRequest,
                            localDemoDiagnosticsReport.buildIdentity,
                            localDemoDiagnosticsReport.environmentName,
                            localDemoDiagnosticsReport.connectTargetName,
                            nowEpochMilliseconds);

                    if (validationResult.accepted)
                    {
                        const SessionTicket ticket =
                            SessionEntryProtocol::buildIssuedTicket(
                                entryRequest,
                                localDemoDiagnosticsReport.buildIdentity,
                                nowEpochMilliseconds);
                        std::string ticketError;
                        if (SessionEntryProtocol::writeIssuedTicket(runtimeBoundaryReport, ticket, ticketError))
                        {
                            const ActiveSessionRecord activeSession =
                                SessionEntryProtocol::buildActiveSessionRecord(ticket, nowEpochMilliseconds);
                            std::string activeSessionError;
                            if (!SessionEntryProtocol::writeActiveSession(runtimeBoundaryReport, activeSession, activeSessionError))
                            {
                                appendLogLine(logPath, std::string("Session entry active-session write failed: ") + activeSessionError);
                                LocalDemoDiagnostics::appendTraceLine(
                                    runtimeBoundaryReport,
                                    "headless_host_trace.txt",
                                    std::string("Session entry active-session write failed: ") + activeSessionError);
                            }
                            else
                            {
                                appendLogLine(
                                    logPath,
                                    std::string("Session entry accepted: request=")
                                        + entryRequest.requestId
                                        + " ticket="
                                        + ticket.ticketId
                                        + " session="
                                        + ticket.grantedSessionId
                                        + " account="
                                        + entryRequest.accountId);
                                LocalDemoDiagnostics::appendTraceLine(
                                    runtimeBoundaryReport,
                                    "headless_host_trace.txt",
                                    std::string("Session entry accepted: ") + ticket.ticketId);
                            }
                        }
                        else
                        {
                            appendLogLine(logPath, std::string("Session entry issued-ticket write failed: ") + ticketError);
                            LocalDemoDiagnostics::appendTraceLine(
                                runtimeBoundaryReport,
                                "headless_host_trace.txt",
                                std::string("Session entry issued-ticket write failed: ") + ticketError);
                        }
                    }
                    else
                    {
                        const SessionTicket deniedTicket =
                            SessionEntryProtocol::buildDeniedTicket(
                                entryRequest,
                                localDemoDiagnosticsReport.buildIdentity,
                                nowEpochMilliseconds,
                                validationResult.denialReason);
                        std::string deniedTicketError;
                        if (SessionEntryProtocol::writeDeniedTicket(runtimeBoundaryReport, deniedTicket, deniedTicketError))
                        {
                            appendLogLine(
                                logPath,
                                std::string("Session entry denied: request=")
                                    + entryRequest.requestId
                                    + " reason="
                                    + validationResult.denialReason
                                    + " account="
                                    + entryRequest.accountId);
                            LocalDemoDiagnostics::appendTraceLine(
                                runtimeBoundaryReport,
                                "headless_host_trace.txt",
                                std::string("Session entry denied: ") + validationResult.denialReason);
                        }
                        else
                        {
                            appendLogLine(logPath, std::string("Session entry denied-ticket write failed: ") + deniedTicketError);
                            LocalDemoDiagnostics::appendTraceLine(
                                runtimeBoundaryReport,
                                "headless_host_trace.txt",
                                std::string("Session entry denied-ticket write failed: ") + deniedTicketError);
                        }
                    }
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
                    (void)AuthoritativeHostProtocol::writeAcknowledgement(runtimeBoundaryReport, acknowledgement, ackError);
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
                    (void)AuthoritativeHostProtocol::writeAuthoritativeSnapshot(runtimeBoundaryReport, pendingSnapshotDelivery->snapshot, snapshotError);
                    pendingSnapshotDelivery.reset();
                }

                if (nowSteady >= nextAutosave)
                {
                    AuthoritativeWorldSnapshot saveSnapshot =
                        simulationRuntime.buildAuthoritativeSnapshot(simulationRuntime.diagnostics().lastIntentSequence);
                    saveSnapshot.publishedEpochMilliseconds = nowEpochMilliseconds;
                    saveSnapshot.persistenceSchemaVersion = kCurrentPersistenceSchemaVersion;
                    saveSnapshot.playerActorState.lastSaveEpochMilliseconds = nowEpochMilliseconds;
                    saveSnapshot.persistenceEpochMilliseconds = nowEpochMilliseconds;

                    std::string saveError;
                    if (AuthoritativeHostProtocol::writeAuthoritativeSnapshotToPath(savePath, saveSnapshot, saveError))
                    {
                        simulationRuntime.notePersistenceSave(kCurrentPersistenceSchemaVersion, nowEpochMilliseconds);
                    }
                    else
                    {
                        simulationRuntime.notePersistenceFailure(saveError, false);
                    }
                    nextAutosave = nowSteady + autosaveDuration;
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

            return 0;
        }
        catch (const std::exception& exception)
        {
            LocalDemoDiagnostics::appendTraceLine(
                runtimeBoundaryReport,
                "headless_host_trace.txt",
                std::string("HeadlessHostMode exception: ") + exception.what());

            std::string bundleDirectory;
            std::string bundleError;
            FailureBundleWriter::capture(
                runtimeBoundaryReport,
                FailureBundleKind::Runtime,
                "headless-host",
                "runtime",
                "headless-host-exception",
                exception.what(),
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                4,
                FailureBundleWriter::headlessHostFailureAttachments(runtimeBoundaryReport),
                bundleDirectory,
                bundleError);

            return 4;
        }
        catch (...)
        {
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostMode unknown exception");

            std::string bundleDirectory;
            std::string bundleError;
            FailureBundleWriter::capture(
                runtimeBoundaryReport,
                FailureBundleKind::Runtime,
                "headless-host",
                "runtime",
                "headless-host-unknown-exception",
                "Unknown headless-host runtime exception.",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                4,
                FailureBundleWriter::headlessHostFailureAttachments(runtimeBoundaryReport),
                bundleDirectory,
                bundleError);

            return 4;
        }
    }
}
