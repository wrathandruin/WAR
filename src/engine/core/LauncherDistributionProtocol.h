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
    class LauncherDistributionProtocol
    {
    public:
        static void recordLauncherEntry(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool returningPlayerDetected)
        {
            const std::filesystem::path directory = launcherDirectory(runtimeBoundaryReport);
            std::error_code error;
            std::filesystem::create_directories(directory, error);
            if (error)
            {
                return;
            }

            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();
            const std::filesystem::path launcherManifestPath = resolveLauncherManifestPath(runtimeBoundaryReport);

            writeTextFile(
                directory / "launcher_session_manifest.txt",
                buildLauncherSessionManifest(
                    launcherManifestPath,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    returningPlayerDetected,
                    nowEpochMilliseconds));

            writeTextFile(
                directory / "latest_launcher_pointer.txt",
                buildLauncherPointer(
                    buildIdentity,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));
        }

        static void recordDistributionRuntime(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName)
        {
            const std::filesystem::path installerDirectoryPath = installerDirectory(runtimeBoundaryReport);
            const std::filesystem::path updaterDirectoryPath = updaterDirectory(runtimeBoundaryReport);

            std::error_code error;
            std::filesystem::create_directories(installerDirectoryPath, error);
            error.clear();
            std::filesystem::create_directories(updaterDirectoryPath, error);
            if (error)
            {
                return;
            }

            const uint64_t nowEpochMilliseconds = currentEpochMilliseconds();
            const std::filesystem::path installerManifestPath = resolveInstallerManifestPath(runtimeBoundaryReport);
            const std::filesystem::path updateManifestPath = resolveUpdateManifestPath(runtimeBoundaryReport);

            writeTextFile(
                installerDirectoryPath / "installer_runtime_manifest.txt",
                buildInstallerRuntimeManifest(
                    installerManifestPath,
                    component,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));

            writeTextFile(
                updaterDirectoryPath / "update_runtime_manifest.txt",
                buildUpdateRuntimeManifest(
                    updateManifestPath,
                    component,
                    buildIdentity,
                    buildChannel,
                    environmentName,
                    connectTargetName,
                    nowEpochMilliseconds));
        }

        [[nodiscard]] static std::filesystem::path launcherDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "Launcher";
        }

        [[nodiscard]] static std::filesystem::path installerDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "Installer";
        }

        [[nodiscard]] static std::filesystem::path updaterDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "Updater";
        }

        [[nodiscard]] static std::filesystem::path resolveLauncherManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "Launcher",
                "m50_launcher_manifest.txt");
        }

        [[nodiscard]] static std::filesystem::path resolveInstallerManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "Installer",
                "m50_installer_manifest.txt");
        }

        [[nodiscard]] static std::filesystem::path resolveUpdateManifestPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return SourceManifestLayout::resolveManifestPath(
                runtimeBoundaryReport,
                "Installer",
                "m50_update_channel_manifest.txt");
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

        [[nodiscard]] static std::string buildLauncherSessionManifest(
            const std::filesystem::path& launcherManifestPath,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            bool returningPlayerDetected,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "launcher_manifest_path=" << launcherManifestPath.generic_string() << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "returning_player_detected=" << (returningPlayerDetected ? "yes" : "no") << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildLauncherPointer(
            const std::string& buildIdentity,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildInstallerRuntimeManifest(
            const std::filesystem::path& installerManifestPath,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "installer_manifest_path=" << installerManifestPath.generic_string() << "\n"
                << "component=" << sanitizeSingleLine(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
                << "environment_name=" << sanitizeSingleLine(environmentName) << "\n"
                << "connect_target_name=" << sanitizeSingleLine(connectTargetName) << "\n"
                << "captured_at_epoch_ms=" << epochMilliseconds << "\n";
            return output.str();
        }

        [[nodiscard]] static std::string buildUpdateRuntimeManifest(
            const std::filesystem::path& updateManifestPath,
            const std::string& component,
            const std::string& buildIdentity,
            const std::string& buildChannel,
            const std::string& environmentName,
            const std::string& connectTargetName,
            uint64_t epochMilliseconds)
        {
            std::ostringstream output;
            output
                << "version=1\n"
                << "update_manifest_path=" << updateManifestPath.generic_string() << "\n"
                << "component=" << sanitizeSingleLine(component) << "\n"
                << "build_identity=" << sanitizeSingleLine(buildIdentity) << "\n"
                << "build_channel=" << sanitizeSingleLine(buildChannel) << "\n"
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
