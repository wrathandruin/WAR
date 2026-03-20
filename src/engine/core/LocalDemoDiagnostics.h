#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"

namespace war
{
    struct LocalDemoDiagnosticsReport
    {
        std::string buildConfiguration;
        std::string buildTimestamp;

        bool repoPackagingScriptsReady = false;
        bool packageAssetsReady = false;
        bool packageRuntimeReady = false;
        bool launchScriptPresent = false;
        bool smokeScriptPresent = false;
        bool packagedLaneReady = false;
        bool startupReportWritten = false;

        std::filesystem::path startupReportPath;
        std::filesystem::path suggestedPackageRoot;

        std::vector<std::string> issues;
    };

    class LocalDemoDiagnostics
    {
    public:
        [[nodiscard]] static LocalDemoDiagnosticsReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport);
        static void writeStartupReport(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            LocalDemoDiagnosticsReport& localDemoDiagnosticsReport);

    private:
        [[nodiscard]] static std::string buildConfigurationText();
        [[nodiscard]] static std::string buildTimestampText();
    };
}
