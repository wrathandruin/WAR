#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

#include <windows.h>

#include "engine/core/RuntimePaths.h"
#include "engine/core/SourceManifestLayout.h"

namespace war
{
    struct ReleaseManagementReport
    {
        bool releaseManifestPresent = false;
        bool releaseLaneReady = false;

        std::filesystem::path releaseDirectory;
        std::filesystem::path releaseManifestSourcePath;
        std::filesystem::path releaseManagementManifestPath;
        std::filesystem::path releaseGateSummaryPath;
        std::filesystem::path latestReleasePointerPath;
    };

    class ReleaseManagementProtocol
    {
    public:
        [[nodiscard]] static ReleaseManagementReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            ReleaseManagementReport report{};
            report.releaseDirectory = releaseDirectory(runtimeBoundaryReport);
            report.releaseManifestSourcePath = resolveReleaseManagementManifestPath(runtimeBoundaryReport);
            report.releaseManagementManifestPath = report.releaseDirectory / "release_management_manifest.txt";
            report.releaseGateSummaryPath = report.releaseDirectory / "release_gate_summary.txt";
            report.latestReleasePointerPath = report.releaseDirectory / "latest_release_management_pointer.txt";

            std::error_code error;
            report.releaseManifestPresent =
                std::filesystem::exists(report.releaseManifestSourcePath, error)
                && std::filesystem::is_regular_file(report.releaseManifestSourcePath, error);
            report.releaseLaneReady = report.releaseManifestPresent;
            return report;
        }

        [[nodiscard]] static ReleaseManagementReport recordReleaseState(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName)
        {
            ReleaseManagementReport report = buildReport(runtimeBoundaryReport);

            std::error_code error;
            std::filesystem::create_directories(report.releaseDirectory, error);
            if (error)
            {
                return report;
            }

            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();

            writeTextFile(
                report.releaseManagementManifestPath,
                buildReleaseManagementManifest(
                    report.releaseManifestSourcePath,
                    component,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));

            writeTextFile(
                report.releaseGateSummaryPath,
                buildReleaseGateSummary(
                    component,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));

            writeTextFile(
                report.latestReleasePointerPath,
                buildReleasePointer(
                    component,
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));

            report.releaseLaneReady = report.releaseManifestPresent;
            return report;
        }

        [[nodiscard]] static std::filesystem::path releaseDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "ReleaseManagement";
        }

        [[nodiscard]] static std::filesystem::path resolveReleaseManagementManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "ReleaseManagement",
                "m52_release_management_manifest.txt");
        }

    private:
        [[nodiscard]] static uint64_t currentEpochMilliseconds()
        {
            FILETIME fileTime{};
            GetSystemTimeAsFileTime(&fileTime);

            ULARGE_INTEGER rawTime{};
            rawTime.LowPart = fileTime.dwLowDateTime;
            rawTime.HighPart = fileTime.dwHighDateTime;

            constexpr uint64_t kWindowsEpochToUnixEpoch100ns = 116444736000000000ull;
            constexpr uint64_t kHundredNanosecondsPerMillisecond = 10000ull;

            if (rawTime.QuadPart <= kWindowsEpochToUnixEpoch100ns)
            {
                return 0ull;
            }

            return (rawTime.QuadPart - kWindowsEpochToUnixEpoch100ns) / kHundredNanosecondsPerMillisecond;
        }

        [[nodiscard]] static std::string sanitizeSingleLine(const std::string& value)
        {
            std::string sanitized = value;
            for (char& ch : sanitized)
            {
                if (ch == '\n' || ch == '\r')
                {
                    ch = ' ';
                }
                else if (ch == '|')
                {
                    ch = '/';
                }
            }
            return sanitized;
        }

        [[nodiscard]] static std::string buildReleaseManagementManifest(
            const std::filesystem::path& sourceManifestPath,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "source_manifest_path=" << sourceManifestPath.generic_string() << "\n"
                << "component=" << sanitizeSingleLine(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n"
                << "preserved_gates=M45,M46,M47,M48,M49,M50,M51\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildReleaseGateSummary(
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "WAR Release Gate Summary\n"
                << "Component: " << sanitizeSingleLine(component) << "\n"
                << "Build identity: " << sanitizeSingleLine(buildIdentity) << "\n"
                << "Build channel: " << sanitizeSingleLine(buildChannel) << "\n"
                << "Environment: " << sanitizeSingleLine(environmentName) << "\n"
                << "Connect target: " << sanitizeSingleLine(connectTargetName) << "\n"
                << "Captured at epoch ms: " << epochMilliseconds << "\n"
                << "Preserved gates: M45, M46, M47, M48, M49, M50, M51\n"
                << "Aggregate M51-M59 validation is deferred to M59.\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildReleasePointer(
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "component=" << sanitizeSingleLine(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n";
            return output.str();
        }

        static void writeTextFile(const std::filesystem::path& path, const std::string& contents)
        {
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error)
            {
                return;
            }

            std::ofstream output(path, std::ios::out | std::ios::trunc);
            if (!output.is_open())
            {
                return;
            }

            output << contents;
            output.flush();
        }
    };
}
