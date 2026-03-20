#include "engine/core/Application.h"

#include <iostream>

#include "engine/core/Log.h"
#include "game/Game.h"

namespace war
{
    int Application::run()
    {
        Log::info("WAR bootstrap starting...");

        Game game;
        game.initialize();
        game.tick();
        game.shutdown();

        Log::info("WAR bootstrap shutting down.");
        return 0;
    }
}
