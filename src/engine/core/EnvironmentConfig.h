#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#include "engine/core/RuntimePaths.h"

namespace war
{
    struct EnvironmentConfigReport
    {
        bool profileFilePresent = false;
        bool profileParsed = false;
        bool secretsRequired = false;
        bool secretsFilePresent = false;
        bool requiredSecretsSatisfied = false;
        bool configurationValid = false;

        size_t requiredSecretCount = 0;
        size_t loadedSecretCount = 0;
        size_t missingRequiredSecretCount = 0;

        std::string environmentName = "local";
        std::string environmentProfileName = "local";
        std::string configIdentity = "local-default";
        std::string description = "unresolved";
        std::string connectTargetPolicy = "unresolved";
        std::string transportPolicy = "unresolved";
        std::string runtimeRootPolicy = "unresolved";
        std::string secretsSourceKind = "runtime-config-default";

        std::filesystem::path profileFilePath;
        std::filesystem::path secretsFilePath;

        std::vector<std::string> requiredSecretKeys;
        std::vector<std::string> missingRequiredSecretKeys;
        std::vector<std::string> issues;
    };

    class EnvironmentConfig
    {
    public:
        [[nodiscard]] static EnvironmentConfigReport load(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            EnvironmentConfigReport report{};
            report.environmentName = runtimeBoundaryReport.environmentName;
            report.environmentProfileName = runtimeBoundaryReport.environmentProfileName;
            report.profileFilePath = runtimeBoundaryReport.environmentProfileFile;

            if (runtimeBoundaryReport.environmentProfileResolved)
            {
                report.profileFilePresent = true;
            }
            else
            {
                report.issues.push_back(
                    std::string("Environment profile file missing: ")
                    + RuntimePaths::displayPath(runtimeBoundaryReport.environmentProfileFile));
                return report;
            }

            const KeyValueMap profileValues = parseKeyValueFile(runtimeBoundaryReport.environmentProfileFile);
            if (profileValues.empty())
            {
                report.issues.push_back(
                    std::string("Environment profile could not be parsed: ")
                    + RuntimePaths::displayPath(runtimeBoundaryReport.environmentProfileFile));
                return report;
            }

            report.profileParsed = true;
            report.configIdentity = valueOr(profileValues, "config_identity", report.environmentProfileName + "-default");
            report.description = valueOr(profileValues, "description", "unresolved");
            report.connectTargetPolicy = valueOr(profileValues, "connect_target_policy", "unresolved");
            report.transportPolicy = valueOr(profileValues, "transport_policy", "unresolved");
            report.runtimeRootPolicy = valueOr(profileValues, "runtime_root_policy", "unresolved");
            report.secretsRequired = valueOrBool(profileValues, "requires_secrets", false);

            const std::string requiredKeysText = valueOr(profileValues, "required_secret_keys", "");
            report.requiredSecretKeys = parseCsv(requiredKeysText);
            report.requiredSecretCount = report.requiredSecretKeys.size();

            if (!report.secretsRequired)
            {
                report.requiredSecretsSatisfied = true;
                report.configurationValid = true;
                return report;
            }

            const std::string explicitSecretsFile = environmentText("WAR_SECRETS_FILE");
            if (!explicitSecretsFile.empty())
            {
                report.secretsSourceKind = "environment-override";
                std::filesystem::path candidate = std::filesystem::path(explicitSecretsFile);
                if (candidate.is_relative())
                {
                    candidate = runtimeBoundaryReport.configDirectory / candidate;
                }
                report.secretsFilePath = candidate.lexically_normal();
            }
            else
            {
                report.secretsSourceKind = "runtime-config-default";
                const std::string defaultSecretsFilename =
                    valueOr(profileValues, "default_secrets_file", report.environmentProfileName + ".secrets.cfg");
                report.secretsFilePath = (runtimeBoundaryReport.configDirectory / defaultSecretsFilename).lexically_normal();
            }

            if (!fileExists(report.secretsFilePath))
            {
                report.issues.push_back(
                    std::string("Required secrets file missing: ")
                    + RuntimePaths::displayPath(report.secretsFilePath));
                report.missingRequiredSecretKeys = report.requiredSecretKeys;
                report.missingRequiredSecretCount = report.missingRequiredSecretKeys.size();
                return report;
            }

            report.secretsFilePresent = true;
            const KeyValueMap secretsValues = parseKeyValueFile(report.secretsFilePath);
            report.loadedSecretCount = secretsValues.size();

            for (const std::string& key : report.requiredSecretKeys)
            {
                const auto it = secretsValues.find(key);
                if (it == secretsValues.end() || it->second.empty())
                {
                    report.missingRequiredSecretKeys.push_back(key);
                }
            }

            report.missingRequiredSecretCount = report.missingRequiredSecretKeys.size();
            report.requiredSecretsSatisfied = report.missingRequiredSecretCount == 0;
            if (!report.requiredSecretsSatisfied)
            {
                report.issues.push_back(
                    std::string("Required secret keys missing from ")
                    + RuntimePaths::displayPath(report.secretsFilePath));
                return report;
            }

            report.configurationValid = true;
            return report;
        }

