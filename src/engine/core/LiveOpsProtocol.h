#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>

#include <windows.h>

#include "engine/core/RuntimePaths.h"
#include "engine/core/SourceManifestLayout.h"

namespace war
{
    struct LiveOpsReport
    {
        bool liveOpsManifestPresent = false;
        bool analyticsManifestPresent = false;
        bool liveOpsLaneReady = false;

        std::filesystem::path liveOpsDirectory;
        std::filesystem::path liveOpsManifestPath;
        std::filesystem::path analyticsManifestPath;
        std::filesystem::path runtimeManifestPath;
        std::filesystem::path operatorBriefPath;
        std::filesystem::path latestPointerPath;
        std::filesystem::path countersPath;

        uint64_t clientLaunches = 0;
        uint64_t hostBootstrapLaunches = 0;
        uint64_t returningLaunches = 0;
        uint64_t firstSessionLaunches = 0;
        uint64_t releaseStateWrites = 0;
    };

    class LiveOpsProtocol
    {
    public:
        [[nodiscard]] static LiveOpsReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            LiveOpsReport report{};
            report.liveOpsDirectory = liveOpsDirectory(runtimeBoundaryReport);
            report.liveOpsManifestPath = resolveLiveOpsManifestPath(runtimeBoundaryReport);
            report.analyticsManifestPath = resolveAnalyticsManifestPath(runtimeBoundaryReport);
            report.runtimeManifestPath = report.liveOpsDirectory / "live_ops_runtime_manifest.txt";
            report.operatorBriefPath = report.liveOpsDirectory / "live_ops_operator_brief.txt";
            report.latestPointerPath = report.liveOpsDirectory / "live_ops_latest_pointer.txt";
            report.countersPath = report.liveOpsDirectory / "live_ops_counters.txt";

            std::error_code error;
            report.liveOpsManifestPresent =
                std::filesystem::exists(report.liveOpsManifestPath, error)
                && std::filesystem::is_regular_file(report.liveOpsManifestPath, error);

            error.clear();
            report.analyticsManifestPresent =
                std::filesystem::exists(report.analyticsManifestPath, error)
                && std::filesystem::is_regular_file(report.analyticsManifestPath, error);

            loadCounters(report.countersPath, report);
            report.liveOpsLaneReady = report.liveOpsManifestPresent && report.analyticsManifestPresent;
            return report;
        }

        [[nodiscard]] static LiveOpsReport recordClientLaunch(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& sessionEntryState,
            bool returningPlayerDetected,
            bool firstSessionDetected)
        {
            LiveOpsReport report = buildReport(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(report.liveOpsDirectory, error);

            report.clientLaunches += 1u;
            if (returningPlayerDetected)
            {
                report.returningLaunches += 1u;
            }
            if (firstSessionDetected)
            {
                report.firstSessionLaunches += 1u;
            }

            writeCounters(report.countersPath, report);
            writeRuntimeManifest(
                report.runtimeManifestPath,
                "client",
                buildIdentity,
                buildChannel,
                environmentName,
                connectTargetName,
                sessionEntryState,
                returningPlayerDetected,
                firstSessionDetected,
                report);
            writeOperatorBrief(
                report.operatorBriefPath,
                "client",
                buildIdentity,
                buildChannel,
                environmentName,
                connectTargetName,
                sessionEntryState,
                returningPlayerDetected,
                firstSessionDetected,
                report);
            writeLatestPointer(
                report.latestPointerPath,
                "client",
                buildIdentity,
                environmentName,
                connectTargetName,
                currentEpochMilliseconds());
            return report;
        }

        [[nodiscard]] static LiveOpsReport recordHostBootstrap(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName)
        {
            LiveOpsReport report = buildReport(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(report.liveOpsDirectory, error);

            report.hostBootstrapLaunches += 1u;
            writeCounters(report.countersPath, report);
            writeRuntimeManifest(
                report.runtimeManifestPath,
                "host-bootstrap",
                buildIdentity,
                buildChannel,
                environmentName,
                connectTargetName,
                "bootstrap",
                false,
                false,
                report);
            writeOperatorBrief(
                report.operatorBriefPath,
                "host-bootstrap",
                buildIdentity,
                buildChannel,
                environmentName,
                connectTargetName,
                "bootstrap",
                false,
                false,
                report);
            writeLatestPointer(
                report.latestPointerPath,
                "host-bootstrap",
                buildIdentity,
                environmentName,
                connectTargetName,
                currentEpochMilliseconds());
            return report;
        }

        static void noteReleaseStateWrite(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            LiveOpsReport report = buildReport(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(report.liveOpsDirectory, error);
            report.releaseStateWrites += 1u;
            writeCounters(report.countersPath, report);
        }

        [[nodiscard]] static std::filesystem::path liveOpsDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "LiveOps";
        }

        [[nodiscard]] static std::filesystem::path resolveLiveOpsManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "LiveOps",
                "m52_live_ops_tooling_manifest.txt");
        }

