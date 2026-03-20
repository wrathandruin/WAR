#include "engine/core/RuntimePaths.h"

#include <system_error>
#include <vector>

#include <windows.h>

namespace war
{
    namespace
    {
        std::filesystem::path modulePath()
        {
            std::vector<wchar_t> buffer(MAX_PATH, L'\0');

            for (;;)
            {
                const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
                if (length == 0)
                {
                    return {};
                }

                if (length < static_cast<DWORD>(buffer.size() - 1u))
                {
                    return std::filesystem::path(std::wstring(buffer.data(), length));
                }

                buffer.resize(buffer.size() * 2u, L'\0');
            }
        }

        bool directoryExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_directory(path, error);
        }

        void ensureDirectory(const std::filesystem::path& path, RuntimeBoundaryReport& report)
        {
            std::error_code error;
            if (!std::filesystem::exists(path, error))
            {
                std::filesystem::create_directories(path, error);
            }

            if (error)
            {
                report.issues.push_back(std::string("Failed to create runtime directory: ") + RuntimePaths::displayPath(path));
            }
        }
    }

    RuntimeBoundaryReport RuntimePaths::buildReport()
    {
        RuntimeBoundaryReport report{};
        report.executablePath = resolveExecutablePath();
        report.executableDirectory = report.executablePath.empty()
            ? std::filesystem::current_path()
            : report.executablePath.parent_path();

        report.repoRoot = findRepoRoot(report.executableDirectory);
        report.repoRootResolved = !report.repoRoot.empty();
        report.runningFromSourceTree = report.repoRootResolved;

        const std::filesystem::path packagedAssetRoot = report.executableDirectory / "assets";
        const std::filesystem::path sourceAssetRoot = report.repoRootResolved
            ? report.repoRoot / "assets"
            : std::filesystem::path{};

        if (directoryExists(packagedAssetRoot))
        {
            report.assetRoot = packagedAssetRoot;
            report.assetRootResolved = true;
        }
        else if (report.repoRootResolved && directoryExists(sourceAssetRoot))
        {
            report.assetRoot = sourceAssetRoot;
            report.assetRootResolved = true;
        }
        else
        {
            report.issues.push_back("Asset root could not be resolved from packaged or source-tree layout.");
        }

        report.runtimeRoot = report.repoRootResolved
            ? report.repoRoot / "Runtime"
            : report.executableDirectory / "runtime";
        report.configDirectory = report.runtimeRoot / "Config";
        report.logsDirectory = report.runtimeRoot / "Logs";
        report.savesDirectory = report.runtimeRoot / "Saves";
        report.crashDirectory = report.runtimeRoot / "CrashDumps";
        report.hostDirectory = report.runtimeRoot / "Host";

        if (report.assetRootResolved)
        {
            const std::filesystem::path shaders = report.assetRoot / "shaders" / "dx11";
            const std::filesystem::path textures = report.assetRoot / "textures";

            if (!directoryExists(shaders))
            {
                report.issues.push_back("Shader asset directory missing: assets/shaders/dx11");
            }

            if (!directoryExists(textures))
            {
                report.issues.push_back("Texture asset directory missing: assets/textures");
            }
        }

        return report;
    }

    void RuntimePaths::ensureRuntimeDirectories(RuntimeBoundaryReport& report)
    {
        ensureDirectory(report.runtimeRoot, report);
        ensureDirectory(report.configDirectory, report);
        ensureDirectory(report.logsDirectory, report);
        ensureDirectory(report.savesDirectory, report);
        ensureDirectory(report.crashDirectory, report);
        ensureDirectory(report.hostDirectory, report);

        report.runtimeDirectoriesReady = true;
    }

    std::string RuntimePaths::displayPath(const std::filesystem::path& path)
    {
        if (path.empty())
        {
            return "unresolved";
        }

        return path.generic_string();
    }

    std::filesystem::path RuntimePaths::resolveExecutablePath()
    {
        return modulePath();
    }

    std::filesystem::path RuntimePaths::findRepoRoot(const std::filesystem::path& startDirectory)
    {
        if (startDirectory.empty())
        {
            return {};
        }

        std::filesystem::path current = startDirectory;
        for (;;)
        {
            if (hasRepoMarkers(current))
            {
                return current;
            }

            const std::filesystem::path parent = current.parent_path();
            if (parent.empty() || parent == current)
            {
                break;
            }

            current = parent;
        }

        return {};
    }

    bool RuntimePaths::hasRepoMarkers(const std::filesystem::path& candidate)
    {
        return directoryExists(candidate / "src")
            && directoryExists(candidate / "assets")
            && directoryExists(candidate / "Docs")
            && std::filesystem::exists(candidate / "README.md");
    }
}