        [[nodiscard]] static std::string diagnosticsSummary(const EnvironmentConfigReport& report)
        {
            std::ostringstream output;
            output
                << "environment=" << report.environmentName
                << " profile=" << report.environmentProfileName
                << " config_identity=" << report.configIdentity
                << " secrets_required=" << (report.secretsRequired ? "yes" : "no")
                << " secrets_source=" << report.secretsSourceKind
                << " required_secret_count=" << report.requiredSecretCount
                << " missing_required_secret_count=" << report.missingRequiredSecretCount
                << " configuration_valid=" << (report.configurationValid ? "yes" : "no");
            return output.str();
        }

    private:
        using KeyValueMap = std::unordered_map<std::string, std::string>;

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

        [[nodiscard]] static bool fileExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_regular_file(path, error);
        }

        [[nodiscard]] static KeyValueMap parseKeyValueFile(const std::filesystem::path& path)
        {
            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                return {};
            }

            KeyValueMap values{};
            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                const std::string trimmed = trim(line);
                if (trimmed.empty() || trimmed[0] == '#')
                {
                    continue;
                }

                const size_t split = trimmed.find('=');
                if (split == std::string::npos)
                {
                    continue;
                }

                const std::string key = trim(trimmed.substr(0, split));
                const std::string value = trim(trimmed.substr(split + 1));
                if (!key.empty())
                {
                    values[key] = value;
                }
            }

            return values;
        }

        [[nodiscard]] static std::string trim(std::string value)
        {
            value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }));
            value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }).base(), value.end());
            return value;
        }

        [[nodiscard]] static std::string valueOr(
            const KeyValueMap& values,
            const std::string& key,
            const std::string& fallback)
        {
            const auto it = values.find(key);
            if (it == values.end() || it->second.empty())
            {
                return fallback;
            }
            return it->second;
        }

        [[nodiscard]] static bool valueOrBool(
            const KeyValueMap& values,
            const std::string& key,
            bool fallback)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return fallback;
            }

            std::string lower = it->second;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char ch)
            {
                return static_cast<char>(std::tolower(ch));
            });

            if (lower == "1" || lower == "true" || lower == "yes")
            {
                return true;
            }
            if (lower == "0" || lower == "false" || lower == "no")
            {
                return false;
            }

            return fallback;
        }

        [[nodiscard]] static std::vector<std::string> parseCsv(const std::string& text)
        {
            std::vector<std::string> values{};
            std::stringstream stream(text);
            std::string item;
            while (std::getline(stream, item, ','))
            {
                const std::string trimmed = trim(item);
                if (!trimmed.empty())
                {
                    values.push_back(trimmed);
                }
            }
            return values;
        }
    };
}
