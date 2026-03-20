#include "engine/host/HeadlessHostBootstrap.h"

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/host/HeadlessHostMode.h"

namespace war
{
    int runHeadlessHostFromCommandLine(const std::wstring& commandLine)
    {
        RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
        RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);

        LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
        LocalDemoDiagnostics::writeStartupReport(runtimeBoundaryReport, localDemoDiagnosticsReport);

        const HeadlessHostOptions options = HeadlessHostMode::parseOptions(commandLine);
        return HeadlessHostMode::run(runtimeBoundaryReport, localDemoDiagnosticsReport, options);
    }
}
