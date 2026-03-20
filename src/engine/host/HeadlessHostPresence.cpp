#include "engine/host/HeadlessHostPresence.h"

#include <chrono>
#include <fstream>
#include <sstream>
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

        bool fileExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_regular_file(path, error);
        }

        uint64_t parseUnsigned(const StringMap& values, const char* key, uint64_t fallback)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return fallback;
            }

            try
            {
                return static_cast<uint64_t>(std::stoull(it->second));
            }
            catch (...)
            {
                return fallback;
            }
        }

        std::string parseString(const StringMap& values, const char* key, const char* fallback)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return fallback;
            }

            return it->second;
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
    }

    HeadlessHostPresenceReport HeadlessHostPresence::buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        HeadlessHostPresenceReport report{};
        report.statusFilePath = runtimeBoundaryReport.hostDirectory / "headless_host_status.txt";

        if (!fileExists(report.statusFilePath))
        {
            return report;
        }

        report.statusFilePresent = true;
        const StringMap values = parseKeyValueFile(report.statusFilePath);
        if (values.empty())
        {
            report.issues.push_back("Host status file exists but could not be parsed.");
            return report;
        }

        report.hostMode = parseString(values, "mode", "unknown");
        report.hostState = parseString(values, "state", "unknown");
        report.hostPid = parseString(values, "pid", "unknown");
        report.hostTickMilliseconds = static_cast<uint32_t>(parseUnsigned(values, "host_tick_ms", 0));
        report.advertisedSimulationTicks = parseUnsigned(values, "simulation_ticks", 0);
        report.authorityHostAdvertised = parseYesNo(values, "local_authority_active") && parseYesNo(values, "fixed_step_enabled");

        const uint64_t heartbeatEpochMilliseconds = parseUnsigned(values, "heartbeat_epoch_ms", 0);
        const uint64_t now = currentEpochMilliseconds();
        if (heartbeatEpochMilliseconds > 0 && now >= heartbeatEpochMilliseconds)
        {
            report.heartbeatAgeMilliseconds = now - heartbeatEpochMilliseconds;
        }

        report.heartbeatFresh = report.heartbeatAgeMilliseconds <= 2500ull && report.statusFilePresent;
        report.hostOnline = report.heartbeatFresh && report.hostState == "running";
        report.localBootstrapLaneReady = report.statusFilePresent && report.authorityHostAdvertised;

        if (report.statusFilePresent && !report.heartbeatFresh && report.hostState == "running")
        {
            report.issues.push_back("Headless host heartbeat is stale.");
        }

        if (report.statusFilePresent && !report.authorityHostAdvertised)
        {
            report.issues.push_back("Headless host status does not advertise fixed-step local authority.");
        }

        return report;
    }

    void HeadlessHostPresence::writeStatus(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const SharedSimulationDiagnostics& simulationDiagnostics,
        uint32_t hostTickMilliseconds,
        uint32_t processId,
        const std::string& hostState)
    {
        std::error_code error;
        std::filesystem::create_directories(runtimeBoundaryReport.hostDirectory, error);
        if (error)
        {
            return;
        }

        const std::filesystem::path path = runtimeBoundaryReport.hostDirectory / "headless_host_status.txt";
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            return;
        }

        output
            << "status_version=1\n"
            << "mode=headless-authoritative-bootstrap\n"
            << "state=" << hostState << "\n"
            << "pid=" << processId << "\n"
            << "host_tick_ms=" << hostTickMilliseconds << "\n"
            << "heartbeat_epoch_ms=" << currentEpochMilliseconds() << "\n"
            << "simulation_ticks=" << simulationDiagnostics.simulationTicks << "\n"
            << "local_authority_active=" << (simulationDiagnostics.localAuthorityActive ? "yes" : "no") << "\n"
            << "fixed_step_enabled=" << (simulationDiagnostics.fixedStepEnabled ? "yes" : "no") << "\n"
            << "fixed_step_seconds=" << simulationDiagnostics.fixedStepSeconds << "\n"
            << "intents_processed=" << simulationDiagnostics.intentsProcessed << "\n";
    }

    uint64_t HeadlessHostPresence::currentEpochMilliseconds()
    {
        using namespace std::chrono;
        return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    }
}
