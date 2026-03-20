#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "engine/gameplay/ActionQueue.h"
#include "engine/math/Vec2.h"
#include "engine/simulation/SimulationIntent.h"
#include "engine/world/WorldState.h"

namespace war
{
    class SimulationRuntime
    {
    public:
        void initializeForLocalAuthority();

        [[nodiscard]] uint64_t enqueueIntent(SimulationIntentType type, TileCoord target);
        void advanceFrame(float frameDeltaSeconds);
        void appendEvent(const std::string& message);

        [[nodiscard]] const WorldState& worldState() const;
        [[nodiscard]] WorldState& worldState();

        [[nodiscard]] const Vec2& authoritativePlayerPosition() const;
        [[nodiscard]] const Vec2& presentedPlayerPosition() const;
        [[nodiscard]] const std::vector<TileCoord>& currentPath() const;
        [[nodiscard]] size_t pathIndex() const;
        [[nodiscard]] const std::vector<std::string>& eventLog() const;
        [[nodiscard]] const SharedSimulationDiagnostics& diagnostics() const;

        [[nodiscard]] bool hasMovementTarget() const;
        [[nodiscard]] TileCoord movementTargetTile() const;

    private:
        static constexpr float kFixedStepSeconds = 0.05f;
        static constexpr float kPlayerSpeedUnitsPerSecond = 210.0f;

        void processQueuedIntents();
        void advanceAuthoritativePlayer(float stepSeconds);
        void refreshPresentedPlayerPosition();
        void trimEventLog();

        WorldState m_worldState{};
        ActionQueue m_actions{};
        std::deque<SimulationIntent> m_pendingIntents{};
        std::vector<TileCoord> m_currentPath{};
        size_t m_pathIndex = 0;
        std::vector<std::string> m_eventLog{};

        Vec2 m_authoritativePlayerPosition{ 0.0f, 0.0f };
        Vec2 m_previousAuthoritativePlayerPosition{ 0.0f, 0.0f };
        Vec2 m_presentedPlayerPosition{ 0.0f, 0.0f };

        float m_accumulatorSeconds = 0.0f;
        uint64_t m_nextIntentSequence = 1;
        SharedSimulationDiagnostics m_diagnostics{};

        bool m_hasMovementTarget = false;
        TileCoord m_movementTargetTile{};
    };
}
