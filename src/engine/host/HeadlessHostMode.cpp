#include "engine/host/HeadlessHostMode.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include <windows.h>

#include "engine/host/HeadlessHostPresence.h"
#include "engine/simulation/SimulationRuntime.h"

namespace war
{
    namespace
    {
        uint32_t parseOptionValue(const std::wstring& commandLine, const std::wstring& key, uint32_t fallback)
        {
            const size_t start = commandLine.find(key);
            if (start == std::wstring::npos)
            {
                return fallback;
            }

            size_t valueStart = start + key.size();
            size_t valueEnd = valueStart;
            while (valueEnd < commandLine.size() && commandLine[valueEnd] != L' ' && commandLine[valueEnd] != L'\t')
            {
                ++valueEnd;
            }

            try
            {
                return static_cast<uint32_t>(std::stoul(commandLine.substr(valueStart, valueEnd - valueStart)));
            }
            catch (...)
            {
                return fallback;
            }
        }

        void appendLogLine(const std::filesystem::path& path, const std::string& line)
        {
            std::ofstream output(path, std::ios::out | std::ios::app);
            if (!output.is_open())
            {
                return;
            }

            output << line << "\n";
        }
    }

    HeadlessHostOptions HeadlessHostMode::parseOptions(const std::wstring& commandLine)
    {
        HeadlessHostOptions options{};
        options.tickMilliseconds = std::max<uint32_t>(1u, parseOptionValue(commandLine, L"--host-tick-ms=", 50u));
        options.heartbeatMilliseconds = std::max<uint32_t>(50u, parseOptionValue(commandLine, L"--host-heartbeat-ms=", 500u));
        options.runSeconds = parseOptionValue(commandLine, L"--host-run-seconds=", 0u);
        return options;
    }

    int HeadlessHostMode::run(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
        const HeadlessHostOptions& options)
    {
        SimulationRuntime simulationRuntime{};
        simulationRuntime.initializeForLocalAuthority();
        simulationRuntime.appendEvent("Headless host bootstrap active");
        simulationRuntime.appendEvent("Separate host process running shared simulation runtime");
        simulationRuntime.appendEvent("Client intent protocol remains future work for M35");

        const std::filesystem::path logPath = runtimeBoundaryReport.logsDirectory / "headless_host_log.txt";
        const uint32_t processId = GetCurrentProcessId();

        appendLogLine(logPath, "WAR Headless Host Bootstrap");
        appendLogLine(logPath, std::string("PID: ") + std::to_string(processId));
        appendLogLine(logPath, std::string("Tick ms: ") + std::to_string(options.tickMilliseconds));
        appendLogLine(logPath, std::string("Runtime root: ") + RuntimePaths::displayPath(runtimeBoundaryReport.runtimeRoot));
        appendLogLine(logPath, std::string("Startup report: ") + RuntimePaths::displayPath(localDemoDiagnosticsReport.startupReportPath));
        appendLogLine(logPath, "State: running");

        const auto tickDuration = std::chrono::milliseconds(options.tickMilliseconds);
        const auto heartbeatDuration = std::chrono::milliseconds(options.heartbeatMilliseconds);
        const auto startedAt = std::chrono::steady_clock::now();
        auto nextTick = startedAt;
        auto nextHeartbeat = startedAt;

        for (;;)
        {
            const auto now = std::chrono::steady_clock::now();
            if (options.runSeconds > 0)
            {
                const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - startedAt).count();
                if (elapsedSeconds >= static_cast<long long>(options.runSeconds))
                {
                    break;
                }
            }

            if (now < nextTick)
            {
                std::this_thread::sleep_until(nextTick);
                continue;
            }

            simulationRuntime.advanceFrame(static_cast<float>(options.tickMilliseconds) / 1000.0f);
            nextTick += tickDuration;

            if (now >= nextHeartbeat)
            {
                HeadlessHostPresence::writeStatus(
                    runtimeBoundaryReport,
                    simulationRuntime.diagnostics(),
                    options.tickMilliseconds,
                    processId,
                    "running");
                nextHeartbeat = now + heartbeatDuration;
            }
        }

        HeadlessHostPresence::writeStatus(
            runtimeBoundaryReport,
            simulationRuntime.diagnostics(),
            options.tickMilliseconds,
            processId,
            "stopped");

        appendLogLine(logPath, std::string("Simulation ticks: ") + std::to_string(simulationRuntime.diagnostics().simulationTicks));
        appendLogLine(logPath, "State: stopped");
        return 0;
    }
}
