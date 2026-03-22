#include "engine/host/HeadlessHostBootstrap.h"

#include <exception>
#include <string>

#include "engine/core/BetaRetentionProtocol.h"
#include "engine/core/EnvironmentConfig.h"
#include "engine/core/FailureBundleProtocol.h"
#include "engine/core/FailureBundleWriter.h"
#include "engine/core/IncidentResponseProtocol.h"
#include "engine/core/LauncherDistributionProtocol.h"
#include "engine/core/LiveOpsProtocol.h"
#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/MarketOnboardingProtocol.h"
#include "engine/core/ReleaseCandidateProtocol.h"
#include "engine/core/ReleaseManagementProtocol.h"
#include "engine/core/RuntimeOwnership.h"
#include "engine/core/RuntimePaths.h"
#include "engine/core/SupportWorkflowProtocol.h"
#include "engine/host/HeadlessHostMode.h"
#include "engine/host/SessionEntryProtocol.h"

namespace war
{
    int runHeadlessHostFromCommandLine(const std::wstring& commandLine)
    {
        try
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap entered");

            LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
            const EnvironmentConfigReport environmentConfigReport = EnvironmentConfig::load(runtimeBoundaryReport);
            const RuntimeOwnershipReport runtimeOwnershipReport = RuntimeOwnership::analyze(runtimeBoundaryReport);

            SessionEntryProtocolReport sessionEntryProtocolReport = SessionEntryProtocol::buildReport(runtimeBoundaryReport);
            SessionEntryProtocol::ensureDirectories(sessionEntryProtocolReport);

            FailureBundleProtocolReport failureBundleProtocolReport = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(failureBundleProtocolReport);

            BetaRetentionProtocol::recordLaunch(
                runtimeBoundaryReport,
                "host-bootstrap",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                false);

            ReleaseCandidateProtocol::recordCandidateState(
                runtimeBoundaryReport,
                "host-bootstrap",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            LauncherDistributionProtocol::recordDistributionRuntime(
                runtimeBoundaryReport,
                "host-bootstrap",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            const MarketOnboardingReport marketOnboardingReport = MarketOnboardingProtocol::buildReport(runtimeBoundaryReport);

            const LiveOpsReport liveOpsReport =
                LiveOpsProtocol::recordHostBootstrap(
                    runtimeBoundaryReport,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName);

            const ReleaseManagementReport releaseManagementReport =
                ReleaseManagementProtocol::recordReleaseState(
                    runtimeBoundaryReport,
                    "host-bootstrap",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName);

            const SupportWorkflowReport supportWorkflowReport =
                SupportWorkflowProtocol::recordHostBootstrap(
                    runtimeBoundaryReport,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName);

            const IncidentResponseReport incidentResponseReport =
                IncidentResponseProtocol::recordIncidentState(
                    runtimeBoundaryReport,
                    "host-bootstrap",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "nominal");

            LiveOpsProtocol::noteReleaseStateWrite(runtimeBoundaryReport);

            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap diagnostics report built");
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
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap startup report written");

            if (!environmentConfigReport.configurationValid)
            {
                const std::string failureReason =
                    std::string("Bootstrap configuration invalid: ")
                    + EnvironmentConfig::diagnosticsSummary(environmentConfigReport);

                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "headless_host_trace.txt",
                    std::string("HeadlessHostBootstrap fail-fast config: ")
                        + EnvironmentConfig::diagnosticsSummary(environmentConfigReport));

                (void)IncidentResponseProtocol::recordIncidentState(
                    runtimeBoundaryReport,
                    "host-bootstrap",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "config-invalid");

                std::string bundleDirectory;
                std::string bundleError;
                FailureBundleWriter::capture(
                    runtimeBoundaryReport,
                    FailureBundleKind::Bootstrap,
                    "host-bootstrap",
                    "bootstrap",
                    "config-invalid",
                    failureReason,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    4,
                    FailureBundleWriter::bootstrapFailureAttachments(runtimeBoundaryReport),
                    bundleDirectory,
                    bundleError);

                return 4;
            }

            if (!runtimeOwnershipReport.ownershipValid)
            {
                const std::string failureReason =
                    std::string("Bootstrap runtime ownership invalid: ")
                    + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport);

                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "headless_host_trace.txt",
                    std::string("HeadlessHostBootstrap fail-fast ownership: ")
                        + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport));

