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

namespace war
{
    struct MarketOnboardingBrief
    {
        std::string title = "Welcome To Wrath And Ruin";
        std::string subtitle = "Read the room text, trust the prompt strip, and use the typed shell as part of play.";
        std::string primaryGoal = "Use left click to move, right click to interact, Shift+right click to inspect, and type help.";
        std::string controlsHint = "Left click move | Right click interact | Shift+Right click inspect | Enter submits typed commands";
        std::string sessionHint = "Use session to inspect entry state, entry for a fresh handoff, and resume when a saved identity exists.";
        std::string returnHint = "Returning players should continue from the retained world state and use resume identity when prompted.";
        std::string firstObjective = "Reach a different authored room and confirm the room-entry text changes.";
        std::string textLayerPromise = "The room title, description, prompt strip, and typed shell are part of the intended product experience.";
    };

    struct MarketOnboardingReport
    {
        bool onboardingManifestPresent = false;
        bool firstSessionManifestPresent = false;
        bool onboardingStatePresent = false;
        bool onboardingLaneReady = false;
        bool firstSessionDetected = true;

        std::filesystem::path onboardingDirectory;
        std::filesystem::path onboardingManifestPath;
        std::filesystem::path firstSessionManifestPath;
        std::filesystem::path onboardingStatePath;
        std::filesystem::path onboardingSummaryPath;
        std::filesystem::path onboardingBriefPath;
        std::filesystem::path onboardingPointerPath;
    };

    class MarketOnboardingProtocol
    {
    public:
        [[nodiscard]] static MarketOnboardingReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            MarketOnboardingReport report{};
            report.onboardingDirectory = onboardingDirectory(runtimeBoundaryReport);
            report.onboardingManifestPath = resolveOnboardingManifestPath(runtimeBoundaryReport);
            report.firstSessionManifestPath = resolveFirstSessionManifestPath(runtimeBoundaryReport);
            report.onboardingStatePath = report.onboardingDirectory / "market_onboarding_state.txt";
            report.onboardingSummaryPath = report.onboardingDirectory / "market_onboarding_summary.txt";
            report.onboardingBriefPath = report.onboardingDirectory / "market_onboarding_brief.txt";
            report.onboardingPointerPath = report.onboardingDirectory / "market_onboarding_latest_pointer.txt";

            std::error_code error;
            report.onboardingManifestPresent =
                std::filesystem::exists(report.onboardingManifestPath, error)
                && std::filesystem::is_regular_file(report.onboardingManifestPath, error);

            error.clear();
            report.firstSessionManifestPresent =
                std::filesystem::exists(report.firstSessionManifestPath, error)
                && std::filesystem::is_regular_file(report.firstSessionManifestPath, error);

            error.clear();
            report.onboardingStatePresent =
                std::filesystem::exists(report.onboardingStatePath, error)
                && std::filesystem::is_regular_file(report.onboardingStatePath, error);

            report.onboardingLaneReady = report.onboardingManifestPresent && report.firstSessionManifestPresent;

            const KeyValueMap stateValues = parseKeyValueFile(report.onboardingStatePath);
            const auto stateIt = stateValues.find("first_session_detected");
            if (stateIt != stateValues.end())
            {
                report.firstSessionDetected = stateIt->second != "no";
            }
            else
            {
                report.firstSessionDetected = !report.onboardingStatePresent;
            }

            return report;
        }
        [[nodiscard]] static MarketOnboardingReport recordSession(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool returningPlayerDetected)
        {
            MarketOnboardingReport report = buildReport(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(report.onboardingDirectory, error);
            if (error)
            {
                return report;
            }

            report.firstSessionDetected = !returningPlayerDetected && !report.onboardingStatePresent;
            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();

            writeTextFile(
                report.onboardingStatePath,
                buildStateFile(
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    returningPlayerDetected,
                    report.firstSessionDetected,
                    nowEpochMilliseconds));

            writeTextFile(
                report.onboardingSummaryPath,
                buildSummaryFile(
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    returningPlayerDetected,
                    report.firstSessionDetected,
                    nowEpochMilliseconds));

            MarketOnboardingBrief brief{};
            (void)loadBrief(runtimeBoundaryReport, brief);
            writeTextFile(report.onboardingBriefPath, buildBriefFile(brief));

            writeTextFile(
                report.onboardingPointerPath,
                buildPointerFile(
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    report.firstSessionDetected,
                    nowEpochMilliseconds));

            report.onboardingStatePresent = true;
            return report;
        }
        [[nodiscard]] static bool loadBrief(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            MarketOnboardingBrief& outBrief)
        {
            outBrief = MarketOnboardingBrief{};

            const KeyValueMap values = parseKeyValueFile(resolveOnboardingManifestPath(runtimeBoundaryReport));
            if (values.empty())
            {
                return false;
            }

            applyIfPresent(values, "title", outBrief.title);
            applyIfPresent(values, "subtitle", outBrief.subtitle);
            applyIfPresent(values, "primary_goal", outBrief.primaryGoal);
            applyIfPresent(values, "controls_hint", outBrief.controlsHint);
            applyIfPresent(values, "session_hint", outBrief.sessionHint);
            applyIfPresent(values, "return_hint", outBrief.returnHint);
            applyIfPresent(values, "first_objective", outBrief.firstObjective);
            applyIfPresent(values, "text_layer_promise", outBrief.textLayerPromise);
            return true;
        }

        [[nodiscard]] static std::filesystem::path onboardingDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "Onboarding";
        }

