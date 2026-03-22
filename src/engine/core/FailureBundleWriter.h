#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include <windows.h>

#include "engine/core/FailureBundleProtocol.h"
#include "engine/core/RuntimePaths.h"

namespace war
{
    class FailureBundleWriter
    {
    public:
        [[nodiscard]] static bool capture(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            FailureBundleKind bundleKind,
            const std::string& component,
            const std::string& failureStage,
            const std::string& failureCode,
            const std::string& failureReason,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            int processExitCode,
            const std::vector<std::filesystem::path>& attachmentCandidates,
            std::string& outBundleDirectory,
            std::string& outError)
        {
            outBundleDirectory.clear();
            outError.clear();

            FailureBundleProtocolReport report = FailureBundleProtocol::buildReport(runtimeBoundaryReport);
            FailureBundleProtocol::ensureDirectories(report);
            if (!report.failureBundleLaneReady)
            {
                outError = "Failure-bundle lane is not ready.";
                return false;
            }

            const std::filesystem::path targetRoot = directoryForKind(report, bundleKind);
            if (targetRoot.empty())
            {
                outError = "Failure-bundle target directory is unresolved.";
                return false;
            }

            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();
            const std::string sanitizedComponent = sanitizeToken(component);
            const std::string sanitizedCode = sanitizeToken(failureCode);
            const std::string bundleName =
                std::to_string(nowEpochMilliseconds)
                + "_"
                + sanitizedComponent
                + "_"
                + sanitizedCode;

            const std::filesystem::path bundleDirectory = targetRoot / bundleName;
            std::error_code error;
            std::filesystem::create_directories(bundleDirectory, error);
            if (error)
            {
                outError = std::string("Failed to create failure-bundle directory: ") + bundleDirectory.generic_string();
                return false;
            }

            const std::filesystem::path manifestPath = bundleDirectory / "bundle_manifest.txt";
            const std::filesystem::path attachmentsPath = bundleDirectory / "attachments_manifest.txt";

            std::ostringstream manifest;
            manifest
                << "bundle_version=1\n"
                << "bundle_kind=" << FailureBundleProtocol::kindText(bundleKind) << "\n"
                << "component=" << sanitizeToken(component) << "\n"
                << "failure_stage=" << sanitizeToken(failureStage) << "\n"
                << "failure_code=" << sanitizeToken(failureCode) << "\n"
                << "failure_reason=" << sanitizeSingleLine(failureReason) << "\n"
                << "process_exit_code=" << processExitCode << "\n"
                << "captured_at_epoch_ms=" << nowEpochMilliseconds << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeToken(buildChannel) << "\n"
                << "environment_name=" << sanitizeToken(environmentName) << "\n"
                << "connect_target_name=" << sanitizeToken(connectTargetName) << "\n"
                << "runtime_root=" << runtimeBoundaryReport.runtimeRoot.generic_string() << "\n"
                << "logs_directory=" << runtimeBoundaryReport.logsDirectory.generic_string() << "\n"
                << "crash_directory=" << runtimeBoundaryReport.crashDirectory.generic_string() << "\n";

            std::ostringstream attachmentManifest;
            attachmentManifest
                << "bundle_directory=" << bundleDirectory.generic_string() << "\n";

            size_t copiedCount = 0;
            for (const std::filesystem::path& candidate : attachmentCandidates)
            {
                if (candidate.empty())
                {
                    continue;
                }

                const std::filesystem::path normalizedCandidate = candidate.lexically_normal();
                const std::string filename = normalizedCandidate.filename().string();
                const std::filesystem::path destinationPath = bundleDirectory / filename;

                if (copyFileIfPresent(normalizedCandidate, destinationPath))
                {
                    ++copiedCount;
                    attachmentManifest
                        << "copied=" << normalizedCandidate.generic_string()
                        << " -> " << destinationPath.generic_string() << "\n";
                }
                else
                {
                    attachmentManifest
                        << "missing=" << normalizedCandidate.generic_string() << "\n";
                }
            }

            attachmentManifest << "copied_count=" << copiedCount << "\n";

            if (!writeTextFile(manifestPath, manifest.str()))
            {
                outError = std::string("Failed to write failure-bundle manifest: ") + manifestPath.generic_string();
                return false;
            }

            if (!writeTextFile(attachmentsPath, attachmentManifest.str()))
            {
                outError = std::string("Failed to write failure-bundle attachment manifest: ") + attachmentsPath.generic_string();
                return false;
            }

            if (!writeOperatorTriageArtifacts(
                    report,
                    bundleKind,
                    bundleDirectory,
                    manifestPath,
                    attachmentsPath,
                    component,
                    failureStage,
                    failureCode,
                    failureReason,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    processExitCode,
                    copiedCount,
                    nowEpochMilliseconds))
            {
                outError = std::string("Failed to write operator-triage artifacts for bundle: ") + bundleDirectory.generic_string();
                return false;
            }

            outBundleDirectory = bundleDirectory.generic_string();
            return true;
        }

