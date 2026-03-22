#pragma once

#include <filesystem>
#include <system_error>

#include "engine/core/RuntimePaths.h"

namespace war
{
    class SourceManifestLayout
    {
    public:
        [[nodiscard]] static std::filesystem::path resolveManifestPath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const char* packagedLane,
            const char* manifestFileName)
        {
            const std::filesystem::path packagedPath =
                runtimeBoundaryReport.executableDirectory / packagedLane / manifestFileName;
            const std::filesystem::path sourceManifestPath =
                runtimeBoundaryReport.repoRoot / "SourceManifests" / packagedLane / manifestFileName;
            const std::filesystem::path legacySourcePath =
                runtimeBoundaryReport.repoRoot / packagedLane / manifestFileName;

            std::error_code error;
            if (std::filesystem::exists(packagedPath, error) && std::filesystem::is_regular_file(packagedPath, error))
            {
                return packagedPath;
            }

            error.clear();
            if (std::filesystem::exists(sourceManifestPath, error)
                && std::filesystem::is_regular_file(sourceManifestPath, error))
            {
                return sourceManifestPath;
            }

            error.clear();
            if (std::filesystem::exists(legacySourcePath, error)
                && std::filesystem::is_regular_file(legacySourcePath, error))
            {
                return legacySourcePath;
            }

            return packagedPath;
        }
    };
}
