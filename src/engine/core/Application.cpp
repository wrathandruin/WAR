#include "engine/core/Application.h"

#include <exception>
#include <string>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/core/Timer.h"
#include "game/GameLayer.h"
#include "platform/win32/Win32Window.h"

namespace war
{
    int Application::run(const std::wstring& commandLine)
    {
        try
        {
            (void)commandLine;

            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run entered");

            Win32Window window;
            if (!window.create(1600, 900, L"WAR - Milestone 44"))
            {
                LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run window.create failed");
                return -1;
            }

            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run window created");

            GameLayer game;
            game.initialize(window);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run game initialized");

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
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run completed cleanly");
            return 0;
        }
        catch (const std::exception& exception)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(
                runtimeBoundaryReport,
                "client_runtime_trace.txt",
                std::string("Application::run exception: ") + exception.what());
            return -2;
        }
        catch (...)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run unknown exception");
            return -3;
        }
    }
}
