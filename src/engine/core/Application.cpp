#include "engine/core/Application.h"

#include "engine/core/Timer.h"
#include "game/GameLayer.h"
#include "platform/win32/Win32Window.h"

namespace war
{
    int Application::run(const std::wstring& commandLine)
    {
        (void)commandLine;

        Win32Window window;
        if (!window.create(1600, 900, L"WAR - Milestone 38"))
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
