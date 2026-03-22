#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

#include <windows.h>

#include "engine/core/BetaRetentionProtocol.h"
#include "engine/core/RuntimePaths.h"

namespace war
{
    class ReleaseCandidateProtocol
    {
    public:
        static void recordCandidateState(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName)
        {
            const std::filesystem::path directory = releaseCandidateDirectory(runtimeBoundaryReport);

            std::error_code error;
            std::filesystem::create_directories(directory, error);
            if (error)
            {
                return;
            }

            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();
            const std::filesystem::path contentManifestPath =
                BetaRetentionProtocol::resolveBetaContentManifestPath(runtimeBoundaryReport);
            const uint64_t contentCardCount = countContentCards(contentManifestPath);

            const std::string rcId =
                std::string("rc-")
                + sanitizeToken(environmentName)
                + "-"
                + sanitizeToken(component)
                + "-"
                + std::to_string(nowEpochMilliseconds);

            writeTextFile(
                directory / "release_candidate_manifest.txt",
                buildManifest(
                    rcId,
                    component,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds,
                    contentManifestPath,
                    contentCardCount));

            writeTextFile(
                directory / "release_candidate_checklist.txt",
                buildChecklist());

            writeTextFile(
                directory / "latest_release_candidate_pointer.txt",
                buildPointer(rcId, component, environmentName, connectTargetName, nowEpochMilliseconds));
        }

        [[nodiscard]] static std::filesystem::path releaseCandidateDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "ReleaseCandidate";
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

        [[nodiscard]] static std::string sanitizeToken(std::string value)
        {
            if (value.empty())
            {
                return "unresolved";
            }

            std::string sanitized;
            sanitized.reserve(value.size());

            for (const unsigned char raw : value)
            {
                const char ch = static_cast<char>(raw);
                if ((ch >= 'a' && ch <= 'z')
                    || (ch >= 'A' && ch <= 'Z')
                    || (ch >= '0' && ch <= '9')
                    || ch == '_'
                    || ch == '-'
                    || ch == '.')
                {
                    sanitized.push_back(static_cast<char>(std::tolower(raw)));
                }
                else if (ch == ' ' || ch == ':' || ch == '/' || ch == '\\')
                {
                    sanitized.push_back('_');
                }
            }

            while (!sanitized.empty() && (sanitized.front() == '.' || sanitized.front() == '_'))
            {
                sanitized.erase(sanitized.begin());
            }

            if (sanitized.empty())
            {
                sanitized = "unresolved";
            }

            return sanitized;
        }

        [[nodiscard]] static std::string buildManifest(
            const std::string& rcId,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t capturedAtEpochMilliseconds,
            const std::filesystem::path& contentManifestPath,
            uint64_t contentCardCount)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "release_candidate_id=" << sanitizeToken(rcId) << "\n"
                << "component=" << sanitizeToken(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
                << "environment_name=" << sanitizeToken(environmentName) << "\n"
                << "connect_target_name=" << sanitizeToken(connectTargetName) << "\n"
                << "captured_at_epoch_ms=" << capturedAtEpochMilliseconds << "\n"
                << "beta_content_manifest_path=" << contentManifestPath.generic_string() << "\n"
                << "beta_content_card_count=" << contentCardCount << "\n"
                << "required_validation_scripts=validate_m45_hosted_bootstrap_win64.bat,validate_m46_environment_identity_win64.bat,validate_m46_missing_required_secrets_win64.bat,validate_m46_runtime_save_hygiene_win64.bat,validate_m47_ticket_issue_and_client_entry_win64.bat,validate_m47_ticket_denial_and_fail_states_win64.bat,validate_m47_reconnect_identity_win64.bat,validate_m48_failure_bundle_capture_win64.bat,validate_m48_operator_triage_artifacts_win64.bat,validate_m49_beta_content_scale_win64.bat,validate_m49_retention_expansion_win64.bat,validate_m49_release_candidate_discipline_win64.bat\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildChecklist()
        {
            std::ostringstream output;
            output
                << "WAR Release Candidate Checklist\n"
                << "1. Confirm the beta content manifest is staged in BetaContent/.\n"
                << "2. Confirm retention summary and launch journal are present under Runtime/Logs/Retention.\n"
                << "3. Confirm release candidate manifest and checklist are present under Runtime/Logs/ReleaseCandidate.\n"
                << "4. Run the preserved M45 through M48 validation lanes before treating the beta candidate as credible.\n"
                << "5. Run the M49 validation lane before classifying the candidate for sign-off.\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildPointer(
            const std::string& rcId,
            const std::string& component,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t capturedAtEpochMilliseconds)
        {
            std::ostringstream output;
            output
                << "release_candidate_id=" << sanitizeToken(rcId) << "\n"
                << "component=" << sanitizeToken(component) << "\n"
                << "environment_name=" << sanitizeToken(environmentName) << "\n"
                << "connect_target_name=" << sanitizeToken(connectTargetName) << "\n"
                << "captured_at_epoch_ms=" << capturedAtEpochMilliseconds << "\n";
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
    };
}
