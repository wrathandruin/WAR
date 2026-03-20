#pragma once

#include <cstdint>
#include <string>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"

namespace war
{
    struct HeadlessHostOptions
    {
        uint32_t tickMilliseconds = 50;
        uint32_t heartbeatMilliseconds = 500;
        uint32_t runSeconds = 0;
    };

    class HeadlessHostMode
    {
    public:
        [[nodiscard]] static HeadlessHostOptions parseOptions(const std::wstring& commandLine);
        [[nodiscard]] static int run(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
            const HeadlessHostOptions& options);
    };
}
