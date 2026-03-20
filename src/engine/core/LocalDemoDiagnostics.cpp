#include "engine/core/LocalDemoDiagnostics.h"

#include <fstream>
#include <system_error>

namespace war
{
    namespace
    {
        bool directoryExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_directory(path, error);
        }

        bool fileExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_regular_file(path, error);
        }
    }

    LocalDemoDiagnosticsReport LocalDemoDiagnostics::buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        LocalDemoDiagnosticsReport report{};
        report.buildConfiguration = buildConfigurationText();
        report.buildTimestamp = buildTimestampText();
        report.startupReportPath = runtimeBoundaryReport.logsDirectory / "local_demo_startup_report.txt";
        report.suggestedPackageRoot = runtimeBoundaryReport.repoRootResolved
            ? runtimeBoundaryReport.repoRoot / "out" / "local_demo"
            : runtimeBoundaryReport.executableDirectory;

        const std::filesystem::path executableDirectory = runtimeBoundaryReport.executableDirectory;
        const std::filesystem::path packagedAssets = executableDirectory / "assets";
        const std::filesystem::path packagedRuntime = executableDirectory / "runtime";
        const std::filesystem::path repoScripts = runtimeBoundaryReport.repoRoot / "scripts";

        report.packageAssetsReady =
            directoryExists(packagedAssets / "shaders" / "dx11")
            && directoryExists(packagedAssets / "textures");

        report.packageRuntimeReady =
            directoryExists(packagedRuntime / "Config")
            && directoryExists(packagedRuntime / "Logs")
            && directoryExists(packagedRuntime / "Saves")
            && directoryExists(packagedRuntime / "CrashDumps");

        report.launchScriptPresent =
            fileExists(executableDirectory / "launch_local_demo_win64.bat")
            || fileExists(executableDirectory / "scripts" / "launch_local_demo_win64.bat");

        report.smokeScriptPresent =
            fileExists(executableDirectory / "smoke_test_local_demo_win64.bat")
            || fileExists(executableDirectory / "scripts" / "smoke_test_local_demo_win64.bat");

        report.repoPackagingScriptsReady = runtimeBoundaryReport.repoRootResolved
            && fileExists(repoScripts / "build_local_demo_package_win64.bat")
            && fileExists(repoScripts / "launch_local_demo_win64.bat")
            && fileExists(repoScripts / "smoke_test_local_demo_win64.bat");

        report.packagedLaneReady =
            report.packageAssetsReady
            && report.packageRuntimeReady
            && report.launchScriptPresent
            && report.smokeScriptPresent;

        if (runtimeBoundaryReport.runningFromSourceTree)
        {
            if (!report.repoPackagingScriptsReady)
            {
                report.issues.push_back("Local demo packaging scripts are missing from repo/scripts.");
            }
        }
        else
        {
            if (!report.packageAssetsReady)
            {
                report.issues.push_back("Packaged demo asset layout is incomplete next to the executable.");
            }

            if (!report.packageRuntimeReady)
            {
                report.issues.push_back("Packaged demo runtime layout is incomplete next to the executable.");
            }

            if (!report.launchScriptPresent)
            {
                report.issues.push_back("Packaged demo launch script is missing.");
            }

            if (!report.smokeScriptPresent)
            {
                report.issues.push_back("Packaged demo smoke-test script is missing.");
            }
        }

        return report;
    }

    void LocalDemoDiagnostics::writeStartupReport(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        LocalDemoDiagnosticsReport& localDemoDiagnosticsReport)
    {
        std::error_code error;
        std::filesystem::create_directories(localDemoDiagnosticsReport.startupReportPath.parent_path(), error);
        if (error)
        {
            localDemoDiagnosticsReport.issues.push_back(
                std::string("Failed to create startup report directory: ")
                + RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath.parent_path()));
            return;
        }

        std::ofstream output(localDemoDiagnosticsReport.startupReportPath, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            localDemoDiagnosticsReport.issues.push_back(
                std::string("Failed to write startup report: ")
                + RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath));
            return;
        }

        output
            << "WAR Local Demo Startup Report\n"
            << "Milestone: M32 - Local Demo Lane / Packaging / Diagnostics Baseline\n"
            << "Build configuration: " << localDemoDiagnosticsReport.buildConfiguration << "\n"
            << "Build timestamp: " << localDemoDiagnosticsReport.buildTimestamp << "\n"
            << "Runtime mode: " << (runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged") << "\n"
            << "Executable path: " << RuntimePaths::displayPath(runtimeBoundaryReport.executablePath) << "\n"
            << "Executable directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.executableDirectory) << "\n"
            << "Repo root: " << RuntimePaths::displayPath(runtimeBoundaryReport.repoRoot) << "\n"
            << "Asset root: " << RuntimePaths::displayPath(runtimeBoundaryReport.assetRoot) << "\n"
            << "Runtime root: " << RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot) << "\n"
            << "Config directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.configDirectory) << "\n"
            << "Logs directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.logsDirectory) << "\n"
            << "Saves directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.savesDirectory) << "\n"
            << "Crash directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.crashDirectory) << "\n"
            << "Suggested package root: " << RuntimePaths::displayPath(localDemoDiagnosticsReport.suggestedPackageRoot) << "\n"
            << "Repo packaging scripts ready: " << (localDemoDiagnosticsReport.repoPackagingScriptsReady ? "yes" : "no") << "\n"
            << "Packaged asset layout ready: " << (localDemoDiagnosticsReport.packageAssetsReady ? "yes" : "no") << "\n"
            << "Packaged runtime layout ready: " << (localDemoDiagnosticsReport.packageRuntimeReady ? "yes" : "no") << "\n"
            << "Launch script present: " << (localDemoDiagnosticsReport.launchScriptPresent ? "yes" : "no") << "\n"
            << "Smoke-test script present: " << (localDemoDiagnosticsReport.smokeScriptPresent ? "yes" : "no") << "\n"
            << "Packaged lane ready: " << (localDemoDiagnosticsReport.packagedLaneReady ? "yes" : "no") << "\n";

        output << "\nRuntime boundary issues:\n";
        if (runtimeBoundaryReport.issues.empty())
        {
            output << "- none\n";
        }
        else
        {
            for (const std::string& issue : runtimeBoundaryReport.issues)
            {
                output << "- " << issue << "\n";
            }
        }

        output << "\nLocal demo issues:\n";
        if (localDemoDiagnosticsReport.issues.empty())
        {
            output << "- none\n";
        }
        else
        {
            for (const std::string& issue : localDemoDiagnosticsReport.issues)
            {
                output << "- " << issue << "\n";
            }
        }

        output << "\nExpected local demo packaging scripts:\n"
            << "- scripts/build_local_demo_package_win64.bat\n"
            << "- scripts/launch_local_demo_win64.bat\n"
            << "- scripts/smoke_test_local_demo_win64.bat\n";

        output.flush();
        localDemoDiagnosticsReport.startupReportWritten = output.good();

        if (!localDemoDiagnosticsReport.startupReportWritten)
        {
            localDemoDiagnosticsReport.issues.push_back(
                std::string("Startup report flush failed: ")
                + RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath));
        }
    }

    std::string LocalDemoDiagnostics::buildConfigurationText()
    {
    #if defined(_DEBUG)
        return "Debug";
    #else
        return "Release";
    #endif
    }

    std::string LocalDemoDiagnostics::buildTimestampText()
    {
        return std::string(__DATE__) + " " + std::string(__TIME__);
    }
}
