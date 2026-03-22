#include "engine/host/HeadlessHostBootstrap.h"

#include <exception>
#include <string>

#include "engine/core/EnvironmentConfig.h"
#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimeOwnership.h"
#include "engine/core/RuntimePaths.h"
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

            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap diagnostics report built");
            LocalDemoDiagnostics::writeStartupReport(
                runtimeBoundaryReport,
                localDemoDiagnosticsReport,
                &environmentConfigReport,
                &runtimeOwnershipReport,
                &sessionEntryProtocolReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap startup report written");

            if (!environmentConfigReport.configurationValid)
            {
                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "headless_host_trace.txt",
                    std::string("HeadlessHostBootstrap fail-fast config: ")
                        + EnvironmentConfig::diagnosticsSummary(environmentConfigReport));
                return 4;
            }

            if (!runtimeOwnershipReport.ownershipValid)
            {
                LocalDemoDiagnostics::appendTraceLine(
                    runtimeBoundaryReport,
                    "headless_host_trace.txt",
                    std::string("HeadlessHostBootstrap fail-fast ownership: ")
                        + RuntimeOwnership::diagnosticsSummary(runtimeOwnershipReport));
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
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", std::string("HeadlessHostBootstrap exception: ") + exception.what());
            return 2;
        }
        catch (...)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "headless_host_trace.txt", "HeadlessHostBootstrap unknown exception");
            return 3;
        }
    }
}