                (void)IncidentResponseProtocol::recordIncidentState(
                    runtimeBoundaryReport,
                    "host-bootstrap",
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    "runtime-ownership-invalid");

                std::string bundleDirectory;
                std::string bundleError;
                FailureBundleWriter::capture(
                    runtimeBoundaryReport,
                    FailureBundleKind::Bootstrap,
                    "host-bootstrap",
                    "bootstrap",
                    "runtime-ownership-invalid",
                    failureReason,
                    localDemoDiagnosticsReport.buildIdentity,
                    localDemoDiagnosticsReport.buildChannel,
                    localDemoDiagnosticsReport.environmentName,
                    localDemoDiagnosticsReport.connectTargetName,
                    5,
                    FailureBundleWriter::bootstrapFailureAttachments(runtimeBoundaryReport),
                    bundleDirectory,
                    bundleError);

                return 5;
            }

            const HeadlessHostOptions options = HeadlessHostMode::parseOptions(commandLine);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap options parsed");
            return HeadlessHostMode::run(runtimeBoundaryReport, localDemoDiagnosticsReport, options);
        }
        catch (const std::exception& exception)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);

            LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
            FailureBundleProtocolReport failureBundleProtocolReport = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(failureBundleProtocolReport);

            BetaRetentionProtocol::recordLaunch(
                runtimeBoundaryReport,
                "host-bootstrap-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                false);

            ReleaseCandidateProtocol::recordCandidateState(
                runtimeBoundaryReport,
                "host-bootstrap-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            LauncherDistributionProtocol::recordDistributionRuntime(
                runtimeBoundaryReport,
                "host-bootstrap-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)LiveOpsProtocol::recordHostBootstrap(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)ReleaseManagementProtocol::recordReleaseState(
                runtimeBoundaryReport,
                "host-bootstrap-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)SupportWorkflowProtocol::recordHostBootstrap(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)IncidentResponseProtocol::recordIncidentState(
                runtimeBoundaryReport,
                "host-bootstrap-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "exception");

            LiveOpsProtocol::noteReleaseStateWrite(runtimeBoundaryReport);

            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", std::string("HeadlessHostBootstrap exception: ") + exception.what());

            std::string bundleDirectory;
            std::string bundleError;
            FailureBundleWriter::capture(
                runtimeBoundaryReport,
                FailureBundleKind::Bootstrap,
                "host-bootstrap",
                "bootstrap",
                "bootstrap-exception",
                exception.what(),
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                2,
                FailureBundleWriter::bootstrapFailureAttachments(runtimeBoundaryReport),
                bundleDirectory,
                bundleError);

            return 2;
        }
        catch (...)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);

            LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
            FailureBundleProtocolReport failureBundleProtocolReport = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(failureBundleProtocolReport);

            BetaRetentionProtocol::recordLaunch(
                runtimeBoundaryReport,
                "host-bootstrap-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                false);

            ReleaseCandidateProtocol::recordCandidateState(
                runtimeBoundaryReport,
                "host-bootstrap-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            LauncherDistributionProtocol::recordDistributionRuntime(
                runtimeBoundaryReport,
                "host-bootstrap-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)LiveOpsProtocol::recordHostBootstrap(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)ReleaseManagementProtocol::recordReleaseState(
                runtimeBoundaryReport,
                "host-bootstrap-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)SupportWorkflowProtocol::recordHostBootstrap(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName);

            (void)IncidentResponseProtocol::recordIncidentState(
                runtimeBoundaryReport,
                "host-bootstrap-unknown-exception",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                "unknown-exception");

            LiveOpsProtocol::noteReleaseStateWrite(runtimeBoundaryReport);

            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap unknown exception");

            std::string bundleDirectory;
            std::string bundleError;
            FailureBundleWriter::capture(
                runtimeBoundaryReport,
                FailureBundleKind::Bootstrap,
                "host-bootstrap",
                "bootstrap",
                "bootstrap-unknown-exception",
                "Unknown bootstrap exception.",
                localDemoDiagnosticsReport.buildIdentity,
                localDemoDiagnosticsReport.buildChannel,
                localDemoDiagnosticsReport.environmentName,
                localDemoDiagnosticsReport.connectTargetName,
                3,
                FailureBundleWriter::bootstrapFailureAttachments(runtimeBoundaryReport),
                bundleDirectory,
                bundleError);

            return 3;
        }
    }
}
