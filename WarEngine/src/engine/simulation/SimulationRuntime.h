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
        void setAuthorityMode(bool localAuthorityActive, bool hostAuthorityActive, bool clientPredictionEnabled);
        void setReplicationHarnessState(
            bool latencyHarnessEnabled,
            uint32_t intentLatencyMilliseconds,
            uint32_t acknowledgementLatencyMilliseconds,
            uint32_t snapshotLatencyMilliseconds,
            uint32_t jitterMilliseconds,
            uint64_t snapshotAgeMilliseconds);
        void setPersistenceState(bool persistenceActive, const std::string& slotName);
        void notePersistenceSave(uint32_t schemaVersion, uint64_t saveEpochMilliseconds);
        void notePersistenceLoad(uint32_t loadedSchemaVersion, uint32_t migratedFromSchemaVersion, uint64_t loadEpochMilliseconds);
        void notePersistenceFailure(const std::string& error, bool duringLoad);

        [[nodiscard]] uint64_t enqueueIntent(SimulationIntentType type, TileCoord target);
        [[nodiscard]] SimulationIntentAck submitAuthoritativeIntent(const SimulationIntent& intent);

        void advanceFrame(float frameDeltaSeconds);
        void appendEvent(const std::string& message);
        void recordSnapshotReadFailure(const std::string& error);
        void clearSnapshotReadFailure();
        void applyAcknowledgement(const SimulationIntentAck& ack);
        [[nodiscard]] bool applyAuthoritativeSnapshot(
            const AuthoritativeWorldSnapshot& snapshot,
            uint64_t snapshotAgeMilliseconds,
            std::string& outCorrectionReason);
        [[nodiscard]] AuthoritativeWorldSnapshot buildAuthoritativeSnapshot(uint64_t lastProcessedIntentSequence) const;
        [[nodiscard]] bool loadPersistedState(
            const AuthoritativeWorldSnapshot& snapshot,
            std::string& outError,
            bool resetPresentationState = true,
            bool preserveLocalPlayerRuntime = false);

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
        static constexpr uint32_t kCombatRoundTicks = 120u;

        [[nodiscard]] SimulationIntentAck validateIntent(const SimulationIntent& intent) const;
        void queueAcceptedIntent(const SimulationIntent& intent);
        void processQueuedIntents(std::vector<SimulationIntent>& processedIntents);
        void advanceAuthoritativePlayer(float stepSeconds);
        void refreshPresentedPlayerPosition();
        void trimEventLog();
        void refreshDiagnosticsFromState();

        void initializeMissionState();
        void initializeShipRuntimeState();
        void initializeOrbitalRuntimeState();
        void applyMissionWorldState();
        void applyShipRuntimeWorldState();
        void updateOrbitalTravel();
        void beginOrbitalTransfer(OrbitalNodeId targetNode, OrbitalTravelPhase transferPhase, uint32_t transferTicks);
        void completeOrbitalTransfer();
        void setAuthoritativePlayerTile(TileCoord tile, bool clearMovementState = true);
        [[nodiscard]] bool isShipboardInteractionTarget(TileCoord target) const;
        void evaluateMissionProgressFromIntent(const SimulationIntent& intent);
        void handleMissionInspect(TileCoord target);
        void handleMissionInteraction(TileCoord target);
        void handleMissionEncounterVictory(const std::string& encounterLabel);

        void processSurvivalState(float stepSeconds);
        void evaluateEncounterTriggers();
        void startEncounterAtHotspot(const WorldAuthoringHotspot& hotspot);
        void updateCombatEncounter();
        void resolveCombatRound();
        void finishEncounterVictory();
        void finishEncounterDefeat();
        void tryAutoUseMedicalRecovery();
        void clearMovement();

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

        PlayerActorRuntimeState m_playerActorState{};
        CombatEncounterState m_combatEncounterState{};
        MissionRuntimeState m_missionRuntimeState{};
        ShipRuntimeState m_shipRuntimeState{};
        OrbitalRuntimeState m_orbitalRuntimeState{};
    };
}
