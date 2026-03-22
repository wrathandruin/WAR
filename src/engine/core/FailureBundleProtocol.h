#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"

namespace war
{
    enum class FailureBundleKind
    {
        Startup = 0,
        Runtime,
        Bootstrap,
        OperatorTriage
    };

    struct FailureBundleProtocolReport
    {
        bool failureBundleRootReady = false;
        bool startupBundleDirectoryReady = false;
        bool runtimeBundleDirectoryReady = false;
        bool bootstrapBundleDirectoryReady = false;
        bool operatorTriageDirectoryReady = false;
        bool failureBundleLaneReady = false;

        std::filesystem::path failureBundleRootDirectory;
        std::filesystem::path startupBundleDirectory;
        std::filesystem::path runtimeBundleDirectory;
        std::filesystem::path bootstrapBundleDirectory;
        std::filesystem::path operatorTriageDirectory;

        std::vector<std::string> issues;
    };

    class FailureBundleProtocol
    {
    public:
        [[nodiscard]] static FailureBundleProtocolReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            FailureBundleProtocolReport report{};
            report.failureBundleRootDirectory = runtimeBoundaryReport.failureBundleRootDirectory;
            report.startupBundleDirectory = runtimeBoundaryReport.startupFailureBundleDirectory;
            report.runtimeBundleDirectory = runtimeBoundaryReport.runtimeFailureBundleDirectory;
            report.bootstrapBundleDirectory = runtimeBoundaryReport.bootstrapFailureBundleDirectory;
            report.operatorTriageDirectory = runtimeBoundaryReport.operatorTriageDirectory;

            report.failureBundleRootReady = directoryExists(report.failureBundleRootDirectory);
            report.startupBundleDirectoryReady = directoryExists(report.startupBundleDirectory);
            report.runtimeBundleDirectoryReady = directoryExists(report.runtimeBundleDirectory);
            report.bootstrapBundleDirectoryReady = directoryExists(report.bootstrapBundleDirectory);
            report.operatorTriageDirectoryReady = directoryExists(report.operatorTriageDirectory);

            report.failureBundleLaneReady =
                report.failureBundleRootReady
                && report.startupBundleDirectoryReady
                && report.runtimeBundleDirectoryReady
                && report.bootstrapBundleDirectoryReady
                && report.operatorTriageDirectoryReady;

            return report;
        }

        static void ensureDirectories(FailureBundleProtocolReport& report)
        {
            ensureDirectory(report.failureBundleRootDirectory, report.issues);
            ensureDirectory(report.startupBundleDirectory, report.issues);
            ensureDirectory(report.runtimeBundleDirectory, report.issues);
            ensureDirectory(report.bootstrapBundleDirectory, report.issues);
            ensureDirectory(report.operatorTriageDirectory, report.issues);

            report.failureBundleRootReady = directoryExists(report.failureBundleRootDirectory);
            report.startupBundleDirectoryReady = directoryExists(report.startupBundleDirectory);
            report.runtimeBundleDirectoryReady = directoryExists(report.runtimeBundleDirectory);
            report.bootstrapBundleDirectoryReady = directoryExists(report.bootstrapBundleDirectory);
            report.operatorTriageDirectoryReady = directoryExists(report.operatorTriageDirectory);

            report.failureBundleLaneReady =
                report.failureBundleRootReady
                && report.startupBundleDirectoryReady
                && report.runtimeBundleDirectoryReady
                && report.bootstrapBundleDirectoryReady
                && report.operatorTriageDirectoryReady;
        }

        [[nodiscard]] static const char* kindText(const FailureBundleKind kind)
        {
            switch (kind)
            {
            case FailureBundleKind::Startup: return "startup";
            case FailureBundleKind::Runtime: return "runtime";
            case FailureBundleKind::Bootstrap: return "bootstrap";
            case FailureBundleKind::OperatorTriage: return "operator-triage";
            default: return "unknown";
            }
        }

    private:
        [[nodiscard]] static bool directoryExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_directory(path, error);
        }

        static void ensureDirectory(const std::filesystem::path& path, std::vector<std::string>& issues)
        {
            std::error_code error;
            if (!std::filesystem::exists(path, error))
            {
                std::filesystem::create_directories(path, error);
            }

            if (error)
            {
                issues.push_back(std::string("Failed to create failure-bundle directory: ") + path.generic_string());
            }
        }
    };
}
