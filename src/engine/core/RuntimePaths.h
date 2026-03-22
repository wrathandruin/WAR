#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace war
{
    struct RuntimeBoundaryReport
    {
        bool runningFromSourceTree = false;
        bool repoRootResolved = false;
        bool assetRootResolved = false;
        bool runtimeDirectoriesReady = false;
        bool runtimeRootOverrideActive = false;
        bool environmentRootResolved = false;
        bool environmentProfileResolved = false;

        std::string environmentName = "local";
        std::string environmentProfileName = "local";

        std::filesystem::path executablePath;
        std::filesystem::path executableDirectory;
        std::filesystem::path repoRoot;
        std::filesystem::path assetRoot;
        std::filesystem::path runtimeRoot;
        std::filesystem::path configDirectory;
        std::filesystem::path logsDirectory;
        std::filesystem::path savesDirectory;
        std::filesystem::path crashDirectory;
        std::filesystem::path hostDirectory;
        std::filesystem::path failureBundleRootDirectory;
        std::filesystem::path startupFailureBundleDirectory;
        std::filesystem::path runtimeFailureBundleDirectory;
        std::filesystem::path bootstrapFailureBundleDirectory;
        std::filesystem::path operatorTriageDirectory;
        std::filesystem::path environmentRoot;
        std::filesystem::path environmentProfilesDirectory;
        std::filesystem::path environmentProfileDirectory;
        std::filesystem::path environmentProfileFile;

        std::vector<std::string> issues;
    };

    class RuntimePaths
    {
    public:
        [[nodiscard]] static RuntimeBoundaryReport buildReport();
        static void ensureRuntimeDirectories(RuntimeBoundaryReport& report);
        [[nodiscard]] static std::string displayPath(const std::filesystem::path& path);

    private:
        [[nodiscard]] static std::filesystem::path resolveExecutablePath();
        [[nodiscard]] static std::filesystem::path findRepoRoot(const std::filesystem::path& startDirectory);
        [[nodiscard]] static bool hasRepoMarkers(const std::filesystem::path& candidate);
    };
}
