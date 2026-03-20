#include "engine/simulation/SimulationRuntime.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <utility>

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

        bool nearlyEqual(float lhs, float rhs, float epsilon = 0.5f)
        {
            return std::fabs(lhs - rhs) <= epsilon;
        }

        bool entityStateMatches(const Entity& entity, const ReplicatedEntityState& replicated)
        {
            return entity.id == replicated.id
                && entity.name == replicated.name
                && entity.type == replicated.type
                && entity.tile == replicated.tile
                && entity.isOpen == replicated.isOpen
                && entity.isLocked == replicated.isLocked
                && entity.isPowered == replicated.isPowered;
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
        m_diagnostics.fixedStepEnabled = true;
        m_diagnostics.localAuthorityActive = true;
        m_diagnostics.hostAuthorityActive = false;
        m_diagnostics.clientPredictionEnabled = false;
        m_diagnostics.lastSnapshotReadFailed = false;
        m_diagnostics.lastSnapshotReadError.clear();
        m_diagnostics.snapshotReadFailures = 0;
    }

    void SimulationRuntime::setAuthorityMode(bool localAuthorityActive, bool hostAuthorityActive, bool clientPredictionEnabled)
    {
        m_diagnostics.localAuthorityActive = localAuthorityActive;
        m_diagnostics.hostAuthorityActive = hostAuthorityActive;
        m_diagnostics.clientPredictionEnabled = clientPredictionEnabled;
    }

    void SimulationRuntime::setReplicationHarnessState(
        bool latencyHarnessEnabled,
        uint32_t intentLatencyMilliseconds,
        uint32_t acknowledgementLatencyMilliseconds,
        uint32_t snapshotLatencyMilliseconds,
        uint32_t jitterMilliseconds,
        uint64_t snapshotAgeMilliseconds)
    {
        m_diagnostics.latencyHarnessEnabled = latencyHarnessEnabled;
        m_diagnostics.intentLatencyMilliseconds = intentLatencyMilliseconds;
        m_diagnostics.acknowledgementLatencyMilliseconds = acknowledgementLatencyMilliseconds;
        m_diagnostics.snapshotLatencyMilliseconds = snapshotLatencyMilliseconds;
        m_diagnostics.jitterMilliseconds = jitterMilliseconds;
        m_diagnostics.lastSnapshotAgeMilliseconds = snapshotAgeMilliseconds;
    }

    uint64_t SimulationRuntime::enqueueIntent(SimulationIntentType type, TileCoord target)
    {
        SimulationIntent intent{};
        intent.sequence = m_nextIntentSequence++;
        intent.type = type;
        intent.target = target;

        queueAcceptedIntent(intent);
        return intent.sequence;
    }

    SimulationIntentAck SimulationRuntime::submitAuthoritativeIntent(const SimulationIntent& intent)
    {
        SimulationIntentAck ack = validateIntent(intent);
        if (ack.result == SimulationIntentAckResult::Accepted)
        {
            queueAcceptedIntent(intent);
        }
        else
        {
            ++m_diagnostics.intentsRejected;
            m_diagnostics.lastRejectedSequence = intent.sequence;

            appendEvent(
                std::string("Host rejected intent #")
                + std::to_string(intent.sequence)
                + " ["
                + simulationIntentTypeText(intent.type)
                + "]: "
                + ack.reason);
        }

        return ack;
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

    void SimulationRuntime::recordSnapshotReadFailure(const std::string& error)
    {
        m_diagnostics.lastSnapshotReadFailed = true;
        ++m_diagnostics.snapshotReadFailures;
        m_diagnostics.lastSnapshotReadError = error;
    }

    void SimulationRuntime::clearSnapshotReadFailure()
    {
        m_diagnostics.lastSnapshotReadFailed = false;
        m_diagnostics.lastSnapshotReadError.clear();
    }

    void SimulationRuntime::applyAcknowledgement(const SimulationIntentAck& ack)
    {
        if (ack.result == SimulationIntentAckResult::Accepted)
        {
            ++m_diagnostics.intentsAcknowledged;
            m_diagnostics.lastAcknowledgedSequence = ack.sequence;

            appendEvent(
                std::string("Host acknowledged intent #")
                + std::to_string(ack.sequence)
                + " ["
                + simulationIntentTypeText(ack.type)
                + "]");
            return;
        }

        if (ack.result == SimulationIntentAckResult::Rejected)
        {
            ++m_diagnostics.intentsRejected;
            m_diagnostics.lastRejectedSequence = ack.sequence;

            if (ack.type == SimulationIntentType::MoveToTile)
            {
                m_currentPath.clear();
                m_pathIndex = 0;
                m_hasMovementTarget = false;
            }

            appendEvent(
                std::string("Host rejected intent #")
                + std::to_string(ack.sequence)
                + " ["
                + simulationIntentTypeText(ack.type)
                + "]: "
                + ack.reason);
        }
    }

    bool SimulationRuntime::applyAuthoritativeSnapshot(
        const AuthoritativeWorldSnapshot& snapshot,
        uint64_t snapshotAgeMilliseconds,
        std::string& outCorrectionReason)
    {
        outCorrectionReason.clear();
        if (!snapshot.valid)
        {
            return false;
        }

        clearSnapshotReadFailure();

        bool corrected = false;
        bool playerCorrected = false;
        bool pathCorrected = false;
        bool entityCorrected = false;

        const float dx = m_authoritativePlayerPosition.x - snapshot.authoritativePlayerPosition.x;
        const float dy = m_authoritativePlayerPosition.y - snapshot.authoritativePlayerPosition.y;
        const float positionDistance = std::sqrt(dx * dx + dy * dy);
        m_diagnostics.lastPositionDivergenceDistance = positionDistance;
        m_diagnostics.lastSnapshotAgeMilliseconds = snapshotAgeMilliseconds;
        m_diagnostics.lastPathDivergence = false;
        m_diagnostics.lastEntityDivergence = false;

        if (!nearlyEqual(m_authoritativePlayerPosition.x, snapshot.authoritativePlayerPosition.x)
            || !nearlyEqual(m_authoritativePlayerPosition.y, snapshot.authoritativePlayerPosition.y))
        {
            corrected = true;
            playerCorrected = true;
        }

        if (m_hasMovementTarget != snapshot.movementTargetActive
            || (snapshot.movementTargetActive && m_movementTargetTile != snapshot.movementTargetTile)
            || m_currentPath.size() != snapshot.currentPath.size()
            || m_pathIndex != snapshot.pathIndex)
        {
            corrected = true;
            pathCorrected = true;
        }
        else
        {
            for (size_t i = 0; i < m_currentPath.size(); ++i)
            {
                if (m_currentPath[i] != snapshot.currentPath[i])
                {
                    corrected = true;
                    pathCorrected = true;
                    break;
                }
            }
        }

        const std::vector<Entity>& currentEntities = m_worldState.entities().all();
        if (currentEntities.size() != snapshot.entities.size())
        {
            corrected = true;
            entityCorrected = true;
        }
        else
        {
            for (size_t i = 0; i < currentEntities.size(); ++i)
            {
                if (!entityStateMatches(currentEntities[i], snapshot.entities[i]))
                {
                    corrected = true;
                    entityCorrected = true;
                    break;
                }
            }
        }

        m_authoritativePlayerPosition = snapshot.authoritativePlayerPosition;
        m_previousAuthoritativePlayerPosition = snapshot.authoritativePlayerPosition;
        m_presentedPlayerPosition = snapshot.authoritativePlayerPosition;
        m_currentPath = snapshot.currentPath;
        m_pathIndex = snapshot.pathIndex;
        m_hasMovementTarget = snapshot.movementTargetActive;
        m_movementTargetTile = snapshot.movementTargetTile;

        m_worldState.entities().clear();
        for (const ReplicatedEntityState& replicated : snapshot.entities)
        {
            Entity entity{};
            entity.id = replicated.id;
            entity.name = replicated.name;
            entity.type = replicated.type;
            entity.tile = replicated.tile;
            entity.isOpen = replicated.isOpen;
            entity.isLocked = replicated.isLocked;
            entity.isPowered = replicated.isPowered;
            m_worldState.entities().add(entity);
        }

        m_eventLog = snapshot.eventLog;
        trimEventLog();

        m_diagnostics.lastSnapshotSequence = snapshot.lastProcessedIntentSequence;
        m_diagnostics.lastSnapshotSimulationTicks = snapshot.simulationTicks;
        m_diagnostics.pendingIntentCount = m_pendingIntents.size();
        m_diagnostics.movementTargetActive = m_hasMovementTarget;
        m_diagnostics.lastPathDivergence = pathCorrected;
        m_diagnostics.lastEntityDivergence = entityCorrected;

        if (!corrected)
        {
            return false;
        }

        ++m_diagnostics.correctionsApplied;
        ++m_diagnostics.divergenceEvents;

        outCorrectionReason = "Host correction applied:";
        if (playerCorrected)
        {
            outCorrectionReason += " player";
        }
        if (pathCorrected)
        {
            outCorrectionReason += playerCorrected ? ", path" : " path";
        }
        if (entityCorrected)
        {
            outCorrectionReason += (playerCorrected || pathCorrected) ? ", entities" : " entities";
        }
        outCorrectionReason += " | drift=" + std::to_string(positionDistance);
        outCorrectionReason += " | snapshot age ms=" + std::to_string(snapshotAgeMilliseconds);

        return true;
    }

    AuthoritativeWorldSnapshot SimulationRuntime::buildAuthoritativeSnapshot(uint64_t lastProcessedIntentSequence) const
    {
        AuthoritativeWorldSnapshot snapshot{};
        snapshot.valid = true;
        snapshot.simulationTicks = m_diagnostics.simulationTicks;
        snapshot.lastProcessedIntentSequence = lastProcessedIntentSequence;
        snapshot.authoritativePlayerPosition = m_authoritativePlayerPosition;
        snapshot.movementTargetActive = m_hasMovementTarget;
        snapshot.movementTargetTile = m_movementTargetTile;
        snapshot.currentPath = m_currentPath;
        snapshot.pathIndex = m_pathIndex;
        snapshot.eventLog = m_eventLog;

        snapshot.entities.reserve(m_worldState.entities().all().size());
        for (const Entity& entity : m_worldState.entities().all())
        {
            ReplicatedEntityState replicated{};
            replicated.id = entity.id;
            replicated.name = entity.name;
            replicated.type = entity.type;
            replicated.tile = entity.tile;
            replicated.isOpen = entity.isOpen;
            replicated.isLocked = entity.isLocked;
            replicated.isPowered = entity.isPowered;
            snapshot.entities.push_back(replicated);
        }

        return snapshot;
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

    SimulationIntentAck SimulationRuntime::validateIntent(const SimulationIntent& intent) const
    {
        SimulationIntentAck ack{};
        ack.sequence = intent.sequence;
        ack.type = intent.type;
        ack.target = intent.target;
        ack.hostSimulationTicks = m_diagnostics.simulationTicks;

        switch (intent.type)
        {
        case SimulationIntentType::MoveToTile:
            if (!m_worldState.world().isInBounds(intent.target))
            {
                ack.result = SimulationIntentAckResult::Rejected;
                ack.reason = "target out of bounds";
                return ack;
            }

            if (m_worldState.world().isBlocked(intent.target))
            {
                ack.result = SimulationIntentAckResult::Rejected;
                ack.reason = "target tile blocked";
                return ack;
            }

            ack.result = SimulationIntentAckResult::Accepted;
            ack.reason = "path request accepted";
            return ack;

        case SimulationIntentType::InspectTile:
        case SimulationIntentType::InteractTile:
            if (!m_worldState.world().isInBounds(intent.target))
            {
                ack.result = SimulationIntentAckResult::Rejected;
                ack.reason = "target out of bounds";
                return ack;
            }

            ack.result = SimulationIntentAckResult::Accepted;
            ack.reason = "request accepted";
            return ack;

        default:
            ack.result = SimulationIntentAckResult::Rejected;
            ack.reason = "unknown intent type";
            return ack;
        }
    }

    void SimulationRuntime::queueAcceptedIntent(const SimulationIntent& intent)
    {
        m_pendingIntents.push_back(intent);

        ++m_diagnostics.intentsQueued;
        m_diagnostics.pendingIntentCount = m_pendingIntents.size();

        if (intent.type == SimulationIntentType::MoveToTile)
        {
            m_hasMovementTarget = true;
            m_movementTargetTile = intent.target;
        }

        appendEvent(
            std::string("Intent #")
            + std::to_string(intent.sequence)
            + " queued: "
            + simulationIntentTypeText(intent.type)
            + " -> ("
            + std::to_string(intent.target.x)
            + ", "
            + std::to_string(intent.target.y)
            + ")");
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