        [[nodiscard]] static std::filesystem::path resolveOnboardingManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return resolvePreferredPath(
                runtimeBoundaryReport.executableDirectory / "Onboarding" / "m51_market_onboarding_manifest.txt",
                runtimeBoundaryReport.repoRoot / "Onboarding" / "m51_market_onboarding_manifest.txt");
        }

        [[nodiscard]] static std::filesystem::path resolveFirstSessionManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return resolvePreferredPath(
                runtimeBoundaryReport.executableDirectory / "Onboarding" / "m51_first_session_polish_manifest.txt",
                runtimeBoundaryReport.repoRoot / "Onboarding" / "m51_first_session_polish_manifest.txt");
        }

    private:
        using KeyValueMap = std::unordered_map<std::string, std::string>;

        [[nodiscard]] static std::filesystem::path resolvePreferredPath(
            const std::filesystem::path& packagedPath,
            const std::filesystem::path& sourcePath)
        {
            std::error_code error;
            if (std::filesystem::exists(packagedPath, error) && std::filesystem::is_regular_file(packagedPath, error))
            {
                return packagedPath;
            }

            error.clear();
            if (std::filesystem::exists(sourcePath, error) && std::filesystem::is_regular_file(sourcePath, error))
            {
                return sourcePath;
            }

            return packagedPath;
        }

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

        static void applyIfPresent(const KeyValueMap& values, const std::string& key, std::string& target)
        {
            const auto it = values.find(key);
            if (it != values.end() && !it->second.empty())
            {
                target = it->second;
            }
        }

        [[nodiscard]] static KeyValueMap parseKeyValueFile(const std::filesystem::path& path)
        {
            KeyValueMap values;

            std::ifstream input(path);
            if (!input.is_open())
            {
                return values;
            }

            std::string line;
            while (std::getline(input, line))
            {
                const size_t split = line.find('=');
                if (split == std::string::npos)
                {
                    continue;
                }

                values[line.substr(0, split)] = line.substr(split + 1);
            }

            return values;
        }

        [[nodiscard]] static std::string buildStateFile(
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool returningPlayerDetected,
            bool firstSessionDetected,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "returning_player_detected=" << (returningPlayerDetected ? "yes" : "no") << "\n"
                << "first_session_detected=" << (firstSessionDetected ? "yes" : "no") << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildSummaryFile(
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool returningPlayerDetected,
            bool firstSessionDetected,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "WAR Market Onboarding Summary\n"
                << "Build identity: " << sanitizeSingleLine(buildIdentity) << "\n"
                << "Build channel: " << sanitizeSingleLine(buildChannel) << "\n"
                << "Environment: " << sanitizeSingleLine(environmentName) << "\n"
                << "Connect target: " << sanitizeSingleLine(connectTargetName) << "\n"
                << "Returning player detected: " << (returningPlayerDetected ? "yes" : "no") << "\n"
                << "First session detected: " << (firstSessionDetected ? "yes" : "no") << "\n"
                << "Captured at epoch ms: " << epochMilliseconds << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildBriefFile(const MarketOnboardingBrief& brief)
        {
            std::ostringstream output;
            output
                << "WAR Market Onboarding Brief\n"
                << "Title: " << sanitizeSingleLine(brief.title) << "\n"
                << "Subtitle: " << sanitizeSingleLine(brief.subtitle) << "\n"
                << "Primary goal: " << sanitizeSingleLine(brief.primaryGoal) << "\n"
                << "Controls hint: " << sanitizeSingleLine(brief.controlsHint) << "\n"
                << "Session hint: " << sanitizeSingleLine(brief.sessionHint) << "\n"
                << "Return hint: " << sanitizeSingleLine(brief.returnHint) << "\n"
                << "First objective: " << sanitizeSingleLine(brief.firstObjective) << "\n"
                << "Text layer promise: " << sanitizeSingleLine(brief.textLayerPromise) << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildPointerFile(
            const std::string& buildIdentity,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool firstSessionDetected,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "first_session_detected=" << (firstSessionDetected ? "yes" : "no") << "\n"
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