        [[nodiscard]] static std::vector<std::filesystem::path> clientFailureAttachments(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return {
                runtimeBoundaryReport.logsDirectory / "local_demo_startup_report.txt",
                runtimeBoundaryReport.logsDirectory / "client_runtime_trace.txt",
                runtimeBoundaryReport.logsDirectory / "client_replication_status.txt",
                runtimeBoundaryReport.hostDirectory / "headless_host_status.txt"
            };
        }

        [[nodiscard]] static std::vector<std::filesystem::path> bootstrapFailureAttachments(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return {
                runtimeBoundaryReport.logsDirectory / "local_demo_startup_report.txt",
                runtimeBoundaryReport.logsDirectory / "headless_host_trace.txt",
                runtimeBoundaryReport.logsDirectory / "headless_host_log.txt",
                runtimeBoundaryReport.hostDirectory / "headless_host_status.txt"
            };
        }

        [[nodiscard]] static std::vector<std::filesystem::path> headlessHostFailureAttachments(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return {
                runtimeBoundaryReport.logsDirectory / "local_demo_startup_report.txt",
                runtimeBoundaryReport.logsDirectory / "headless_host_trace.txt",
                runtimeBoundaryReport.logsDirectory / "headless_host_log.txt",
                runtimeBoundaryReport.logsDirectory / "client_replication_status.txt",
                runtimeBoundaryReport.hostDirectory / "headless_host_status.txt"
            };
        }

    private:
        [[nodiscard]] static std::filesystem::path directoryForKind(
            const FailureBundleProtocolReport& report,
            FailureBundleKind bundleKind)
        {
            switch (bundleKind)
            {
            case FailureBundleKind::Startup:
                return report.startupBundleDirectory;
            case FailureBundleKind::Runtime:
                return report.runtimeBundleDirectory;
            case FailureBundleKind::Bootstrap:
                return report.bootstrapBundleDirectory;
            case FailureBundleKind::OperatorTriage:
                return report.operatorTriageDirectory;
            default:
                return {};
            }
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

        [[nodiscard]] static bool writeTextFile(const std::filesystem::path& path, const std::string& contents)
        {
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error)
            {
                return false;
            }

            std::ofstream output(path, std::ios::out | std::ios::trunc);
            if (!output.is_open())
            {
                return false;
            }

            output << contents;
            output.flush();
            return output.good();
        }

        [[nodiscard]] static bool copyFileIfPresent(
            const std::filesystem::path& sourcePath,
            const std::filesystem::path& destinationPath)
        {
            std::error_code error;
            if (!std::filesystem::exists(sourcePath, error) || !std::filesystem::is_regular_file(sourcePath, error))
            {
                return false;
            }

            std::filesystem::copy_file(
                sourcePath,
                destinationPath,
                std::filesystem::copy_options::overwrite_existing,
                error);

            return !error;
        }

