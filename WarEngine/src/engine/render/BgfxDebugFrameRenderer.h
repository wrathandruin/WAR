#pragma once

#include <string>
#include <vector>

#include "engine/math/Vec2.h"
#include "engine/simulation/SimulationIntent.h"
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
            const std::string& statusMessage,
            const SharedSimulationDiagnostics& simulationDiagnostics,
            const std::string& roomTitle,
            const std::string& promptLine,
            const std::string& commandBarText,
            const std::string& environmentIdentityText,
            const std::string& textPresentationProfile,
            uint32_t sameSpacePresenceCount,
            const std::string& sameSpacePresenceSummary,
            const std::string& reconnectRecoveryState,
            const std::string& reconnectContinuitySummary,
            uint32_t reconnectStaleSessionCount) const;
    };
}
