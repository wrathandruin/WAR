#include "engine/core/Application.h"

#include <exception>
#include <filesystem>
#include <string>

#include <windows.h>

#include "engine/core/BetaRetentionProtocol.h"
#include "engine/core/EnvironmentConfig.h"
#include "engine/core/FailureBundleProtocol.h"
#include "engine/core/FailureBundleWriter.h"
#include "engine/core/IncidentResponseProtocol.h"
#include "engine/core/LauncherDistributionProtocol.h"
#include "engine/core/LiveOpsProtocol.h"
#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/MarketOnboardingProtocol.h"
#include "engine/core/ReleaseManagementProtocol.h"
#include "engine/core/ReleaseCandidateProtocol.h"
#include "engine/core/RuntimeOwnership.h"
#include "engine/core/RuntimePaths.h"
#include "engine/core/SupportWorkflowProtocol.h"
#include "engine/core/Timer.h"
#include "engine/host/SessionEntryProtocol.h"
#include "game/GameLayer.h"
#include "platform/win32/Win32Window.h"

namespace war
{
    namespace
    {
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

        std::wstring windowTitleText()
        {
            const std::wstring connectTargetName = readEnvironmentWide(L"WAR_CONNECT_TARGET_NAME");
            if (connectTargetName.empty())
            {
                return L"WAR";
            }

            return std::wstring(L"WAR [") + connectTargetName + L"]";
        }
    }

    int Application::run(const std::wstring& commandLine)
    {
        try
        {
            (void)commandLine;

            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run entered");

            LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
            const EnvironmentConfigReport environmentConfigReport = EnvironmentConfig::load(runtimeBoundaryReport);
            const RuntimeOwnershipReport runtimeOwnershipReport = RuntimeOwnership::analyze(runtimeBoundaryReport);
            const SessionEntryProtocolReport sessionEntryProtocolReport = SessionEntryProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocolReport failureBundleProtocolReport = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(failureBundleProtocolReport);

            const bool returningPlayerDetected = BetaRetentionProtocol::hasPersistedResumeIdentity(runtimeBoundaryReport);
            BetaRetentionProtocol::recordLaunch(
                runtimeBoundaryReport,
                "client",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                returningPlayerDetected);

            ReleaseCandidateProtocol::recordCandidateState(
                runtimeBoundaryReport,
                "client",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            LauncherDistributionProtocol::recordLauncherEntry(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                returningPlayerDetected);

            const MarketOnboardingReport marketOnboardingReport =
                MarketOnboardingProtocol::recordSession(
                    runtimeBoundaryReport,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    returningPlayerDetected);

            const LiveOpsReport liveOpsReport =
                LiveOpsProtocol::recordClientLaunch(
                    runtimeBoundaryReport,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "client-startup",
                    returningPlayerDetected,
                    marketOnboardingReport.firstSessionDetected);

            const ReleaseManagementReport releaseManagementReport =
                ReleaseManagementProtocol::recordReleaseState(
                    runtimeBoundaryReport,
                    "client",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName);

            const SupportWorkflowReport supportWorkflowReport =
                SupportWorkflowProtocol::recordClientSession(
                    runtimeBoundaryReport,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "client-startup",
                    returningPlayerDetected,
                    marketOnboardingReport.firstSessionDetected);

            const IncidentResponseReport incidentResponseReport =
                IncidentResponseProtocol::recordIncidentState(
                    runtimeBoundaryReport,
                    "client",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "nominal");

            LiveOpsProtocol::noteReleaseStateWrite(runtimeBoundaryReport);

            LocalDemoDiagnostics::writeStartupReport(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport,
                &environmentConfigReport,
                &runtimeOwnershipReport,
                &sessionEntryProtocolReport,
                &failureBundleProtocolReport,
                &marketOnboardingReport,
                &liveOpsReport,
                &releaseManagementReport,
                &supportWorkflowReport,
                &incidentResponseReport);

            if (!environmentConfigReport.configurationValid)
            {
                const std::string failureReason =
                    std::string("Client startup configuration invalid: ")
                    + EnvironmentConfig::diagnosticsSummary(environmentConfigReport);

                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "client_runtime_trace.txt",
                    std::string("Application::run fail-fast config: ")
                        + EnvironmentConfig::diagnosticsSummary(environmentConfigReport));

                (void)IncidentResponseProtocol::recordIncidentState(
                    runtimeBoundaryReport,
                    "client",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "config-invalid");

                std::string bundleDirectory;
                std::string bundleError;
                FailureBundleWriter::capture(
                    runtimeBoundaryReport,
                    FailureBundleKind::Startup,
                    "client",
                    "startup",
                    "config-invalid",
                    failureReason,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    -4,
                    FailureBundleWriter::clientFailureAttachments(runtimeBoundaryReport),
                    bundleDirectory,
                    bundleError);

                return -4;
            }

            if (!runtimeOwnershipReport.ownershipValid)
            {
                const std::string failureReason =
                    std::string("Client startup runtime ownership invalid: ")
                    + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport);

                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "client_runtime_trace.txt",
                    std::string("Application::run fail-fast ownership: ")
                        + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport));

                (void)IncidentResponseProtocol::recordIncidentState(
                    runtimeBoundaryReport,
                    "client",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "runtime-ownership-invalid");