        [[nodiscard]] static bool writeOperatorTriageArtifacts(
            const FailureBundleProtocolReport& report,
            FailureBundleKind bundleKind,
            const std::filesystem::path& bundleDirectory,
            const std::filesystem::path& manifestPath,
            const std::filesystem::path& attachmentsPath,
            const std::string& component,
            const std::string& failureStage,
            const std::string& failureCode,
            const std::string& failureReason,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            int processExitCode,
            size_t copiedAttachmentCount,
            uint64_t capturedAtEpochMilliseconds)
        {
            const std::filesystem::path triageDirectory = report.operatorTriageDirectory;
            if (triageDirectory.empty())
            {
                return false;
            }

            std::ostringstream latestPointer;
            latestPointer
                << "bundle_kind=" << FailureBundleProtocol::kindText(bundleKind) << "\n"
                << "component=" << sanitizeToken(component) << "\n"
                << "failure_stage=" << sanitizeToken(failureStage) << "\n"
                << "failure_code=" << sanitizeToken(failureCode) << "\n"
                << "bundle_directory=" << bundleDirectory.generic_string() << "\n"
                << "captured_at_epoch_ms=" << capturedAtEpochMilliseconds << "\n";

            std::ostringstream triageSummary;
            triageSummary
                << "WAR Operator Triage Summary\n"
                << "Latest failure bundle kind: " << FailureBundleProtocol::kindText(bundleKind) << "\n"
                << "Component: " << sanitizeToken(component) << "\n"
                << "Failure stage: " << sanitizeToken(failureStage) << "\n"
                << "Failure code: " << sanitizeToken(failureCode) << "\n"
                << "Failure reason: " << sanitizeSingleLine(failureReason) << "\n"
                << "Process exit code: " << processExitCode << "\n"
                << "Captured at epoch ms: " << capturedAtEpochMilliseconds << "\n"
                << "Build identity: " << sanitizeSingleLine(buildIdentity) << "\n"
                << "Build channel: " << sanitizeToken(buildChannel) << "\n"
                << "Environment: " << sanitizeToken(environmentName) << "\n"
                << "Connect target: " << sanitizeToken(connectTargetName) << "\n"
                << "Latest bundle directory: " << bundleDirectory.generic_string() << "\n"
                << "Copied attachment count: " << copiedAttachmentCount << "\n"
                << "Primary files to inspect:\n"
                << "1. latest_bundle_manifest.txt\n"
                << "2. latest_attachments_manifest.txt\n"
                << "3. copied trace/log/status files in the latest bundle directory\n";

            std::ostringstream checklist;
            checklist
                << "WAR Operator Triage Checklist\n"
                << "1. Open latest_failure_bundle.txt to confirm the newest bundle path.\n"
                << "2. Read latest_bundle_manifest.txt for component, stage, code, and exit status.\n"
                << "3. Read latest_attachments_manifest.txt to see which runtime artifacts were copied versus missing.\n"
                << "4. Open the referenced latest bundle directory and inspect copied trace/log/status files.\n"
                << "5. Preserve the entire latest bundle directory when escalating the failure.\n";

            if (!writeTextFile(triageDirectory / "latest_failure_bundle.txt", latestPointer.str()))
            {
                return false;
            }

            if (!writeTextFile(triageDirectory / "operator_triage_summary.txt", triageSummary.str()))
            {
                return false;
            }

            if (!writeTextFile(triageDirectory / "operator_triage_checklist.txt", checklist.str()))
            {
                return false;
            }

            if (!copyFileIfPresent(manifestPath, triageDirectory / "latest_bundle_manifest.txt"))
            {
                return false;
            }

            if (!copyFileIfPresent(attachmentsPath, triageDirectory / "latest_attachments_manifest.txt"))
            {
                return false;
            }

            return true;
        }
    };
}
