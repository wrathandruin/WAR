#include "engine/core/Application.h"

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/core/Timer.h"
#include "engine/host/HeadlessHostMode.h"
#include "game/GameLayer.h"
#include "platform/win32/Win32Window.h"

namespace war
{
    int Application::run(const std::wstring& commandLine)
    {
        if (commandLine.find(L"--headless-host") != std::wstring::npos)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnosticsReport localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(runtimeBoundaryReport);
            LocalDemoDiagnostics::writeStartupReport(runtimeBoundaryReport, localDemoDiagnosticsReport);

            const HeadlessHostOptions options = HeadlessHostMode::parseOptions(commandLine);
            return HeadlessHostMode::run(runtimeBoundaryReport, localDemoDiagnosticsReport, options);
        }

        Win32Window window;
        if (!window.create(1600, 900, L"WAR - Milestone 34"))
        {
            return -1;
        }

        GameLayer game;
        game.initialize(window);

        Timer timer;
        timer.reset();

        while (!window.shouldClose())
        {
            window.pollEvents();

            const float dt = timer.tick();
            game.update(dt);
            game.render();
        }

        game.shutdown();
        return 0;
    }
}