                std::string bundleDirectory;
                std::string bundleError;
                FailureBundleWriter::capture(
                    runtimeBoundaryReport,
                    FailureBundleKind::Startup,
                    "client",
                    "startup",
                    "runtime-ownership-invalid",
                    failureReason,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    -5,
                    FailureBundleWriter::clientFailureAttachments(runtimeBoundaryReport),
                    bundleDirectory,
                    bundleError);

                return -5;
            }

            Win32Window window;
            if (!window.create(1600, 900, windowTitleText().c_str()))
            {
                LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run window.create failed");

                (void)IncidentResponseProtocol::recordIncidentState(
                    runtimeBoundaryReport,
                    "client",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "window-create-failed");

                std::string bundleDirectory;
                std::string bundleError;
                FailureBundleWriter::capture(
                    runtimeBoundaryReport,
                    FailureBundleKind::Startup,
                    "client",
                    "startup",
                    "window-create-failed",
                    "Client window creation failed.",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    -1,
                    FailureBundleWriter::clientFailureAttachments(runtimeBoundaryReport),
                    bundleDirectory,
                    bundleError);

                return -1;
            }

            GameLayer game;
            game.initialize(window);

            Timer timer;
            timer.reset();

            while (!window.shouldClose())
            {
                window.pollEvents();
                const float dt = timer.tick();
                game.update(dt);
                game.render();
            }

            game.shutdown();
            return 0;
        }
        catch (const std::exception& exception)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);

            LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
            FailureBundleProtocolReport failureBundleProtocolReport = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(failureBundleProtocolReport);

            const bool returningPlayerDetected = BetaRetentionProtocol::hasPersistedResumeIdentity(runtimeBoundaryReport);
            BetaRetentionProtocol::recordLaunch(
                runtimeBoundaryReport,
                "client-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                returningPlayerDetected);

            ReleaseCandidateProtocol::recordCandidateState(
                runtimeBoundaryReport,
                "client-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            LauncherDistributionProtocol::recordLauncherEntry(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                returningPlayerDetected);

            const MarketOnboardingReport marketOnboardingReport =
                MarketOnboardingProtocol::recordSession(
                    runtimeBoundaryReport,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    returningPlayerDetected);

            (void)LiveOpsProtocol::recordClientLaunch(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "client-exception",
                returningPlayerDetected,
                marketOnboardingReport.firstSessionDetected);

            (void)ReleaseManagementProtocol::recordReleaseState(
                runtimeBoundaryReport,
                "client-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)SupportWorkflowProtocol::recordClientSession(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "client-exception",
                returningPlayerDetected,
                marketOnboardingReport.firstSessionDetected);

            (void)IncidentResponseProtocol::recordIncidentState(
                runtimeBoundaryReport,
                "client-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "exception");

            LiveOpsProtocol::noteReleaseStateWrite(runtimeBoundaryReport);

            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", std::string("Application::run exception: ") + exception.what());

            std::string bundleDirectory;
            std::string bundleError;
            FailureBundleWriter::capture(
                runtimeBoundaryReport,
                FailureBundleKind::Runtime,
                "client",
                "runtime",
                "client-exception",
                exception.what(),
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                -2,
                FailureBundleWriter::clientFailureAttachments(runtimeBoundaryReport),
                bundleDirectory,
                bundleError);

            return -2;
        }
        catch (...)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);

            LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
            FailureBundleProtocolReport failureBundleProtocolReport = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(failureBundleProtocolReport);

            const bool returningPlayerDetected = BetaRetentionProtocol::hasPersistedResumeIdentity(runtimeBoundaryReport);
            BetaRetentionProtocol::recordLaunch(
                runtimeBoundaryReport,
                "client-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                returningPlayerDetected);

            ReleaseCandidateProtocol::recordCandidateState(
                runtimeBoundaryReport,
                "client-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            LauncherDistributionProtocol::recordLauncherEntry(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                returningPlayerDetected);

            const MarketOnboardingReport marketOnboardingReport =
                MarketOnboardingProtocol::recordSession(
                    runtimeBoundaryReport,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    returningPlayerDetected);

            (void)LiveOpsProtocol::recordClientLaunch(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "client-unknown-exception",
                returningPlayerDetected,
                marketOnboardingReport.firstSessionDetected);

            (void)ReleaseManagementProtocol::recordReleaseState(
                runtimeBoundaryReport,
                "client-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)SupportWorkflowProtocol::recordClientSession(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "client-unknown-exception",
                returningPlayerDetected,
                marketOnboardingReport.firstSessionDetected);

            (void)IncidentResponseProtocol::recordIncidentState(
                runtimeBoundaryReport,
                "client-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "unknown-exception");

            LiveOpsProtocol::noteReleaseStateWrite(runtimeBoundaryReport);

            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run unknown exception");

            std::string bundleDirectory;
            std::string bundleError;
            FailureBundleWriter::capture(
                runtimeBoundaryReport,
                FailureBundleKind::Runtime,
                "client",
                "runtime",
                "client-unknown-exception",
                "Unknown client runtime exception.",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                -3,
                FailureBundleWriter::clientFailureAttachments(runtimeBoundaryReport),
                bundleDirectory,
                bundleError);

            return -3;
        }
    }
}
