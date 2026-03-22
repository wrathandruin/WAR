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
    struct IncidentResponseReport
    {
        bool incidentManifestPresent = false;
        bool incidentLaneReady = false;

        std::filesystem::path incidentDirectory;
        std::filesystem::path incidentManifestSourcePath;
        std::filesystem::path runtimeManifestPath;
        std::filesystem::path operatorBriefPath;
        std::filesystem::path latestPointerPath;
    };

    class IncidentResponseProtocol
    {
    public:
        [[nodiscard]] static IncidentResponseReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            IncidentResponseReport report{};
            report.incidentDirectory = incidentDirectory(runtimeBoundaryReport);
            report.incidentManifestSourcePath = resolveIncidentManifestPath(runtimeBoundaryReport);
            report.runtimeManifestPath = report.incidentDirectory / "incident_runtime_manifest.txt";
            report.operatorBriefPath = report.incidentDirectory / "incident_operator_brief.txt";
            report.latestPointerPath = report.incidentDirectory / "incident_latest_pointer.txt";

            std::error_code error;
            report.incidentManifestPresent =
                std::filesystem::exists(report.incidentManifestSourcePath, error)
                && std::filesystem::is_regular_file(report.incidentManifestSourcePath, error);
            report.incidentLaneReady = report.incidentManifestPresent;
            return report;
        }

        [[nodiscard]] static IncidentResponseReport recordIncidentState(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& incidentState)
        {
            IncidentResponseReport report = buildReport(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(report.incidentDirectory, error);
            if (error)
            {
                return report;
            }

            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();

            writeTextFile(
                report.runtimeManifestPath,
                buildRuntimeManifest(
                    component,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    incidentState,
                    nowEpochMilliseconds));

            writeTextFile(
                report.operatorBriefPath,
                buildOperatorBrief(
                    component,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    incidentState,
                    nowEpochMilliseconds));

            writeTextFile(
                report.latestPointerPath,
                buildPointer(
                    component,
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    incidentState,
                    nowEpochMilliseconds));

            report.incidentLaneReady = report.incidentManifestPresent;
            return report;
        }

        [[nodiscard]] static std::filesystem::path incidentDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "IncidentResponse";
        }

        [[nodiscard]] static std::filesystem::path resolveIncidentManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "IncidentResponse",
                "m53_incident_response_manifest.txt");
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

        [[nodiscard]] static std::string buildRuntimeManifest(
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& incidentState,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "component=" << sanitizeSingleLine(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "incident_state=" << sanitizeSingleLine(incidentState) << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildOperatorBrief(
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& incidentState,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "WAR Incident Response Brief\n"
                << "Component: " << sanitizeSingleLine(component) << "\n"
                << "Build identity: " << sanitizeSingleLine(buildIdentity) << "\n"
                << "Build channel: " << sanitizeSingleLine(buildChannel) << "\n"
                << "Environment: " << sanitizeSingleLine(environmentName) << "\n"
                << "Connect target: " << sanitizeSingleLine(connectTargetName) << "\n"
                << "Incident state: " << sanitizeSingleLine(incidentState) << "\n"
                << "Captured at epoch ms: " << epochMilliseconds << "\n"
                << "Primary incident step: preserve failure bundles, support handoff, live-ops summary, and release-management summary.\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildPointer(
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& incidentState,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "component=" << sanitizeSingleLine(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "incident_state=" << sanitizeSingleLine(incidentState) << "\n"
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
