#include "game/Game.h"

#include "engine/core/Log.h"

namespace war
{
    void Game::initialize()
    {
        Log::info("Game::initialize()");
    }

    void Game::tick()
    {
        Log::info("Game::tick() - placeholder real-time world loop");
    }

    void Game::shutdown()
    {
        Log::info("Game::shutdown()");
    }
}