        [[nodiscard]] static std::filesystem::path resolveAnalyticsManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "LiveOps",
                "m52_analytics_hardening_manifest.txt");
        }

    private:
        using KeyValueMap = std::unordered_map<std::string, std::string>;

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

        static void loadCounters(const std::filesystem::path& countersPath, LiveOpsReport& outReport)
        {
            const KeyValueMap values = parseKeyValueFile(countersPath);
            if (values.empty())
            {
                return;
            }

            outReport.clientLaunches = readUnsigned(values, "client_launches");
            outReport.hostBootstrapLaunches = readUnsigned(values, "host_bootstrap_launches");
            outReport.returningLaunches = readUnsigned(values, "returning_launches");
            outReport.firstSessionLaunches = readUnsigned(values, "first_session_launches");
            outReport.releaseStateWrites = readUnsigned(values, "release_state_writes");
        }

        static void writeCounters(const std::filesystem::path& countersPath, const LiveOpsReport& report)
        {
            writeTextFile(
                countersPath,
                "version=1\n"
                "client_launches=" + std::to_string(report.clientLaunches) + "\n"
                "host_bootstrap_launches=" + std::to_string(report.hostBootstrapLaunches) + "\n"
                "returning_launches=" + std::to_string(report.returningLaunches) + "\n"
                "first_session_launches=" + std::to_string(report.firstSessionLaunches) + "\n"
                "release_state_writes=" + std::to_string(report.releaseStateWrites) + "\n");
        }

        static void writeRuntimeManifest(
            const std::filesystem::path& path,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& sessionEntryState,
            bool returningPlayerDetected,
            bool firstSessionDetected,
            const LiveOpsReport& report)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "component=" << sanitizeSingleLine(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "session_entry_state=" << sanitizeSingleLine(sessionEntryState) << "\n"
                << "returning_player_detected=" << (returningPlayerDetected ? "yes" : "no") << "\n"
                << "first_session_detected=" << (firstSessionDetected ? "yes" : "no") << "\n"
                << "captured_at_epoch_ms=" << currentEpochMilliseconds() << "\n"
                << "client_launches=" << report.clientLaunches << "\n"
                << "host_bootstrap_launches=" << report.hostBootstrapLaunches << "\n"
                << "returning_launches=" << report.returningLaunches << "\n"
                << "first_session_launches=" << report.firstSessionLaunches << "\n"
                << "release_state_writes=" << report.releaseStateWrites << "\n";
            writeTextFile(path, output.str());
        }

        static void writeOperatorBrief(
            const std::filesystem::path& path,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& sessionEntryState,
            bool returningPlayerDetected,
            bool firstSessionDetected,
            const LiveOpsReport& report)
        {
            std::ostringstream output;
            output
                << "WAR Live-Ops Operator Brief\n"
                << "Component: " << sanitizeSingleLine(component) << "\n"
                << "Build identity: " << sanitizeSingleLine(buildIdentity) << "\n"
                << "Build channel: " << sanitizeSingleLine(buildChannel) << "\n"
                << "Environment: " << sanitizeSingleLine(environmentName) << "\n"
                << "Connect target: " << sanitizeSingleLine(connectTargetName) << "\n"
                << "Session entry state: " << sanitizeSingleLine(sessionEntryState) << "\n"
                << "Returning player detected: " << (returningPlayerDetected ? "yes" : "no") << "\n"
                << "First session detected: " << (firstSessionDetected ? "yes" : "no") << "\n"
                << "Client launches: " << report.clientLaunches << "\n"
                << "Host bootstrap launches: " << report.hostBootstrapLaunches << "\n"
                << "Returning launches: " << report.returningLaunches << "\n"
                << "First-session launches: " << report.firstSessionLaunches << "\n"
                << "Release-state writes: " << report.releaseStateWrites << "\n";
            writeTextFile(path, output.str());
        }

        static void writeLatestPointer(
            const std::filesystem::path& path,
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
            writeTextFile(path, output.str());
        }

        [[nodiscard]] static KeyValueMap parseKeyValueFile(const std::filesystem::path& path)
        {
            KeyValueMap values{};
            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                return values;
            }

            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                const size_t split = line.find('=');
                if (split == std::string::npos)
                {
                    continue;
                }

                values.emplace(line.substr(0, split), line.substr(split + 1));
            }
            return values;
        }

        [[nodiscard]] static uint64_t readUnsigned(const KeyValueMap& values, const std::string& key)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return 0u;
            }

            try
            {
                return static_cast<uint64_t>(std::stoull(it->second));
            }
            catch (...)
            {
                return 0u;
            }
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
