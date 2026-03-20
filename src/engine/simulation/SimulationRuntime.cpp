#include "engine/simulation/SimulationRuntime.h"

#include <cstddef>
#include <string>

#include "engine/gameplay/Action.h"
#include "engine/gameplay/ActionSystem.h"

namespace war
{
    namespace
    {
        float clampAccumulator(float value)
        {
            constexpr float kFixedStepSeconds = 0.05f;
            const float maxAccumulator = kFixedStepSeconds * 6.0f;
            if (value < 0.0f)
            {
                return 0.0f;
            }

            if (value > maxAccumulator)
            {
                return maxAccumulator;
            }

            return value;
        }
    }

    void SimulationRuntime::initializeForLocalAuthority()
    {
        m_worldState.initializeTestWorld();
        m_actions = ActionQueue{};
        m_pendingIntents.clear();
        m_currentPath.clear();
        m_pathIndex = 0;
        m_eventLog.clear();
        m_accumulatorSeconds = 0.0f;
        m_nextIntentSequence = 1;
        m_hasMovementTarget = false;
        m_movementTargetTile = {};

        const TileCoord spawnTile{ 2, 2 };
        m_authoritativePlayerPosition = m_worldState.world().tileToWorldCenter(spawnTile);
        m_previousAuthoritativePlayerPosition = m_authoritativePlayerPosition;
        m_presentedPlayerPosition = m_authoritativePlayerPosition;

        m_diagnostics = SharedSimulationDiagnostics{};
        m_diagnostics.fixedStepSeconds = kFixedStepSeconds;
        m_diagnostics.localAuthorityActive = true;
        m_diagnostics.fixedStepEnabled = true;
    }

    uint64_t SimulationRuntime::enqueueIntent(SimulationIntentType type, TileCoord target)
    {
        SimulationIntent intent{};
        intent.sequence = m_nextIntentSequence++;
        intent.type = type;
        intent.target = target;
        m_pendingIntents.push_back(intent);

        ++m_diagnostics.intentsQueued;
        m_diagnostics.pendingIntentCount = m_pendingIntents.size();

        if (type == SimulationIntentType::MoveToTile)
        {
            m_hasMovementTarget = true;
            m_movementTargetTile = target;
        }

        appendEvent(
            std::string("Intent #")
            + std::to_string(intent.sequence)
            + " queued: "
            + simulationIntentTypeText(intent.type)
            + " -> ("
            + std::to_string(target.x)
            + ", "
            + std::to_string(target.y)
            + ")");

        return intent.sequence;
    }

    void SimulationRuntime::advanceFrame(float frameDeltaSeconds)
    {
        ++m_diagnostics.renderedFrames;

        m_accumulatorSeconds = clampAccumulator(m_accumulatorSeconds + frameDeltaSeconds);

        while (m_accumulatorSeconds >= kFixedStepSeconds)
        {
            m_previousAuthoritativePlayerPosition = m_authoritativePlayerPosition;

            processQueuedIntents();
            ActionSystem::processPending(
                m_worldState,
                m_actions,
                m_authoritativePlayerPosition,
                m_currentPath,
                m_pathIndex,
                m_eventLog);

            if (m_hasMovementTarget && (m_currentPath.empty() || m_pathIndex >= m_currentPath.size()) && m_pendingIntents.empty())
            {
                m_hasMovementTarget = false;
            }

            advanceAuthoritativePlayer(kFixedStepSeconds);

            m_accumulatorSeconds -= kFixedStepSeconds;
            ++m_diagnostics.simulationTicks;
        }

        m_diagnostics.accumulatorSeconds = m_accumulatorSeconds;
        m_diagnostics.presentationAlpha = kFixedStepSeconds > 0.0f
            ? m_accumulatorSeconds / kFixedStepSeconds
            : 0.0f;
        if (m_diagnostics.presentationAlpha < 0.0f)
        {
            m_diagnostics.presentationAlpha = 0.0f;
        }
        if (m_diagnostics.presentationAlpha > 1.0f)
        {
            m_diagnostics.presentationAlpha = 1.0f;
        }
        m_diagnostics.pendingIntentCount = m_pendingIntents.size();
        m_diagnostics.movementTargetActive = m_hasMovementTarget;
        refreshPresentedPlayerPosition();
    }

