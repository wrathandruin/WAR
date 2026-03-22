#include "engine/host/ReplicationHarness.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <system_error>
#include <unordered_map>

namespace war
{
    namespace
    {
        using StringMap = std::unordered_map<std::string, std::string>;

        StringMap parseKeyValueFile(const std::filesystem::path& path)
        {
            StringMap values{};
            std::ifstream input(path, std::ios::in);
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

                values.emplace(line.substr(0, split), line.substr(split + 1));
            }

            return values;
        }

        bool parseYesNo(const StringMap& values, const char* key)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            return it->second == "yes" || it->second == "true" || it->second == "1";
        }

        uint32_t parseUnsigned(const StringMap& values, const char* key, uint32_t fallback)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return fallback;
            }

            try
            {
                return static_cast<uint32_t>(std::stoul(it->second));
            }
            catch (...)
            {
                return fallback;
            }
        }
    }

    std::filesystem::path ReplicationHarness::configPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        return runtimeBoundaryReport.configDirectory / "replication_harness.cfg";
    }

    ReplicationHarnessConfig ReplicationHarness::loadConfig(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        ReplicationHarnessConfig config{};
        const StringMap values = parseKeyValueFile(configPath(runtimeBoundaryReport));
        if (values.empty())
        {
            return config;
        }

        config.enabled = parseYesNo(values, "enabled");
        config.intentLatencyMilliseconds = parseUnsigned(values, "intent_latency_ms", 0);
        config.acknowledgementLatencyMilliseconds = parseUnsigned(values, "ack_latency_ms", 0);
        config.snapshotLatencyMilliseconds = parseUnsigned(values, "snapshot_latency_ms", 0);
        config.jitterMilliseconds = parseUnsigned(values, "jitter_ms", 0);
        return config;
    }

    bool ReplicationHarness::saveConfig(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const ReplicationHarnessConfig& config,
        std::string& outError)
    {
        outError.clear();

        std::error_code error;
        std::filesystem::create_directories(runtimeBoundaryReport.configDirectory, error);
        if (error)
        {
            outError = std::string("Failed to create replication harness config directory: ")
                + RuntimePaths::displayPath(runtimeBoundaryReport.configDirectory);
            return false;
        }

        const std::filesystem::path path = configPath(runtimeBoundaryReport);
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            outError = std::string("Failed to write replication harness config: ")
                + RuntimePaths::displayPath(path);
            return false;
        }

        output
            << "version=1\n"
            << "enabled=" << (config.enabled ? "yes" : "no") << "\n"
            << "intent_latency_ms=" << config.intentLatencyMilliseconds << "\n"
            << "ack_latency_ms=" << config.acknowledgementLatencyMilliseconds << "\n"
            << "snapshot_latency_ms=" << config.snapshotLatencyMilliseconds << "\n"
            << "jitter_ms=" << config.jitterMilliseconds << "\n";

        return true;
    }

    uint64_t ReplicationHarness::currentEpochMilliseconds()
    {
        using namespace std::chrono;
        return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    }

    uint64_t ReplicationHarness::computeArrivalEpochMilliseconds(
        uint64_t nowEpochMilliseconds,
        uint64_t sequenceBasis,
        uint32_t baseLatencyMilliseconds,
        uint32_t jitterMilliseconds)
    {
        int64_t totalDelayMilliseconds = static_cast<int64_t>(baseLatencyMilliseconds);

        if (jitterMilliseconds > 0)
        {
            const uint64_t jitterRange = static_cast<uint64_t>(jitterMilliseconds) * 2ull + 1ull;
            const uint64_t hashed = (sequenceBasis * 1103515245ull + 12345ull) % jitterRange;
            totalDelayMilliseconds += static_cast<int64_t>(hashed) - static_cast<int64_t>(jitterMilliseconds);
        }

        if (totalDelayMilliseconds < 0)
        {
            totalDelayMilliseconds = 0;
        }

        return nowEpochMilliseconds + static_cast<uint64_t>(totalDelayMilliseconds);
    }
}
