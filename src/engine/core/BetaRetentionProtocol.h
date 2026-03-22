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
    struct BetaRetentionSummary
    {
        uint64_t totalLaunches = 0;
        uint64_t returningLaunches = 0;
        uint64_t lastLaunchEpochMilliseconds = 0;
        uint64_t contentCardCount = 0;
        std::string lastComponent = "none";
        std::string lastBuildIdentity = "unknown";
        std::string lastBuildChannel = "unknown";
        std::string lastEnvironmentName = "local";
        std::string lastConnectTargetName = "localhost-fallback";
    };

    class BetaRetentionProtocol
    {
    public:
        static void recordLaunch(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool returningPlayerDetected)
        {
            const std::filesystem::path summaryPath = summaryFilePath(runtimeBoundaryReport);
            const std::filesystem::path journalPath = journalFilePath(runtimeBoundaryReport);

            BetaRetentionSummary summary{};
            loadSummary(summaryPath, summary);

            summary.totalLaunches += 1u;
            if (returningPlayerDetected)
            {
                summary.returningLaunches += 1u;
            }

            summary.lastLaunchEpochMilliseconds = currentEpochMilliseconds();
            summary.contentCardCount = countContentCards(resolveBetaContentManifestPath(runtimeBoundaryReport));
            summary.lastComponent = sanitizeSingleLine(component);
            summary.lastBuildIdentity = sanitizeSingleLine(buildIdentity);
            summary.lastBuildChannel = sanitizeSingleLine(buildChannel);
            summary.lastEnvironmentName = sanitizeSingleLine(environmentName);
            summary.lastConnectTargetName = sanitizeSingleLine(connectTargetName);

            writeSummary(summaryPath, summary);
            appendJournal(
                journalPath,
                summary.lastLaunchEpochMilliseconds,
                component,
                buildIdentity,
                buildChannel,
                environmentName,
                connectTargetName,
                returningPlayerDetected,
                summary.totalLaunches,
                summary.returningLaunches,
                summary.contentCardCount);
        }

        [[nodiscard]] static bool hasPersistedResumeIdentity(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            const std::filesystem::path path = runtimeBoundaryReport.configDirectory / "client_resume_identity.txt";
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_regular_file(path, error);
        }

        [[nodiscard]] static std::filesystem::path retentionDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "Retention";
        }

        [[nodiscard]] static std::filesystem::path summaryFilePath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return retentionDirectory(runtimeBoundaryReport) / "beta_retention_summary.txt";
        }

        [[nodiscard]] static std::filesystem::path journalFilePath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return retentionDirectory(runtimeBoundaryReport) / "beta_launch_journal.txt";
        }

        [[nodiscard]] static std::filesystem::path resolveBetaContentManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "BetaContent",
                "m49_beta_content_manifest.txt");
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

        static void loadSummary(const std::filesystem::path& summaryPath, BetaRetentionSummary& outSummary)
        {
            outSummary = BetaRetentionSummary{};

            std::ifstream input(summaryPath, std::ios::in);
            if (!input.is_open())
            {
                return;
            }

            KeyValueMap values{};
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

            outSummary.totalLaunches = readUnsigned(values, "total_launches");
            outSummary.returningLaunches = readUnsigned(values, "returning_launches");
            outSummary.lastLaunchEpochMilliseconds = readUnsigned(values, "last_launch_epoch_ms");
            outSummary.contentCardCount = readUnsigned(values, "content_card_count");
            outSummary.lastComponent = readString(values, "last_component", "none");
            outSummary.lastBuildIdentity = readString(values, "last_build_identity", "unknown");
            outSummary.lastBuildChannel = readString(values, "last_build_channel", "unknown");
            outSummary.lastEnvironmentName = readString(values, "last_environment_name", "local");
            outSummary.lastConnectTargetName = readString(values, "last_connect_target_name", "localhost-fallback");
        }

        static void writeSummary(const std::filesystem::path& summaryPath, const BetaRetentionSummary& summary)
        {
            std::error_code error;
            std::filesystem::create_directories(summaryPath.parent_path(), error);
            if (error)
            {
                return;
            }

            std::ofstream output(summaryPath, std::ios::out | std::ios::trunc);
            if (!output.is_open())
            {
                return;
            }

            output
                << "version=1\n"
                << "total_launches=" << summary.totalLaunches << "\n"
                << "returning_launches=" << summary.returningLaunches << "\n"
                << "last_launch_epoch_ms=" << summary.lastLaunchEpochMilliseconds << "\n"
                << "content_card_count=" << summary.contentCardCount << "\n"
                << "last_component=" << summary.lastComponent << "\n"
                << "last_build_identity=" << summary.lastBuildIdentity << "\n"
                << "last_build_channel=" << summary.lastBuildChannel << "\n"
                << "last_environment_name=" << summary.lastEnvironmentName << "\n"
                << "last_connect_target_name=" << summary.lastConnectTargetName << "\n";
        }

        static void appendJournal(
            const std::filesystem::path& journalPath,
            uint64_t epochMilliseconds,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool returningPlayerDetected,
            uint64_t totalLaunches,
            uint64_t returningLaunches,
            uint64_t contentCardCount)
        {
            std::error_code error;
            std::filesystem::create_directories(journalPath.parent_path(), error);
            if (error)
            {
                return;
            }

            std::ofstream output(journalPath, std::ios::out | std::ios::app);
            if (!output.is_open())
            {
                return;
            }

            output
                << "epoch_ms=" << epochMilliseconds
                << "|component=" << sanitizeSingleLine(component)
                << "|build_identity=" << sanitizeSingleLine(buildIdentity)
                << "|build_channel=" << sanitizeSingleLine(buildChannel)
                << "|environment=" << sanitizeSingleLine(environmentName)
                << "|connect_target=" << sanitizeSingleLine(connectTargetName)
                << "|returning=" << (returningPlayerDetected ? "yes" : "no")
                << "|total_launches=" << totalLaunches
                << "|returning_launches=" << returningLaunches
                << "|content_card_count=" << contentCardCount
                << "\n";
        }

        [[nodiscard]] static uint64_t countContentCards(const std::filesystem::path& manifestPath)
        {
            std::ifstream input(manifestPath, std::ios::in);
            if (!input.is_open())
            {
                return 0u;
            }

            uint64_t count = 0u;
            std::string line;
            while (std::getline(input, line))
            {
                if (line.rfind("card_", 0) == 0 && line.find("_id=") != std::string::npos)
                {
                    ++count;
                }
            }

            return count;
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

        [[nodiscard]] static std::string readString(const KeyValueMap& values, const std::string& key, const std::string& fallback)
        {
            const auto it = values.find(key);
            if (it == values.end() || it->second.empty())
            {
                return fallback;
            }

            return it->second;
        }
    };
}
