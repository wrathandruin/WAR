#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "engine/core/RuntimePaths.h"

namespace war
{
    struct LocalDemoDiagnosticsReport
    {
        std::string buildConfiguration;
        std::string buildTimestamp;
        std::string buildIdentity = "unknown";
        std::string buildChannel = "internal-alpha";
        std::string connectTargetName = "localhost-fallback";
        std::string connectTransport = "file-backed-localhost-fallback";
        std::string connectLaneMode = "localhost-fallback";
        std::string runtimeRootDisplay = "unresolved";

        bool repoPackagingScriptsReady = false;
        bool packageAssetsReady = false;
        bool packageRuntimeReady = false;
        bool launchScriptPresent = false;
        bool smokeScriptPresent = false;
        bool m45ValidationScriptPresent = false;
        bool packagedLaneReady = false;
        bool startupReportWritten = false;
        bool runtimeRootOverrideActive = false;

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
        static void appendTraceLine(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& filename,
            std::string_view line);

    private:
        [[nodiscard]] static std::string buildConfigurationText();
        [[nodiscard]] static std::string buildTimestampText();
    };
}
