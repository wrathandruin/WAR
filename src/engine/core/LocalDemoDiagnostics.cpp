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

        std::string joinKeys(const std::vector<std::string>& values)
        {
            std::string result;
            for (size_t index = 0; index < values.size(); ++index)
            {
                if (index > 0)
                {
                    result += ", ";
                }
                result += values[index];
            }
            return result.empty() ? std::string("none") : result;
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
        report.environmentRootResolved = runtimeBoundaryReport.environmentRootResolved;
        report.environmentProfileResolved = runtimeBoundaryReport.environmentProfileResolved;
        report.environmentName = runtimeBoundaryReport.environmentName;
        report.environmentProfileName = runtimeBoundaryReport.environmentProfileName;
        report.deployableEnvironmentRoot = runtimeBoundaryReport.environmentRoot;
        report.deployableEnvironmentProfileDirectory = runtimeBoundaryReport.environmentProfileDirectory;
        report.deployableEnvironmentProfileFile = runtimeBoundaryReport.environmentProfileFile;

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
            && report.m45ValidationScriptPresent
            && report.environmentRootResolved
            && report.environmentProfileResolved;

        return report;
    }

    void LocalDemoDiagnostics::writeStartupReport(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
        const EnvironmentConfigReport* environmentConfigReport,
        const RuntimeOwnershipReport* runtimeOwnershipReport,
        const SessionEntryProtocolReport* sessionEntryProtocolReport)
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
            << "WAR Startup Report\n"
            << "Milestone: M47 - Account Session Ticket Handoff / Authenticated Entry\n"
            << "Build configuration: " << localDemoDiagnosticsReport.buildConfiguration << "\n"
            << "Build timestamp: " << localDemoDiagnosticsReport.buildTimestamp << "\n"
            << "Build identity: " << localDemoDiagnosticsReport.buildIdentity << "\n"
            << "Build channel: " << localDemoDiagnosticsReport.buildChannel << "\n"
            << "Environment: " << localDemoDiagnosticsReport.environmentName << "\n"
            << "Environment profile: " << localDemoDiagnosticsReport.environmentProfileName << "\n"
            << "Deployable environment root: " << RuntimePaths::displayPath(localDemoDiagnosticsReport.deployableEnvironmentRoot) << "\n"
            << "Deployable environment profile directory: " << RuntimePaths::displayPath(localDemoDiagnosticsReport.deployableEnvironmentProfileDirectory) << "\n"
            << "Deployable environment profile file: " << RuntimePaths::displayPath(localDemoDiagnosticsReport.deployableEnvironmentProfileFile) << "\n"
            << "Mutable runtime config directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.configDirectory) << "\n"
            << "Mutable runtime logs directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.logsDirectory) << "\n"
            << "Mutable runtime saves directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.savesDirectory) << "\n"
            << "Mutable runtime host directory: " << RuntimePaths::displayPath(runtimeBoundaryReport.hostDirectory) << "\n"
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
            << "Environment root resolved: " << (localDemoDiagnosticsReport.environmentRootResolved ? "yes" : "no") << "\n"
            << "Environment profile resolved: " << (localDemoDiagnosticsReport.environmentProfileResolved ? "yes" : "no") << "\n"
            << "Packaged lane ready: " << (localDemoDiagnosticsReport.packagedLaneReady ? "yes" : "no") << "\n";

        if (environmentConfigReport != nullptr)
        {
            output
                << "Config identity: " << environmentConfigReport->configIdentity << "\n"
                << "Config description: " << environmentConfigReport->description << "\n"
                << "Config connect target policy: " << environmentConfigReport->connectTargetPolicy << "\n"
                << "Config transport policy: " << environmentConfigReport->transportPolicy << "\n"
                << "Config runtime root policy: " << environmentConfigReport->runtimeRootPolicy << "\n"
                << "Secrets required: " << (environmentConfigReport->secretsRequired ? "yes" : "no") << "\n"
                << "Secrets source kind: " << environmentConfigReport->secretsSourceKind << "\n"
                << "Secrets file path: " << RuntimePaths::displayPath(environmentConfigReport->secretsFilePath) << "\n"
                << "Required secret count: " << environmentConfigReport->requiredSecretCount << "\n"
                << "Loaded secret count: " << environmentConfigReport->loadedSecretCount << "\n"
                << "Missing required secret count: " << environmentConfigReport->missingRequiredSecretCount << "\n"
                << "Missing required secret keys: " << joinKeys(environmentConfigReport->missingRequiredSecretKeys) << "\n"
                << "Configuration valid: " << (environmentConfigReport->configurationValid ? "yes" : "no") << "\n";
        }

        if (runtimeOwnershipReport != nullptr)
        {
            output
                << "Requested save slot: " << runtimeOwnershipReport->requestedSaveSlotName << "\n"
                << "Sanitized primary save slot: " << runtimeOwnershipReport->primarySaveSlotName << "\n"
                << "Primary save path: " << RuntimePaths::displayPath(runtimeOwnershipReport->primarySavePath) << "\n"
                << "Runtime owned directories sane: " << (runtimeOwnershipReport->runtimeOwnedDirectoriesSane ? "yes" : "no") << "\n"
                << "Deployable environment separated: " << (runtimeOwnershipReport->deployableEnvironmentSeparated ? "yes" : "no") << "\n"
                << "Primary save path owned: " << (runtimeOwnershipReport->primarySavePathOwned ? "yes" : "no") << "\n"
                << "Runtime ownership valid: " << (runtimeOwnershipReport->ownershipValid ? "yes" : "no") << "\n";
        }

        if (sessionEntryProtocolReport != nullptr)
        {
            output
                << "Session entry lane ready: " << (sessionEntryProtocolReport->sessionEntryLaneReady ? "yes" : "no") << "\n"
                << "Session entry root directory: " << RuntimePaths::displayPath(sessionEntryProtocolReport->sessionEntryRootDirectory) << "\n"
                << "Session entry request queue: " << RuntimePaths::displayPath(sessionEntryProtocolReport->requestQueueDirectory) << "\n"
                << "Session entry issued ticket directory: " << RuntimePaths::displayPath(sessionEntryProtocolReport->issuedTicketDirectory) << "\n"
                << "Session entry denied ticket directory: " << RuntimePaths::displayPath(sessionEntryProtocolReport->deniedTicketDirectory) << "\n"
                << "Session entry active session directory: " << RuntimePaths::displayPath(sessionEntryProtocolReport->activeSessionDirectory) << "\n";
        }

        output
            << "Trust boundary summary: Environment/ is deployable config; Runtime/Config is mutable runtime config; Runtime/Saves, Runtime/Logs, Runtime/Host are runtime-owned.\n";

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
