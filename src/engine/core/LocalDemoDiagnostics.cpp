#include "engine/core/LocalDemoDiagnostics.h"

#include <fstream>
#include <string_view>
#include <system_error>

#include <windows.h>

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

        std::string narrowText(const std::wstring& value)
        {
            if (value.empty())
            {
                return {};
            }

            const int requiredBytes = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.c_str(),
                static_cast<int>(value.size()),
                nullptr,
                0,
                nullptr,
                nullptr);
            if (requiredBytes <= 0)
            {
                return {};
            }

            std::string result(static_cast<size_t>(requiredBytes), '\0');
            const int writtenBytes = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.c_str(),
                static_cast<int>(value.size()),
                result.data(),
                requiredBytes,
                nullptr,
                nullptr);
            if (writtenBytes <= 0)
            {
                return {};
            }

            result.resize(static_cast<size_t>(writtenBytes));
            return result;
        }

        std::string environmentText(const wchar_t* variableName)
        {
            return narrowText(readEnvironmentWide(variableName));
        }

        std::string defaultTransport(bool runtimeRootOverrideActive)
        {
            return runtimeRootOverrideActive
                ? std::string("file-backed-hosted-bootstrap")
                : std::string("file-backed-localhost-fallback");
        }

        std::string defaultLaneMode(bool runtimeRootOverrideActive)
        {
            return runtimeRootOverrideActive
                ? std::string("hosted-bootstrap")
                : std::string("localhost-fallback");
        }
    }

    LocalDemoDiagnosticsReport LocalDemoDiagnostics::buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        LocalDemoDiagnosticsReport report{};
        report.buildConfiguration = buildConfigurationText();
        report.buildTimestamp = buildTimestampText();
        report.buildIdentity = report.buildConfiguration + "|" + report.buildTimestamp;
        report.buildChannel = environmentText(L"WAR_BUILD_CHANNEL");
        if (report.buildChannel.empty())
        {
            report.buildChannel = "internal-alpha";
        }

        report.runtimeRootOverrideActive = runtimeBoundaryReport.runtimeRootOverrideActive;
        report.connectTargetName = environmentText(L"WAR_CONNECT_TARGET_NAME");
        if (report.connectTargetName.empty())
        {
            report.connectTargetName = report.runtimeRootOverrideActive ? "internal-alpha-lan" : "localhost-fallback";
        }

        report.connectTransport = environmentText(L"WAR_CONNECT_TRANSPORT");
        if (report.connectTransport.empty())
        {
            report.connectTransport = defaultTransport(report.runtimeRootOverrideActive);
        }

        report.connectLaneMode = environmentText(L"WAR_CONNECT_LANE_MODE");
        if (report.connectLaneMode.empty())
        {
            report.connectLaneMode = defaultLaneMode(report.runtimeRootOverrideActive);
        }

        report.runtimeRootDisplay = RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot);
        report.startupReportPath = runtimeBoundaryReport.logsDirectory / "local_demo_startup_report.txt";
        report.suggestedPackageRoot = runtimeBoundaryReport.repoRootResolved
            ? runtimeBoundaryReport.repoRoot / "out" / "internal_alpha"
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
            && directoryExists(packagedRuntime / "CrashDumps")
            && directoryExists(packagedRuntime / "Host");

        report.launchScriptPresent =
            fileExists(executableDirectory / "launch_headless_host_win64.bat")
            || fileExists(executableDirectory / "launch_local_client_against_host_win64.bat")
            || fileExists(executableDirectory / "scripts" / "launch_headless_host_win64.bat")
            || fileExists(executableDirectory / "scripts" / "launch_local_client_against_host_win64.bat");

        report.smokeScriptPresent =
            fileExists(executableDirectory / "smoke_test_headless_host_win64.bat")
            || fileExists(executableDirectory / "smoke_test_local_demo_win64.bat")
            || fileExists(executableDirectory / "scripts" / "smoke_test_headless_host_win64.bat")
            || fileExists(executableDirectory / "scripts" / "smoke_test_local_demo_win64.bat");

        report.m45ValidationScriptPresent =
            fileExists(executableDirectory / "validate_m45_hosted_bootstrap_win64.bat")
            || fileExists(executableDirectory / "validate_m45_internal_alpha_package_win64.bat")
            || fileExists(executableDirectory / "scripts" / "validate_m45_hosted_bootstrap_win64.bat")
            || fileExists(executableDirectory / "scripts" / "validate_m45_internal_alpha_package_win64.bat");

        report.repoPackagingScriptsReady = runtimeBoundaryReport.repoRootResolved
            && fileExists(repoScripts / "build_internal_alpha_package_win64.bat")
            && fileExists(repoScripts / "launch_headless_host_win64.bat")
            && fileExists(repoScripts / "launch_local_client_against_host_win64.bat")
            && fileExists(repoScripts / "smoke_test_headless_host_win64.bat")
            && fileExists(repoScripts / "smoke_test_local_demo_win64.bat")
            && fileExists(repoScripts / "validate_m45_hosted_bootstrap_win64.bat")
            && fileExists(repoScripts / "validate_m45_internal_alpha_package_win64.bat");

        report.packagedLaneReady =
            report.packageAssetsReady
            && report.packageRuntimeReady
            && report.launchScriptPresent
            && report.smokeScriptPresent
            && report.m45ValidationScriptPresent;

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
            return;
        }

        std::ofstream output(localDemoDiagnosticsReport.startupReportPath, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            return;
        }

        output
            << "WAR Internal Alpha Startup Report\n"
            << "Milestone: M45 - Internal Alpha Package / Hosted Deploy / Telemetry Baseline\n"
            << "Build configuration: " << localDemoDiagnosticsReport.buildConfiguration << "\n"
            << "Build timestamp: " << localDemoDiagnosticsReport.buildTimestamp << "\n"
            << "Build identity: " << localDemoDiagnosticsReport.buildIdentity << "\n"
            << "Build channel: " << localDemoDiagnosticsReport.buildChannel << "\n"
            << "Connect target: " << localDemoDiagnosticsReport.connectTargetName << "\n"
            << "Connect transport: " << localDemoDiagnosticsReport.connectTransport << "\n"
            << "Connect lane mode: " << localDemoDiagnosticsReport.connectLaneMode << "\n"
            << "Runtime root override active: " << (localDemoDiagnosticsReport.runtimeRootOverrideActive ? "yes" : "no") << "\n"
            << "Runtime mode: " << (runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged") << "\n"
            << "Executable path: " << RuntimePaths::displayPath(runtimeBoundaryReport.executablePath) << "\n"
            << "Executable directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.executableDirectory) << "\n"
            << "Repo root: " << RuntimePaths::displayPath(runtimeBoundaryReport.repoRoot) << "\n"
            << "Asset root: " << RuntimePaths::displayPath(runtimeBoundaryReport.assetRoot) << "\n"
            << "Runtime root: " << RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot) << "\n"
            << "Packaged lane ready: " << (localDemoDiagnosticsReport.packagedLaneReady ? "yes" : "no") << "\n";

        output.flush();
        localDemoDiagnosticsReport.startupReportWritten = output.good();
    }

    void LocalDemoDiagnostics::appendTraceLine(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const std::string& filename,
        std::string_view line)
    {
        const std::filesystem::path tracePath = runtimeBoundaryReport.logsDirectory / filename;
        std::error_code error;
        std::filesystem::create_directories(tracePath.parent_path(), error);
        if (error)
        {
            return;
        }

        std::ofstream output(tracePath, std::ios::out | std::ios::app);
        if (!output.is_open())
        {
            return;
        }

        output << line << "\n";
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
