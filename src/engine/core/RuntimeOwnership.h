#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"

namespace war
{
    struct RuntimeOwnershipReport
    {
        bool runtimeRootResolved = false;
        bool runtimeRootSane = false;
        bool runtimeOwnedDirectoriesSane = false;
        bool deployableEnvironmentSeparated = false;
        bool requestedSaveSlotSanitized = false;
        bool primarySavePathOwned = false;
        bool ownershipValid = false;

        std::string requestedSaveSlotName = "primary";
        std::string primarySaveSlotName = "primary";

        std::filesystem::path primarySavePath;

        std::vector<std::string> issues;
    };

    class RuntimeOwnership
    {
    public:
        [[nodiscard]] static RuntimeOwnershipReport analyze(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& preferredSlotName = "primary")
        {
            RuntimeOwnershipReport report{};

            report.runtimeRootResolved = !runtimeBoundaryReport.runtimeRoot.empty();
            report.runtimeRootSane = report.runtimeRootResolved;
            if (!report.runtimeRootResolved)
            {
                report.issues.push_back("Runtime root is unresolved.");
                return report;
            }

            const std::string explicitSlot = environmentText("WAR_PERSISTENCE_SLOT");
            report.requestedSaveSlotName = !explicitSlot.empty()
                ? explicitSlot
                : preferredSlotName;
            report.primarySaveSlotName = sanitizeSaveSlotName(report.requestedSaveSlotName);
            report.requestedSaveSlotSanitized = report.primarySaveSlotName == sanitizeSaveSlotName(report.requestedSaveSlotName);

            report.runtimeOwnedDirectoriesSane =
                isPathWithin(runtimeBoundaryReport.configDirectory, runtimeBoundaryReport.runtimeRoot)
                && isPathWithin(runtimeBoundaryReport.logsDirectory, runtimeBoundaryReport.runtimeRoot)
                && isPathWithin(runtimeBoundaryReport.savesDirectory, runtimeBoundaryReport.runtimeRoot)
                && isPathWithin(runtimeBoundaryReport.crashDirectory, runtimeBoundaryReport.runtimeRoot)
                && isPathWithin(runtimeBoundaryReport.hostDirectory, runtimeBoundaryReport.runtimeRoot);

            report.deployableEnvironmentSeparated =
                runtimeBoundaryReport.environmentRoot.empty()
                || (!isPathWithin(runtimeBoundaryReport.environmentRoot, runtimeBoundaryReport.runtimeRoot)
                    && !isPathWithin(runtimeBoundaryReport.runtimeRoot, runtimeBoundaryReport.environmentRoot));

            report.primarySavePath =
                (runtimeBoundaryReport.savesDirectory
                    / ("authoritative_world_" + report.primarySaveSlotName + ".txt")).lexically_normal();
            report.primarySavePathOwned =
                isPathWithin(report.primarySavePath, runtimeBoundaryReport.savesDirectory)
                && isPathWithin(report.primarySavePath, runtimeBoundaryReport.runtimeRoot);

            if (!report.runtimeOwnedDirectoriesSane)
            {
                report.issues.push_back("Runtime-owned writable directories are not all contained under the runtime root.");
            }

            if (!report.deployableEnvironmentSeparated)
            {
                report.issues.push_back("Deployable Environment/ root overlaps with mutable runtime state.");
            }

            if (!report.primarySavePathOwned)
            {
                report.issues.push_back("Primary save path is not contained under Runtime/Saves.");
            }

            if (report.primarySaveSlotName.empty())
            {
                report.issues.push_back("Primary save slot name resolved empty after sanitization.");
            }

            report.ownershipValid =
                report.runtimeRootResolved
                && report.runtimeRootSane
                && report.runtimeOwnedDirectoriesSane
                && report.deployableEnvironmentSeparated
                && report.primarySavePathOwned
                && !report.primarySaveSlotName.empty();

            return report;
        }

        [[nodiscard]] static std::string diagnosticsSummary(const RuntimeOwnershipReport& report)
        {
            std::ostringstream output;
            output
                << "runtime_root_resolved=" << (report.runtimeRootResolved ? "yes" : "no")
                << " runtime_root_sane=" << (report.runtimeRootSane ? "yes" : "no")
                << " runtime_owned_directories_sane=" << (report.runtimeOwnedDirectoriesSane ? "yes" : "no")
                << " deployable_environment_separated=" << (report.deployableEnvironmentSeparated ? "yes" : "no")
                << " requested_save_slot=" << report.requestedSaveSlotName
                << " primary_save_slot=" << report.primarySaveSlotName
                << " primary_save_path_owned=" << (report.primarySavePathOwned ? "yes" : "no")
                << " ownership_valid=" << (report.ownershipValid ? "yes" : "no");
            return output.str();
        }

    private:
        [[nodiscard]] static std::string environmentText(const char* variableName)
        {
#if defined(_WIN32)
            char* value = nullptr;
            size_t length = 0;
            if (_dupenv_s(&value, &length, variableName) != 0 || value == nullptr)
            {
                return {};
            }

            std::string result(value);
            free(value);
            return result;
#else
            const char* value = std::getenv(variableName);
            return value != nullptr ? std::string(value) : std::string{};
#endif
        }

        [[nodiscard]] static bool isPathWithin(
            const std::filesystem::path& candidatePath,
            const std::filesystem::path& rootPath)
        {
            if (candidatePath.empty() || rootPath.empty())
            {
                return false;
            }

            const std::filesystem::path normalizedCandidate = candidatePath.lexically_normal();
            const std::filesystem::path normalizedRoot = rootPath.lexically_normal();

            auto candidateIt = normalizedCandidate.begin();
            auto rootIt = normalizedRoot.begin();

            for (; rootIt != normalizedRoot.end(); ++rootIt, ++candidateIt)
            {
                if (candidateIt == normalizedCandidate.end() || *candidateIt != *rootIt)
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] static std::string sanitizeSaveSlotName(std::string value)
        {
            if (value.empty())
            {
                return "primary";
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
                    || ch == '-')
                {
                    sanitized.push_back(static_cast<char>(std::tolower(raw)));
                }
                else if (std::isspace(raw) != 0)
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
                sanitized = "primary";
            }

            return sanitized;
        }
    };
}
