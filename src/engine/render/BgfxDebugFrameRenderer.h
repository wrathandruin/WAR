#pragma once

#include <string>
#include <vector>

#include "engine/math/Vec2.h"
#include "engine/world/WorldState.h"

namespace war
{
    class BgfxDebugFrameRenderer
    {
    public:
        void render(
            const WorldState& worldState,
            const Vec2& playerPosition,
            const std::vector<std::string>& eventLog,
            float lastDeltaTime,
            const std::string& statusMessage) const;
    };
}