    void SimulationRuntime::appendEvent(const std::string& message)
    {
        m_eventLog.push_back(message);
        trimEventLog();
    }

    const WorldState& SimulationRuntime::worldState() const
    {
        return m_worldState;
    }

    WorldState& SimulationRuntime::worldState()
    {
        return m_worldState;
    }

    const Vec2& SimulationRuntime::authoritativePlayerPosition() const
    {
        return m_authoritativePlayerPosition;
    }

    const Vec2& SimulationRuntime::presentedPlayerPosition() const
    {
        return m_presentedPlayerPosition;
    }

    const std::vector<TileCoord>& SimulationRuntime::currentPath() const
    {
        return m_currentPath;
    }

    size_t SimulationRuntime::pathIndex() const
    {
        return m_pathIndex;
    }

    const std::vector<std::string>& SimulationRuntime::eventLog() const
    {
        return m_eventLog;
    }

    const SharedSimulationDiagnostics& SimulationRuntime::diagnostics() const
    {
        return m_diagnostics;
    }

    bool SimulationRuntime::hasMovementTarget() const
    {
        return m_hasMovementTarget;
    }

    TileCoord SimulationRuntime::movementTargetTile() const
    {
        return m_movementTargetTile;
    }

    void SimulationRuntime::processQueuedIntents()
    {
        while (!m_pendingIntents.empty())
        {
            const SimulationIntent intent = m_pendingIntents.front();
            m_pendingIntents.pop_front();

            Action action{};
            action.target = intent.target;
            switch (intent.type)
            {
            case SimulationIntentType::MoveToTile:
                action.type = ActionType::Move;
                break;
            case SimulationIntentType::InspectTile:
                action.type = ActionType::Inspect;
                break;
            case SimulationIntentType::InteractTile:
                action.type = ActionType::Interact;
                break;
            default:
                action.type = ActionType::Inspect;
                break;
            }

            m_actions.push(action);
            ++m_diagnostics.intentsProcessed;
            m_diagnostics.lastIntentSequence = intent.sequence;
        }
    }

    void SimulationRuntime::advanceAuthoritativePlayer(float stepSeconds)
    {
        if (m_currentPath.empty() || m_pathIndex >= m_currentPath.size())
        {
            if (m_hasMovementTarget && m_pendingIntents.empty())
            {
                m_hasMovementTarget = false;
            }
            return;
        }

        const Vec2 waypoint = m_worldState.world().tileToWorldCenter(m_currentPath[m_pathIndex]);
        const Vec2 toTarget = waypoint - m_authoritativePlayerPosition;
        const float distance = length(toTarget);

        if (distance < 1.0f)
        {
            m_authoritativePlayerPosition = waypoint;
            ++m_pathIndex;

            if (m_pathIndex >= m_currentPath.size())
            {
                m_hasMovementTarget = false;
                appendEvent("Path complete");
            }
            return;
        }

        const Vec2 direction = normalize(toTarget);
        const float step = kPlayerSpeedUnitsPerSecond * stepSeconds;

        if (step >= distance)
        {
            m_authoritativePlayerPosition = waypoint;
            ++m_pathIndex;

            if (m_pathIndex >= m_currentPath.size())
            {
                m_hasMovementTarget = false;
                appendEvent("Path complete");
            }
            return;
        }

        m_authoritativePlayerPosition += direction * step;
    }

    void SimulationRuntime::refreshPresentedPlayerPosition()
    {
        const float alpha = m_diagnostics.presentationAlpha;
        m_presentedPlayerPosition = m_previousAuthoritativePlayerPosition * (1.0f - alpha)
            + m_authoritativePlayerPosition * alpha;
    }

    void SimulationRuntime::trimEventLog()
    {
        constexpr size_t kMaxEvents = 10;
        if (m_eventLog.size() > kMaxEvents)
        {
            m_eventLog.erase(
                m_eventLog.begin(),
                m_eventLog.begin() + static_cast<std::ptrdiff_t>(m_eventLog.size() - kMaxEvents));
        }
    }
}
