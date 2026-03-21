#include "engine/core/Application.h"

#include <exception>
#include <string>

#include <windows.h>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/core/Timer.h"
#include "game/GameLayer.h"
#include "platform/win32/Win32Window.h"

namespace war
{
    namespace
    {
        std::wstring readEnvironmentWide(const wchar_t* variableName)
        {
            const DWORD requiredLength = GetEnvironmentVariableW(variableName, nullptr, 0);
            if (requiredLength == 0)
            {
                return {};
            }

            std::wstring buffer(static_cast<size_t>(requiredLength), L'\0');
            const DWORD writtenLength = GetEnvironmentVariableW(variableName, buffer.data(), requiredLength);
            if (writtenLength == 0)
            {
                return {};
            }

            buffer.resize(static_cast<size_t>(writtenLength));
            return buffer;
        }

        std::wstring windowTitleText()
        {
            const std::wstring connectTargetName = readEnvironmentWide(L"WAR_CONNECT_TARGET_NAME");
            if (connectTargetName.empty())
            {
                return L"WAR - Milestone 45";
            }

            return std::wstring(L"WAR - Milestone 45 [") + connectTargetName + L"]";
        }
    }

    int Application::run(const std::wstring& commandLine)
    {
        try
        {
            (void)commandLine;

            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run entered");

            Win32Window window;
            if (!window.create(1600, 900, windowTitleText().c_str()))
            {
                LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", "Application::run window.create failed");
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
        catch (const std::exception& exception)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", std::string("Application::run exception: ") + exception.what());
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
