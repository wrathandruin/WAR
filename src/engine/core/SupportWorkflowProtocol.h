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
    struct SupportWorkflowReport
    {
        bool supportManifestPresent = false;
        bool playerMessagingManifestPresent = false;
        bool supportLaneReady = false;

        std::filesystem::path supportDirectory;
        std::filesystem::path supportManifestPath;
        std::filesystem::path playerMessagingManifestPath;
        std::filesystem::path runtimeManifestPath;
        std::filesystem::path playerMessagePath;
        std::filesystem::path operatorHandoffPath;
        std::filesystem::path latestPointerPath;
    };

    class SupportWorkflowProtocol
    {
    public:
        [[nodiscard]] static SupportWorkflowReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            SupportWorkflowReport report{};
            report.supportDirectory = supportDirectory(runtimeBoundaryReport);
            report.supportManifestPath = resolveSupportManifestPath(runtimeBoundaryReport);
            report.playerMessagingManifestPath = resolvePlayerMessagingManifestPath(runtimeBoundaryReport);
            report.runtimeManifestPath = report.supportDirectory / "support_runtime_manifest.txt";
            report.playerMessagePath = report.supportDirectory / "player_message_brief.txt";
            report.operatorHandoffPath = report.supportDirectory / "support_operator_handoff.txt";
            report.latestPointerPath = report.supportDirectory / "support_latest_pointer.txt";

            std::error_code error;
            report.supportManifestPresent =
                std::filesystem::exists(report.supportManifestPath, error)
                && std::filesystem::is_regular_file(report.supportManifestPath, error);

            error.clear();
            report.playerMessagingManifestPresent =
                std::filesystem::exists(report.playerMessagingManifestPath, error)
                && std::filesystem::is_regular_file(report.playerMessagingManifestPath, error);

            report.supportLaneReady = report.supportManifestPresent && report.playerMessagingManifestPresent;
            return report;
        }

        [[nodiscard]] static SupportWorkflowReport recordClientSession(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& sessionState,
            bool returningPlayerDetected,
            bool firstSessionDetected)
        {
            SupportWorkflowReport report = buildReport(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(report.supportDirectory, error);
            if (error)
            {
                return report;
            }

            const MessagePack messages = loadMessages(report.playerMessagingManifestPath);
            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();

            writeTextFile(
                report.runtimeManifestPath,
                buildRuntimeManifest(
                    "client",
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    sessionState,
                    returningPlayerDetected,
                    firstSessionDetected,
                    nowEpochMilliseconds));

            const std::string playerMessage = firstSessionDetected
                ? messages.firstSessionMessage
                : (returningPlayerDetected ? messages.returningSessionMessage : messages.supportMessage);

            writeTextFile(
                report.playerMessagePath,
                buildPlayerMessage(
                    playerMessage,
                    messages.supportMessage,
                    messages.incidentMessage));

            writeTextFile(
                report.operatorHandoffPath,
                buildOperatorHandoff(
                    "client",
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    sessionState,
                    nowEpochMilliseconds));

            writeTextFile(
                report.latestPointerPath,
                buildLatestPointer(
                    "client",
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));

            return report;
        }

        [[nodiscard]] static SupportWorkflowReport recordHostBootstrap(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName)
        {
            SupportWorkflowReport report = buildReport(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(report.supportDirectory, error);
            if (error)
            {
                return report;
            }

            const MessagePack messages = loadMessages(report.playerMessagingManifestPath);
            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();

            writeTextFile(
                report.runtimeManifestPath,
                buildRuntimeManifest(
                    "host-bootstrap",
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    "bootstrap",
                    false,
                    false,
                    nowEpochMilliseconds));

            writeTextFile(
                report.playerMessagePath,
                buildPlayerMessage(
                    messages.supportMessage,
                    messages.supportMessage,
                    messages.incidentMessage));

            writeTextFile(
                report.operatorHandoffPath,
                buildOperatorHandoff(
                    "host-bootstrap",
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    "bootstrap",
                    nowEpochMilliseconds));

            writeTextFile(
                report.latestPointerPath,
                buildLatestPointer(
                    "host-bootstrap",
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));

            return report;
        }

        [[nodiscard]] static std::filesystem::path supportDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "Support";
        }

        [[nodiscard]] static std::filesystem::path resolveSupportManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "Support",
                "m53_support_workflow_manifest.txt");
        }

        [[nodiscard]] static std::filesystem::path resolvePlayerMessagingManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "Support",
                "m53_player_messaging_manifest.txt");
        }

    private:
        struct MessagePack
        {
            std::string firstSessionMessage =
                "Read the room text, trust the prompt strip, and type help if you lose the thread.";
            std::string returningSessionMessage =
                "Resume from the retained state, read the current room text, and use session or status before escalating.";
            std::string supportMessage =
                "If the session state looks wrong, preserve the packaged logs and share the support handoff summary.";
            std::string incidentMessage =
                "If a host or bootstrap lane fails, preserve the failure bundle and the incident-response summary.";
        };

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

        [[nodiscard]] static MessagePack loadMessages(const std::filesystem::path& path)
        {
            MessagePack messages{};
            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                return messages;
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

                const std::string key = line.substr(0, split);
                const std::string value = line.substr(split + 1);

                if (key == "first_session_message")
                {
                    messages.firstSessionMessage = value;
                }
                else if (key == "returning_session_message")
                {
                    messages.returningSessionMessage = value;
                }
                else if (key == "support_message")
                {
                    messages.supportMessage = value;
                }
                else if (key == "incident_message")
                {
                    messages.incidentMessage = value;
                }
            }

            return messages;
        }

        [[nodiscard]] static std::string buildRuntimeManifest(
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& sessionState,
            bool returningPlayerDetected,
            bool firstSessionDetected,
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
                << "session_state=" << sanitizeSingleLine(sessionState) << "\n"
                << "returning_player_detected=" << (returningPlayerDetected ? "yes" : "no") << "\n"
                << "first_session_detected=" << (firstSessionDetected ? "yes" : "no") << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildPlayerMessage(
            const std::string& primaryMessage,
            const std::string& supportMessage,
            const std::string& incidentMessage)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "primary_message=" << sanitizeSingleLine(primaryMessage) << "\n"
                << "support_message=" << sanitizeSingleLine(supportMessage) << "\n"
                << "incident_message=" << sanitizeSingleLine(incidentMessage) << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildOperatorHandoff(
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& environmentName,
            const std::string& connectTargetName,
            const std::string& sessionState,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "WAR Support Operator Handoff\n"
                << "Component: " << sanitizeSingleLine(component) << "\n"
                << "Build identity: " << sanitizeSingleLine(buildIdentity) << "\n"
                << "Environment: " << sanitizeSingleLine(environmentName) << "\n"
                << "Connect target: " << sanitizeSingleLine(connectTargetName) << "\n"
                << "Session state: " << sanitizeSingleLine(sessionState) << "\n"
                << "Captured at epoch ms: " << epochMilliseconds << "\n"
                << "Primary support step: preserve logs, startup report, and current support runtime manifest.\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildLatestPointer(
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
