#include "engine/simulation/SimulationRuntime.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "engine/core/LocalDemoDiagnostics.h"
#include "engine/core/RuntimePaths.h"
#include "engine/gameplay/Action.h"
#include "engine/gameplay/ActionSystem.h"

namespace war
{
    namespace
    {
        constexpr uint32_t kCurrentPersistenceSchemaVersion = 7u;
        constexpr std::string_view kTransitServiceTerminalLabel = "Transit Service Terminal";
        constexpr std::string_view kDiagnosticStationLabel = "Diagnostic Station";
        constexpr std::string_view kQuarantineControlTerminalLabel = "Quarantine Control Terminal";
        constexpr std::string_view kBridgeAccessChokepointLabel = "Bridge Access Chokepoint";
        constexpr std::string_view kQuarantineAccessGateLabel = "Quarantine Access Gate";
        constexpr std::string_view kTriageConvergenceLabel = "Triage Convergence";
        constexpr std::string_view kResponderShuttleLabel = "Responder Shuttle Khepri";
        constexpr std::string_view kShuttleHelmTerminalLabel = "Shuttle Helm Terminal";
        constexpr std::string_view kOrbitalNavigationConsoleLabel = "Orbital Navigation Console";
        constexpr std::string_view kDockedBoardingCollarLabel = "Docked Boarding Collar";
        constexpr std::string_view kFrontierRelayBeaconLabel = "Frontier Relay Beacon";
        constexpr std::string_view kDustFrontierLandingPadLabel = "Dust Frontier Landing Pad";

        constexpr TileCoord kHomeDockShipTile{ 6, 20 };
        constexpr TileCoord kHomeDockHelmTile{ 8, 20 };
        constexpr TileCoord kHomeDockNavTile{ 9, 20 };
        constexpr TileCoord kHomeDockPlayerTile{ 7, 21 };
        constexpr TileCoord kFrontierDockShipTile{ 44, 3 };
        constexpr TileCoord kFrontierDockHelmTile{ 45, 3 };
        constexpr TileCoord kFrontierDockNavTile{ 46, 3 };
        constexpr TileCoord kFrontierPadPlayerTile{ 43, 4 };

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

        float positionCorrectionToleranceUnits(const WorldGrid& world)
        {
            return (std::max)(8.0f, static_cast<float>(world.getTileSize()) * 0.35f);
        }

        bool pathContentsMatch(const std::vector<TileCoord>& lhs, const std::vector<TileCoord>& rhs)
        {
            if (lhs.size() != rhs.size())
            {
                return false;
            }

            for (size_t i = 0; i < lhs.size(); ++i)
            {
                if (lhs[i] != rhs[i])
                {
                    return false;
                }
            }

            return true;
        }

        int deterministicRoll(uint64_t seed, int minValue, int maxValue)
        {
            if (maxValue <= minValue)
            {
                return minValue;
            }

            const uint64_t range = static_cast<uint64_t>(maxValue - minValue + 1);
            return minValue + static_cast<int>(seed % range);
        }

        InventoryItemStack* findInventoryStack(PlayerActorRuntimeState& playerActorState, InventoryItemId id)
        {
            for (InventoryItemStack& stack : playerActorState.inventory)
            {
                if (stack.id == id)
                {
                    return &stack;
                }
            }

            return nullptr;
        }

        const InventoryItemStack* findInventoryStackConst(const PlayerActorRuntimeState& playerActorState, InventoryItemId id)
        {
            for (const InventoryItemStack& stack : playerActorState.inventory)
            {
                if (stack.id == id)
                {
                    return &stack;
                }
            }

            return nullptr;
        }

        bool consumeInventoryItem(PlayerActorRuntimeState& playerActorState, InventoryItemId id, uint32_t quantity)
        {
            InventoryItemStack* stack = findInventoryStack(playerActorState, id);
            if (stack == nullptr || stack->quantity < quantity)
            {
                return false;
            }

            stack->quantity -= quantity;
            if (stack->quantity == 0)
            {
                playerActorState.inventory.erase(
                    std::remove_if(
                        playerActorState.inventory.begin(),
                        playerActorState.inventory.end(),
                        [id](const InventoryItemStack& item) { return item.id == id; }),
                    playerActorState.inventory.end());
            }

            return true;
        }

        std::string inventorySummaryText(const PlayerActorRuntimeState& playerActorState)
        {
            if (playerActorState.inventory.empty())
            {
                return "empty";
            }

            std::ostringstream stream;
            for (size_t i = 0; i < playerActorState.inventory.size(); ++i)
            {
                if (i > 0)
                {
                    stream << ", ";
                }

                stream << inventoryItemText(playerActorState.inventory[i].id)
                       << " x" << playerActorState.inventory[i].quantity;
            }

            return stream.str();
        }

        uint32_t inventoryItemCount(const PlayerActorRuntimeState& playerActorState)
        {
            uint32_t count = 0;
            for (const InventoryItemStack& stack : playerActorState.inventory)
            {
                count += stack.quantity;
            }

            return count;
        }

        bool entityStateMatches(const Entity& entity, const ReplicatedEntityState& replicated)
        {
            return entity.id == replicated.id
                && entity.name == replicated.name
                && entity.type == replicated.type
                && entity.tile == replicated.tile
                && entity.isOpen == replicated.isOpen
                && entity.isLocked == replicated.isLocked
                && entity.isPowered == replicated.isPowered
                && entity.lootProfileId == replicated.lootProfileId
                && entity.lootClaimed == replicated.lootClaimed;
        }

        bool missionStateMatches(const MissionRuntimeState& lhs, const MissionRuntimeState& rhs)
        {
            return lhs.activeMission == rhs.activeMission
                && lhs.phase == rhs.phase
                && lhs.advancementCount == rhs.advancementCount
                && lhs.transitDataRecovered == rhs.transitDataRecovered
                && lhs.medlabDiagnosisComplete == rhs.medlabDiagnosisComplete
                && lhs.quarantineGateUnlocked == rhs.quarantineGateUnlocked
                && lhs.quarantineEncounterResolved == rhs.quarantineEncounterResolved
                && lhs.quarantineControlRestored == rhs.quarantineControlRestored
                && lhs.shipRuntimePrepReady == rhs.shipRuntimePrepReady
                && lhs.orbitalDepartureComplete == rhs.orbitalDepartureComplete
                && lhs.surveyOrbitReached == rhs.surveyOrbitReached
                && lhs.relayTrackStabilized == rhs.relayTrackStabilized
                && lhs.relayPlatformDocked == rhs.relayPlatformDocked
                && lhs.dustFrontierLanded == rhs.dustFrontierLanded
                && lhs.frontierRelaySecured == rhs.frontierRelaySecured
                && lhs.returnLaunchComplete == rhs.returnLaunchComplete
                && lhs.homeDockRestored == rhs.homeDockRestored
                && lhs.returnLoopComplete == rhs.returnLoopComplete
                && lhs.missionComplete == rhs.missionComplete
                && lhs.lastBeat == rhs.lastBeat;
        }

        bool shipStateMatches(const ShipRuntimeState& lhs, const ShipRuntimeState& rhs)
        {
            return lhs.activeShip == rhs.activeShip
                && lhs.ownershipState == rhs.ownershipState
                && lhs.occupancyState == rhs.occupancyState
                && lhs.boardingCount == rhs.boardingCount
                && lhs.boardingUnlocked == rhs.boardingUnlocked
                && lhs.docked == rhs.docked
                && lhs.playerBoarded == rhs.playerBoarded
                && lhs.powerOnline == rhs.powerOnline
                && lhs.airlockPressurized == rhs.airlockPressurized
                && lhs.commandClaimed == rhs.commandClaimed
                && lhs.launchPrepReady == rhs.launchPrepReady
                && lhs.frontierSurfaceAccessUnlocked == rhs.frontierSurfaceAccessUnlocked
                && lhs.frontierSurfaceActive == rhs.frontierSurfaceActive
                && lhs.shipName == rhs.shipName
                && lhs.locationLabel == rhs.locationLabel
                && lhs.lastBeat == rhs.lastBeat;
        }

        bool orbitalStateMatches(const OrbitalRuntimeState& lhs, const OrbitalRuntimeState& rhs)
        {
            return lhs.orbitalLayerUnlocked == rhs.orbitalLayerUnlocked
                && lhs.orbitalLayerActive == rhs.orbitalLayerActive
                && lhs.departureAuthorized == rhs.departureAuthorized
                && lhs.surveyOrbitReached == rhs.surveyOrbitReached
                && lhs.relayTrackReached == rhs.relayTrackReached
                && lhs.relayPlatformDocked == rhs.relayPlatformDocked
                && lhs.returnRouteAuthorized == rhs.returnRouteAuthorized
                && lhs.homeDockReached == rhs.homeDockReached
                && lhs.travelInProgress == rhs.travelInProgress
                && lhs.transferCount == rhs.transferCount
                && lhs.travelTicksRemaining == rhs.travelTicksRemaining
                && lhs.currentNode == rhs.currentNode
                && lhs.targetNode == rhs.targetNode
                && lhs.phase == rhs.phase
                && lhs.ruleText == rhs.ruleText
                && lhs.lastBeat == rhs.lastBeat;
        }

        void appendSimulationInitTrace(std::string_view line)
        {
            RuntimeBoundaryReport runtimeBoundaryReport = RuntimePaths::buildReport();
            RuntimePaths::ensureRuntimeDirectories(runtimeBoundaryReport);
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "simulation_init_trace.txt", line);
        }

        const WorldAuthoringHotspot* findHotspotByLabel(const WorldState& worldState, std::string_view label)
        {
            for (const WorldAuthoringHotspot& hotspot : worldState.authoringHotspots())
            {
                if (hotspot.label == label)
                {
                    return &hotspot;
                }
            }

            return nullptr;
        }

        WorldAuthoringHotspot* findHotspotByLabelMutable(WorldState& worldState, std::string_view label)
        {
            for (const WorldAuthoringHotspot& hotspot : worldState.authoringHotspots())
            {
                if (hotspot.label == label)
                {
                    return worldState.authoringHotspotAtMutable(hotspot.tile);
                }
            }

            return nullptr;
        }

        Entity* findEntityByLabelMutable(WorldState& worldState, std::string_view label)
        {
            for (const Entity& entity : worldState.entities().all())
            {
                if (entity.name == label)
                {
                    return worldState.entities().getAt(entity.tile);
                }
            }

            return nullptr;
        }

        std::string missionObjectiveText(const MissionRuntimeState& missionState)
        {
            switch (missionState.phase)
            {
            case MissionPhase::RecoverTransitData:
                return "Reach the Transit Service Terminal and recover route data.";
            case MissionPhase::DiagnoseMedlab:
                return "Inspect the Diagnostic Station in the MedLab to identify the breach route.";
            case MissionPhase::SecureQuarantineGate:
                return "Push through the unlocked quarantine lane and secure the Quarantine Access Gate.";
            case MissionPhase::RestoreQuarantineControl:
                return "Use the Quarantine Control Terminal to restore the evac corridor.";
            case MissionPhase::BoardResponderShuttle:
                return "Return to the cargo bay and board Responder Shuttle Khepri.";
            case MissionPhase::ClaimShuttleCommand:
                return "Use Shuttle Helm Terminal to claim command authority over the docked shuttle.";
            case MissionPhase::EnterOrbitalLane:
                return "Use Shuttle Helm Terminal to clear the docking collar and enter the local orbital traffic lane.";
            case MissionPhase::ReachSurveyOrbit:
                return "Use Orbital Navigation Console to plot transfer to Debris Survey Orbit.";
            case MissionPhase::StabilizeRelayTrack:
                return "Use Orbital Navigation Console to stabilize the relay holding track.";
            case MissionPhase::DockRelayPlatform:
                return "Use Orbital Navigation Console to dock with Dust Frontier Relay Platform.";
            case MissionPhase::LandDustFrontier:
                return "Interact with Responder Shuttle Khepri to disembark onto Dust Frontier Landing Pad.";
            case MissionPhase::SecureFrontierRelay:
                return "Use Frontier Relay Beacon to secure the surface relay handoff.";
            case MissionPhase::ReturnToShuttle:
                return "Re-board Responder Shuttle Khepri and use Shuttle Helm Terminal to launch back toward Khepri Dock.";
            case MissionPhase::ReturnToHomeDock:
                return "Disembark from Responder Shuttle Khepri at Khepri Dock to complete the return loop.";
            case MissionPhase::MissionComplete:
                return "Docking, landing, and return-loop continuity are stable. M45 internal-alpha packaging is unblocked.";
            default:
                return "No active mission objective.";
            }
        }
    }

    void SimulationRuntime::initializeForLocalAuthority()
    {
        appendSimulationInitTrace("SimulationRuntime::initializeForLocalAuthority entered");
        m_worldState.initializeTestWorld();
        appendSimulationInitTrace("SimulationRuntime::initializeForLocalAuthority world initialized");
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

        m_playerActorState = PlayerActorRuntimeState{};
        m_playerActorState.inventory.push_back({ InventoryItemId::RationPack, 1 });
        m_playerActorState.equipment.suit = InventoryItemId::UtilitySuit;
        appendSimulationInitTrace("SimulationRuntime::initializeForLocalAuthority actor state initialized");

        m_combatEncounterState = CombatEncounterState{};
        initializeMissionState();
        initializeShipRuntimeState();
        initializeOrbitalRuntimeState();
        m_diagnostics = SharedSimulationDiagnostics{};
        m_diagnostics.currentHazardLabel = "none";
        m_diagnostics.currentTerrainConsequence = "stable";
        m_diagnostics.fixedStepSeconds = kFixedStepSeconds;
        m_diagnostics.fixedStepEnabled = true;
        m_diagnostics.localAuthorityActive = true;
        m_diagnostics.hostAuthorityActive = false;
        m_diagnostics.clientPredictionEnabled = false;
        m_diagnostics.lastSnapshotReadFailed = false;
        m_diagnostics.lastSnapshotReadError.clear();
        m_diagnostics.snapshotReadFailures = 0;
        m_diagnostics.persistenceActive = false;
        m_diagnostics.persistenceDataLoaded = false;
        m_diagnostics.persistenceMigrationApplied = false;
        m_diagnostics.lastPersistenceSaveSucceeded = false;
        m_diagnostics.lastPersistenceLoadSucceeded = false;
        m_diagnostics.persistenceSchemaVersion = kCurrentPersistenceSchemaVersion;
        m_diagnostics.persistenceLoadedSchemaVersion = 0;
        m_diagnostics.persistenceMigratedFromSchemaVersion = 0;
        m_diagnostics.persistenceSlotName = "primary";
        m_diagnostics.lastPersistenceError.clear();

        appendEvent("Milestone 44 initialized");
        appendEvent("docking / landing / cross-layer transition persistence / return loop active");
        appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
        refreshDiagnosticsFromState();
        appendSimulationInitTrace("SimulationRuntime::initializeForLocalAuthority completed");
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

    void SimulationRuntime::setPersistenceState(bool persistenceActive, const std::string& slotName)
    {
        m_diagnostics.persistenceActive = persistenceActive;
        m_diagnostics.persistenceSlotName = slotName.empty() ? "primary" : slotName;
    }

    void SimulationRuntime::notePersistenceSave(uint32_t schemaVersion, uint64_t saveEpochMilliseconds)
    {
        m_diagnostics.persistenceSchemaVersion = schemaVersion;
        m_diagnostics.lastPersistenceSaveSucceeded = true;
        ++m_diagnostics.persistenceSaveCount;
        m_diagnostics.lastPersistenceSaveEpochMilliseconds = saveEpochMilliseconds;
        m_diagnostics.lastPersistenceError.clear();
    }

    void SimulationRuntime::notePersistenceLoad(
        uint32_t loadedSchemaVersion,
        uint32_t migratedFromSchemaVersion,
        uint64_t loadEpochMilliseconds)
    {
        m_diagnostics.persistenceLoadedSchemaVersion = loadedSchemaVersion;
        m_diagnostics.persistenceMigratedFromSchemaVersion = migratedFromSchemaVersion;
        m_diagnostics.persistenceMigrationApplied = migratedFromSchemaVersion > 0;
        m_diagnostics.persistenceDataLoaded = true;
        m_diagnostics.lastPersistenceLoadSucceeded = true;
        ++m_diagnostics.persistenceLoadCount;
        m_diagnostics.lastPersistenceLoadEpochMilliseconds = loadEpochMilliseconds;
        m_diagnostics.lastPersistenceError.clear();
    }

    void SimulationRuntime::notePersistenceFailure(const std::string& error, bool duringLoad)
    {
        if (duringLoad)
        {
            m_diagnostics.lastPersistenceLoadSucceeded = false;
        }
        else
        {
            m_diagnostics.lastPersistenceSaveSucceeded = false;
        }

        if (!error.empty())
        {
            m_diagnostics.lastPersistenceError = error;
        }
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

            std::vector<SimulationIntent> processedIntents{};
            processQueuedIntents(processedIntents);
            ActionSystem::processPending(
                m_worldState,
                m_actions,
                m_playerActorState,
                m_authoritativePlayerPosition,
                m_currentPath,
                m_pathIndex,
                m_eventLog);

            for (const SimulationIntent& intent : processedIntents)
            {
                evaluateMissionProgressFromIntent(intent);
            }

            if (!m_combatEncounterState.active)
            {
                advanceAuthoritativePlayer(kFixedStepSeconds);
            }

            processSurvivalState(kFixedStepSeconds);
            evaluateEncounterTriggers();
            updateCombatEncounter();
            updateOrbitalTravel();

            if (m_hasMovementTarget
                && (m_currentPath.empty() || m_pathIndex >= m_currentPath.size())
                && m_pendingIntents.empty())
            {
                m_hasMovementTarget = false;
            }

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
        refreshDiagnosticsFromState();
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
                clearMovement();
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

        const bool preserveLocalPlayerRuntime =
            m_diagnostics.hostAuthorityActive && m_diagnostics.clientPredictionEnabled;

        bool corrected = false;
        bool playerCorrected = false;
        bool pathCorrected = false;
        bool entityCorrected = false;
        bool actorCorrected = false;
        bool combatCorrected = false;
        bool missionCorrected = false;
        bool shipCorrected = false;
        bool orbitalCorrected = false;

        const float dx = m_authoritativePlayerPosition.x - snapshot.authoritativePlayerPosition.x;
        const float dy = m_authoritativePlayerPosition.y - snapshot.authoritativePlayerPosition.y;
        const float positionDistance = std::sqrt(dx * dx + dy * dy);
        const float correctionTolerance = positionCorrectionToleranceUnits(m_worldState.world());
        m_diagnostics.lastPositionDivergenceDistance = positionDistance;
        m_diagnostics.lastSnapshotAgeMilliseconds = snapshotAgeMilliseconds;

        if (!preserveLocalPlayerRuntime && positionDistance > correctionTolerance)
        {
            corrected = true;
            playerCorrected = true;
        }

        const bool pathTopologyMatches =
            m_hasMovementTarget == snapshot.movementTargetActive
            && (!snapshot.movementTargetActive || m_movementTargetTile == snapshot.movementTargetTile)
            && pathContentsMatch(m_currentPath, snapshot.currentPath);
        const size_t pathIndexDelta = m_pathIndex > snapshot.pathIndex
            ? (m_pathIndex - snapshot.pathIndex)
            : (snapshot.pathIndex - m_pathIndex);
        const bool pathCorrectionRequired = !pathTopologyMatches || pathIndexDelta > 1u;
        if (!preserveLocalPlayerRuntime && pathCorrectionRequired)
        {
            corrected = true;
            pathCorrected = true;
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

        if (!preserveLocalPlayerRuntime
            && (m_playerActorState.health != snapshot.playerActorState.health
            || m_playerActorState.armor != snapshot.playerActorState.armor
            || m_playerActorState.suitIntegrity != snapshot.playerActorState.suitIntegrity
            || !nearlyEqual(m_playerActorState.oxygenSecondsRemaining, snapshot.playerActorState.oxygenSecondsRemaining, 0.01f)
            || m_playerActorState.radiationDose != snapshot.playerActorState.radiationDose
            || m_playerActorState.toxicExposure != snapshot.playerActorState.toxicExposure
            || m_playerActorState.inventory.size() != snapshot.playerActorState.inventory.size()
            || m_playerActorState.equipment.weapon != snapshot.playerActorState.equipment.weapon
            || m_playerActorState.equipment.suit != snapshot.playerActorState.equipment.suit
            || m_playerActorState.equipment.tool != snapshot.playerActorState.equipment.tool))
        {
            corrected = true;
            actorCorrected = true;
        }

        if (!preserveLocalPlayerRuntime
            && (m_combatEncounterState.active != snapshot.combatEncounterState.active
            || m_combatEncounterState.roundNumber != snapshot.combatEncounterState.roundNumber
            || m_combatEncounterState.roundTicksRemaining != snapshot.combatEncounterState.roundTicksRemaining
            || m_combatEncounterState.hostileHealth != snapshot.combatEncounterState.hostileHealth
            || m_combatEncounterState.label != snapshot.combatEncounterState.label))
        {
            corrected = true;
            combatCorrected = true;
        }

        if (!missionStateMatches(m_missionRuntimeState, snapshot.missionRuntimeState))
        {
            corrected = true;
            missionCorrected = true;
        }

        if (!shipStateMatches(m_shipRuntimeState, snapshot.shipRuntimeState))
        {
            corrected = true;
            shipCorrected = true;
        }

        if (!orbitalStateMatches(m_orbitalRuntimeState, snapshot.orbitalRuntimeState))
        {
            corrected = true;
            orbitalCorrected = true;
        }

        m_diagnostics.lastSnapshotSequence = snapshot.lastProcessedIntentSequence;
        m_diagnostics.lastSnapshotSimulationTicks = snapshot.simulationTicks;
        m_diagnostics.lastPathDivergence = pathCorrected;
        m_diagnostics.lastEntityDivergence = entityCorrected;
        m_diagnostics.hazardTicks = snapshot.hazardTicks;
        m_diagnostics.terrainConsequenceEvents = snapshot.terrainConsequenceEvents;
        m_diagnostics.persistenceSchemaVersion = snapshot.persistenceSchemaVersion > 0
            ? snapshot.persistenceSchemaVersion
            : kCurrentPersistenceSchemaVersion;
        m_diagnostics.persistenceLoadedSchemaVersion = snapshot.persistenceLoadedSchemaVersion;
        m_diagnostics.persistenceMigratedFromSchemaVersion = snapshot.persistenceMigratedFromSchemaVersion;
        m_diagnostics.persistenceMigrationApplied = snapshot.persistenceMigratedFromSchemaVersion > 0;
        m_diagnostics.persistenceSlotName = snapshot.persistenceSlotName.empty()
            ? std::string("primary")
            : snapshot.persistenceSlotName;
        m_diagnostics.currentHazardLabel = snapshot.currentHazardLabel;
        m_diagnostics.currentTerrainConsequence = snapshot.currentTerrainConsequence;
        const uint64_t recoveredNextIntentSequence = snapshot.nextIntentSequence > 0
            ? snapshot.nextIntentSequence
            : snapshot.lastProcessedIntentSequence + 1ull;
        m_nextIntentSequence = (std::max)(m_nextIntentSequence, recoveredNextIntentSequence);

        if (!corrected)
        {
            return false;
        }

        std::string loadError;
        if (!loadPersistedState(snapshot, loadError, false, preserveLocalPlayerRuntime))
        {
            outCorrectionReason = loadError;
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
        if (actorCorrected)
        {
            outCorrectionReason += (playerCorrected || pathCorrected || entityCorrected) ? ", actor" : " actor";
        }
        if (combatCorrected)
        {
            outCorrectionReason += ", combat";
        }
        if (missionCorrected)
        {
            outCorrectionReason += ", mission";
        }
        if (shipCorrected)
        {
            outCorrectionReason += ", ship";
        }
        if (orbitalCorrected)
        {
            outCorrectionReason += ", orbital";
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
        snapshot.nextIntentSequence = m_nextIntentSequence;
        snapshot.authoritativePlayerPosition = m_authoritativePlayerPosition;
        snapshot.movementTargetActive = m_hasMovementTarget;
        snapshot.movementTargetTile = m_movementTargetTile;
        snapshot.currentPath = m_currentPath;
        snapshot.pathIndex = m_pathIndex;
        snapshot.eventLog = m_eventLog;
        snapshot.playerActorState = m_playerActorState;
        snapshot.combatEncounterState = m_combatEncounterState;
        snapshot.missionRuntimeState = m_missionRuntimeState;
        snapshot.shipRuntimeState = m_shipRuntimeState;
        snapshot.orbitalRuntimeState = m_orbitalRuntimeState;
        snapshot.hazardTicks = m_diagnostics.hazardTicks;
        snapshot.terrainConsequenceEvents = m_diagnostics.terrainConsequenceEvents;
        snapshot.persistenceSchemaVersion = m_diagnostics.persistenceSchemaVersion;
        snapshot.persistenceLoadedSchemaVersion = m_diagnostics.persistenceLoadedSchemaVersion;
        snapshot.persistenceMigratedFromSchemaVersion = m_diagnostics.persistenceMigratedFromSchemaVersion;
        snapshot.persistenceEpochMilliseconds = m_diagnostics.lastPersistenceSaveEpochMilliseconds;
        snapshot.persistenceSlotName = m_diagnostics.persistenceSlotName;
        snapshot.currentHazardLabel = m_diagnostics.currentHazardLabel;
        snapshot.currentTerrainConsequence = m_diagnostics.currentTerrainConsequence;

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
            replicated.lootProfileId = entity.lootProfileId;
            replicated.lootClaimed = entity.lootClaimed;
            snapshot.entities.push_back(replicated);
        }

        return snapshot;
    }

    bool SimulationRuntime::loadPersistedState(
        const AuthoritativeWorldSnapshot& snapshot,
        std::string& outError,
        bool resetPresentationState,
        bool preserveLocalPlayerRuntime)
    {
        outError.clear();
        if (!snapshot.valid)
        {
            outError = "Persisted snapshot is invalid.";
            return false;
        }

        const Vec2 priorAuthoritativePlayerPosition = m_authoritativePlayerPosition;
        const Vec2 priorPresentedPlayerPosition = m_presentedPlayerPosition;
        const Vec2 priorPreviousAuthoritativePlayerPosition = m_previousAuthoritativePlayerPosition;
        const std::vector<TileCoord> priorCurrentPath = m_currentPath;
        const size_t priorPathIndex = m_pathIndex;
        const bool priorHasMovementTarget = m_hasMovementTarget;
        const TileCoord priorMovementTargetTile = m_movementTargetTile;
        const PlayerActorRuntimeState priorPlayerActorState = m_playerActorState;
        const CombatEncounterState priorCombatEncounterState = m_combatEncounterState;

        if (!preserveLocalPlayerRuntime)
        {
            m_authoritativePlayerPosition = snapshot.authoritativePlayerPosition;
        }

        if (resetPresentationState && !preserveLocalPlayerRuntime)
        {
            m_previousAuthoritativePlayerPosition = snapshot.authoritativePlayerPosition;
            m_presentedPlayerPosition = snapshot.authoritativePlayerPosition;
        }
        else if (!preserveLocalPlayerRuntime)
        {
            m_previousAuthoritativePlayerPosition = priorAuthoritativePlayerPosition;
        }

        if (!preserveLocalPlayerRuntime)
        {
            m_currentPath = snapshot.currentPath;
            m_pathIndex = snapshot.pathIndex;
            m_hasMovementTarget = snapshot.movementTargetActive;
            m_movementTargetTile = snapshot.movementTargetTile;
            m_playerActorState = snapshot.playerActorState;
            m_combatEncounterState = snapshot.combatEncounterState;
        }
        else
        {
            m_authoritativePlayerPosition = priorAuthoritativePlayerPosition;
            m_previousAuthoritativePlayerPosition = priorPreviousAuthoritativePlayerPosition;
            m_presentedPlayerPosition = priorPresentedPlayerPosition;
            m_currentPath = priorCurrentPath;
            m_pathIndex = priorPathIndex;
            m_hasMovementTarget = priorHasMovementTarget;
            m_movementTargetTile = priorMovementTargetTile;
            m_playerActorState = priorPlayerActorState;
            m_combatEncounterState = priorCombatEncounterState;
        }

        m_missionRuntimeState = snapshot.missionRuntimeState;
        m_shipRuntimeState = snapshot.shipRuntimeState;
        m_orbitalRuntimeState = snapshot.orbitalRuntimeState;
        m_eventLog = snapshot.eventLog;
        trimEventLog();

        m_worldState.initializeTestWorld();
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
            entity.lootProfileId = replicated.lootProfileId;
            entity.lootClaimed = replicated.lootClaimed;
            m_worldState.entities().add(entity);
        }

        m_diagnostics.simulationTicks = snapshot.simulationTicks;
        m_diagnostics.lastIntentSequence = snapshot.lastProcessedIntentSequence;
        m_diagnostics.lastSnapshotSequence = snapshot.lastProcessedIntentSequence;
        m_diagnostics.lastSnapshotSimulationTicks = snapshot.simulationTicks;
        m_diagnostics.hazardTicks = snapshot.hazardTicks;
        m_diagnostics.terrainConsequenceEvents = snapshot.terrainConsequenceEvents;
        m_diagnostics.persistenceSchemaVersion = snapshot.persistenceSchemaVersion > 0
            ? snapshot.persistenceSchemaVersion
            : kCurrentPersistenceSchemaVersion;
        m_diagnostics.persistenceLoadedSchemaVersion = snapshot.persistenceLoadedSchemaVersion;
        m_diagnostics.persistenceMigratedFromSchemaVersion = snapshot.persistenceMigratedFromSchemaVersion;
        m_diagnostics.persistenceMigrationApplied = snapshot.persistenceMigratedFromSchemaVersion > 0;
        m_diagnostics.persistenceSlotName = snapshot.persistenceSlotName.empty()
            ? std::string("primary")
            : snapshot.persistenceSlotName;
        m_diagnostics.currentHazardLabel = snapshot.currentHazardLabel;
        m_diagnostics.currentTerrainConsequence = snapshot.currentTerrainConsequence;
        const uint64_t recoveredNextIntentSequence = snapshot.nextIntentSequence > 0
            ? snapshot.nextIntentSequence
            : snapshot.lastProcessedIntentSequence + 1ull;
        m_nextIntentSequence = (std::max)(m_nextIntentSequence, recoveredNextIntentSequence);
        applyMissionWorldState();
        refreshDiagnosticsFromState();
        return true;
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
        ack.clientInstanceId = intent.clientInstanceId;
        ack.sessionId = intent.sessionId;
        ack.hostSimulationTicks = m_diagnostics.simulationTicks;

        if (m_combatEncounterState.active && intent.type != SimulationIntentType::InspectTile)
        {
            ack.result = SimulationIntentAckResult::Rejected;
            ack.reason = "combat encounter active";
            return ack;
        }

        switch (intent.type)
        {
        case SimulationIntentType::MoveToTile:
            if (m_shipRuntimeState.playerBoarded)
            {
                ack.result = SimulationIntentAckResult::Rejected;
                ack.reason = "shipboard movement locked to runtime controls";
                return ack;
            }
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
            if (m_shipRuntimeState.playerBoarded && !isShipboardInteractionTarget(intent.target))
            {
                ack.result = SimulationIntentAckResult::Rejected;
                ack.reason = "planetary interaction unavailable while aboard ship";
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

    void SimulationRuntime::processQueuedIntents(std::vector<SimulationIntent>& processedIntents)
    {
        while (!m_pendingIntents.empty())
        {
            const SimulationIntent intent = m_pendingIntents.front();
            m_pendingIntents.pop_front();

            Action action{};
            action.target = intent.target;
            switch (intent.type)
            {
            case SimulationIntentType::MoveToTile: action.type = ActionType::Move; break;
            case SimulationIntentType::InspectTile: action.type = ActionType::Inspect; break;
            case SimulationIntentType::InteractTile: action.type = ActionType::Interact; break;
            default: action.type = ActionType::Inspect; break;
            }

            m_actions.push(action);
            processedIntents.push_back(intent);
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
        constexpr size_t kMaxEvents = 14;
        if (m_eventLog.size() > kMaxEvents)
        {
            m_eventLog.erase(
                m_eventLog.begin(),
                m_eventLog.begin() + static_cast<std::ptrdiff_t>(m_eventLog.size() - kMaxEvents));
        }
    }

    void SimulationRuntime::refreshDiagnosticsFromState()
    {
        m_diagnostics.playerHealth = m_playerActorState.health;
        m_diagnostics.playerMaxHealth = m_playerActorState.maxHealth;
        m_diagnostics.playerArmor = m_playerActorState.armor;
        m_diagnostics.suitIntegrity = m_playerActorState.suitIntegrity;
        m_diagnostics.oxygenSecondsRemaining = m_playerActorState.oxygenSecondsRemaining;
        m_diagnostics.radiationDose = m_playerActorState.radiationDose;
        m_diagnostics.toxicExposure = m_playerActorState.toxicExposure;
        m_diagnostics.inventoryStackCount = static_cast<uint32_t>(m_playerActorState.inventory.size());
        m_diagnostics.inventoryItemCount = inventoryItemCount(m_playerActorState);
        m_diagnostics.lootCollections = m_playerActorState.lootCollections;
        m_diagnostics.encounterWins = m_playerActorState.encounterWins;
        m_diagnostics.encountersSurvived = m_playerActorState.encountersSurvived;
        m_diagnostics.equippedWeaponText = inventoryItemText(m_playerActorState.equipment.weapon);
        m_diagnostics.equippedSuitText = inventoryItemText(m_playerActorState.equipment.suit);
        m_diagnostics.equippedToolText = inventoryItemText(m_playerActorState.equipment.tool);
        m_diagnostics.inventorySummary = inventorySummaryText(m_playerActorState);

        m_diagnostics.combatActive = m_combatEncounterState.active;
        m_diagnostics.currentCombatLabel = m_combatEncounterState.label.empty() ? "none" : m_combatEncounterState.label;
        m_diagnostics.combatRoundNumber = m_combatEncounterState.roundNumber;
        m_diagnostics.combatTicksRemaining = m_combatEncounterState.roundTicksRemaining;
        m_diagnostics.hostileLabel = m_combatEncounterState.hostileLabel.empty() ? "none" : m_combatEncounterState.hostileLabel;
        m_diagnostics.hostileHealth = m_combatEncounterState.hostileHealth;
        m_diagnostics.hostileMaxHealth = m_combatEncounterState.hostileMaxHealth;

        m_diagnostics.missionActive = m_missionRuntimeState.activeMission != MissionId::None;
        m_diagnostics.missionComplete = m_missionRuntimeState.missionComplete;
        switch (m_missionRuntimeState.phase)
        {
        case MissionPhase::SecureQuarantineGate:
            m_diagnostics.missionGateLocked = !m_missionRuntimeState.quarantineGateUnlocked;
            break;
        case MissionPhase::BoardResponderShuttle:
            m_diagnostics.missionGateLocked = !m_shipRuntimeState.boardingUnlocked;
            break;
        case MissionPhase::ClaimShuttleCommand:
            m_diagnostics.missionGateLocked = !m_shipRuntimeState.playerBoarded;
            break;
        case MissionPhase::EnterOrbitalLane:
            m_diagnostics.missionGateLocked = !m_orbitalRuntimeState.departureAuthorized;
            break;
        case MissionPhase::DockRelayPlatform:
            m_diagnostics.missionGateLocked = !m_missionRuntimeState.relayTrackStabilized;
            break;
        case MissionPhase::LandDustFrontier:
            m_diagnostics.missionGateLocked = !m_orbitalRuntimeState.relayPlatformDocked;
            break;
        case MissionPhase::SecureFrontierRelay:
            m_diagnostics.missionGateLocked = false;
            break;
        case MissionPhase::ReturnToShuttle:
            m_diagnostics.missionGateLocked = !m_shipRuntimeState.frontierSurfaceActive && !m_shipRuntimeState.playerBoarded;
            break;
        case MissionPhase::ReturnToHomeDock:
            m_diagnostics.missionGateLocked = !m_orbitalRuntimeState.homeDockReached;
            break;
        default:
            m_diagnostics.missionGateLocked = false;
            break;
        }
        m_diagnostics.shipRuntimePrepReady = m_missionRuntimeState.shipRuntimePrepReady;
        m_diagnostics.missionAdvancementCount = m_missionRuntimeState.advancementCount;
        m_diagnostics.activeMissionId = missionIdText(m_missionRuntimeState.activeMission);
        m_diagnostics.missionPhaseText = missionPhaseText(m_missionRuntimeState.phase);
        m_diagnostics.missionObjectiveText = missionObjectiveText(m_missionRuntimeState);
        m_diagnostics.missionLastBeat = m_missionRuntimeState.lastBeat.empty()
            ? std::string("none")
            : m_missionRuntimeState.lastBeat;

        m_diagnostics.shipActive = m_shipRuntimeState.activeShip != ShipId::None;
        m_diagnostics.shipBoarded = m_shipRuntimeState.playerBoarded;
        m_diagnostics.shipDocked = m_shipRuntimeState.docked;
        m_diagnostics.shipPowerOnline = m_shipRuntimeState.powerOnline;
        m_diagnostics.shipAirlockPressurized = m_shipRuntimeState.airlockPressurized;
        m_diagnostics.shipCommandClaimed = m_shipRuntimeState.commandClaimed;
        m_diagnostics.shipLaunchPrepReady = m_shipRuntimeState.launchPrepReady;
        m_diagnostics.shipBoardingCount = m_shipRuntimeState.boardingCount;
        m_diagnostics.activeShipId = shipIdText(m_shipRuntimeState.activeShip);
        m_diagnostics.shipName = m_shipRuntimeState.shipName.empty() ? std::string("none") : m_shipRuntimeState.shipName;
        m_diagnostics.shipOwnershipText = shipOwnershipStateText(m_shipRuntimeState.ownershipState);
        m_diagnostics.shipOccupancyText = shipOccupancyStateText(m_shipRuntimeState.occupancyState);
        m_diagnostics.shipLocationText = m_shipRuntimeState.locationLabel.empty() ? std::string("unknown") : m_shipRuntimeState.locationLabel;
        m_diagnostics.shipLastBeat = m_shipRuntimeState.lastBeat.empty()
            ? std::string("none")
            : m_shipRuntimeState.lastBeat;
        m_diagnostics.frontierSurfaceActive = m_shipRuntimeState.frontierSurfaceActive;
        m_diagnostics.frontierSiteText = m_shipRuntimeState.frontierSurfaceActive ? std::string("dust-frontier-pad") : std::string("khepri-dock");

        m_diagnostics.orbitalLayerActive = m_orbitalRuntimeState.orbitalLayerActive;
        m_diagnostics.orbitalDepartureAuthorized = m_orbitalRuntimeState.departureAuthorized;
        m_diagnostics.orbitalTravelInProgress = m_orbitalRuntimeState.travelInProgress;
        m_diagnostics.orbitalSurveyOrbitReached = m_orbitalRuntimeState.surveyOrbitReached;
        m_diagnostics.orbitalRelayTrackReached = m_orbitalRuntimeState.relayTrackReached;
        m_diagnostics.orbitalRelayPlatformDocked = m_orbitalRuntimeState.relayPlatformDocked;
        m_diagnostics.orbitalReturnRouteAuthorized = m_orbitalRuntimeState.returnRouteAuthorized;
        m_diagnostics.orbitalHomeDockReached = m_orbitalRuntimeState.homeDockReached;
        m_diagnostics.orbitalTransferCount = m_orbitalRuntimeState.transferCount;
        m_diagnostics.orbitalTravelTicksRemaining = m_orbitalRuntimeState.travelTicksRemaining;
        m_diagnostics.orbitalPhaseText = orbitalTravelPhaseText(m_orbitalRuntimeState.phase);
        m_diagnostics.orbitalCurrentNodeText = orbitalNodeText(m_orbitalRuntimeState.currentNode);
        m_diagnostics.orbitalTargetNodeText = orbitalNodeText(m_orbitalRuntimeState.targetNode);
        m_diagnostics.orbitalRuleText = m_orbitalRuntimeState.ruleText.empty() ? std::string("none") : m_orbitalRuntimeState.ruleText;
        m_diagnostics.orbitalLastBeat = m_orbitalRuntimeState.lastBeat.empty() ? std::string("none") : m_orbitalRuntimeState.lastBeat;

        if (m_orbitalRuntimeState.orbitalLayerActive)
        {
            m_diagnostics.playerRuntimeContextText = "orbital-space";
        }
        else if (m_shipRuntimeState.playerBoarded)
        {
            m_diagnostics.playerRuntimeContextText = "aboard-docked-ship";
        }
        else if (m_shipRuntimeState.frontierSurfaceActive)
        {
            m_diagnostics.playerRuntimeContextText = "second-destination-surface";
        }
        else
        {
            m_diagnostics.playerRuntimeContextText = "planet-surface";
        }
    }

    void SimulationRuntime::initializeMissionState()
    {
        m_missionRuntimeState = MissionRuntimeState{};
        applyMissionWorldState();
    }

    void SimulationRuntime::initializeShipRuntimeState()
    {
        m_shipRuntimeState = ShipRuntimeState{};
        m_shipRuntimeState.boardingUnlocked = false;
        m_shipRuntimeState.powerOnline = false;
        m_shipRuntimeState.airlockPressurized = false;
        m_shipRuntimeState.commandClaimed = false;
        m_shipRuntimeState.launchPrepReady = false;
        applyShipRuntimeWorldState();
    }

    void SimulationRuntime::initializeOrbitalRuntimeState()
    {
        m_orbitalRuntimeState = OrbitalRuntimeState{};
    }

    void SimulationRuntime::applyMissionWorldState()
    {
        if (WorldAuthoringHotspot* bridgeChokepoint = findHotspotByLabelMutable(m_worldState, kBridgeAccessChokepointLabel))
        {
            bridgeChokepoint->encounterReady = false;
        }

        if (WorldAuthoringHotspot* quarantineGate = findHotspotByLabelMutable(m_worldState, kQuarantineAccessGateLabel))
        {
            quarantineGate->encounterReady =
                m_missionRuntimeState.quarantineGateUnlocked
                && !m_missionRuntimeState.quarantineEncounterResolved;
            m_worldState.world().setBlocked(
                quarantineGate->tile,
                !m_missionRuntimeState.quarantineGateUnlocked);
        }

        m_shipRuntimeState.boardingUnlocked = m_missionRuntimeState.shipRuntimePrepReady;
        applyShipRuntimeWorldState();
    }

    void SimulationRuntime::applyShipRuntimeWorldState()
    {
        const bool shipAtFrontierAnchor =
            m_shipRuntimeState.frontierSurfaceActive
            || m_orbitalRuntimeState.relayPlatformDocked
            || m_orbitalRuntimeState.currentNode == OrbitalNodeId::RelayPlatformDock;

        if (Entity* shipEntity = findEntityByLabelMutable(m_worldState, kResponderShuttleLabel))
        {
            shipEntity->tile = shipAtFrontierAnchor ? kFrontierDockShipTile : kHomeDockShipTile;
            shipEntity->isPowered = m_shipRuntimeState.powerOnline;
            shipEntity->isLocked = !m_shipRuntimeState.boardingUnlocked;
        }

        if (Entity* helmTerminal = findEntityByLabelMutable(m_worldState, kShuttleHelmTerminalLabel))
        {
            helmTerminal->tile = shipAtFrontierAnchor ? kFrontierDockHelmTile : kHomeDockHelmTile;
            helmTerminal->isPowered = m_shipRuntimeState.playerBoarded || m_shipRuntimeState.commandClaimed;
            helmTerminal->isLocked = !m_shipRuntimeState.playerBoarded;
        }

        if (Entity* navConsole = findEntityByLabelMutable(m_worldState, kOrbitalNavigationConsoleLabel))
        {
            navConsole->tile = shipAtFrontierAnchor ? kFrontierDockNavTile : kHomeDockNavTile;
            navConsole->isPowered = m_shipRuntimeState.commandClaimed || m_orbitalRuntimeState.orbitalLayerActive;
            navConsole->isLocked = !m_shipRuntimeState.commandClaimed;
        }

        if (Entity* frontierBeacon = findEntityByLabelMutable(m_worldState, kFrontierRelayBeaconLabel))
        {
            frontierBeacon->isPowered = m_shipRuntimeState.frontierSurfaceActive || m_missionRuntimeState.frontierRelaySecured;
            frontierBeacon->isLocked = !m_shipRuntimeState.frontierSurfaceActive;
        }

        if (WorldAuthoringHotspot* boardingCollar = findHotspotByLabelMutable(m_worldState, kDockedBoardingCollarLabel))
        {
            boardingCollar->encounterReady = false;
            if (m_orbitalRuntimeState.orbitalLayerActive)
            {
                boardingCollar->summary = "Docking collar has retracted. Responder Shuttle Khepri is operating off-surface.";
            }
            else if (m_missionRuntimeState.homeDockRestored)
            {
                boardingCollar->summary = "Khepri Dock has accepted the shuttle return. Disembark here to close the return loop.";
            }
            else
            {
                boardingCollar->summary = m_shipRuntimeState.boardingUnlocked
                    ? "Pressurized shuttle collar is authorized for boarding."
                    : "Boarding collar remains locked until the quarantine corridor is restored.";
            }
        }

        if (WorldAuthoringHotspot* frontierPad = findHotspotByLabelMutable(m_worldState, kDustFrontierLandingPadLabel))
        {
            frontierPad->encounterReady = m_orbitalRuntimeState.relayPlatformDocked || m_shipRuntimeState.frontierSurfaceActive;
            if (m_shipRuntimeState.frontierSurfaceActive)
            {
                frontierPad->summary = "Dust Frontier Landing Pad is active. Secure the relay beacon and return to the shuttle.";
            }
            else if (m_orbitalRuntimeState.relayPlatformDocked)
            {
                frontierPad->summary = "Dust Frontier Landing Pad is docked and ready for disembark.";
            }
            else
            {
                frontierPad->summary = "Dust Frontier Landing Pad remains isolated until Dust Frontier docking is complete.";
            }
        }
    }

    void SimulationRuntime::updateOrbitalTravel()
    {
        if (!m_orbitalRuntimeState.travelInProgress)
        {
            return;
        }

        if (m_orbitalRuntimeState.travelTicksRemaining > 0)
        {
            --m_orbitalRuntimeState.travelTicksRemaining;
        }

        if (m_orbitalRuntimeState.travelTicksRemaining == 0)
        {
            completeOrbitalTransfer();
        }
    }

    void SimulationRuntime::beginOrbitalTransfer(OrbitalNodeId targetNode, OrbitalTravelPhase transferPhase, uint32_t transferTicks)
    {
        m_orbitalRuntimeState.travelInProgress = true;
        m_orbitalRuntimeState.orbitalLayerActive = true;
        m_orbitalRuntimeState.targetNode = targetNode;
        m_orbitalRuntimeState.phase = transferPhase;
        m_orbitalRuntimeState.travelTicksRemaining = transferTicks;
        if (targetNode != OrbitalNodeId::RelayPlatformDock)
        {
            m_orbitalRuntimeState.relayPlatformDocked = false;
        }
        if (targetNode != OrbitalNodeId::HomeDockAnchor)
        {
            m_orbitalRuntimeState.homeDockReached = false;
        }
        ++m_orbitalRuntimeState.transferCount;
        m_shipRuntimeState.docked = false;
        m_shipRuntimeState.frontierSurfaceActive = false;
        m_shipRuntimeState.occupancyState = ShipOccupancyState::AboardOrbitalShip;
        m_shipRuntimeState.locationLabel = orbitalNodeText(targetNode);
        applyShipRuntimeWorldState();
    }

    void SimulationRuntime::completeOrbitalTransfer()
    {
        m_orbitalRuntimeState.travelInProgress = false;
        m_orbitalRuntimeState.currentNode = m_orbitalRuntimeState.targetNode;
        m_shipRuntimeState.locationLabel = orbitalNodeText(m_orbitalRuntimeState.currentNode);

        switch (m_orbitalRuntimeState.targetNode)
        {
        case OrbitalNodeId::TrafficSeparationLane:
            m_orbitalRuntimeState.orbitalLayerActive = true;
            m_orbitalRuntimeState.phase = OrbitalTravelPhase::TrafficLaneHolding;
            m_orbitalRuntimeState.lastBeat = "Responder Shuttle Khepri cleared the dock and stabilized in the local orbital traffic lane.";
            m_orbitalRuntimeState.ruleText = "Traffic lane stabilized. Survey orbit must be plotted from the orbital navigation console.";
            m_missionRuntimeState.orbitalDepartureComplete = true;
            m_missionRuntimeState.phase = MissionPhase::ReachSurveyOrbit;
            ++m_missionRuntimeState.advancementCount;
            m_missionRuntimeState.lastBeat = "Local orbit entered cleanly. Plot the first survey transfer.";
            appendEvent("Orbital transition: shuttle clears the dock and enters the traffic separation lane.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            break;

        case OrbitalNodeId::DebrisSurveyOrbit:
            m_orbitalRuntimeState.surveyOrbitReached = true;
            m_orbitalRuntimeState.phase = OrbitalTravelPhase::SurveyHolding;
            m_orbitalRuntimeState.lastBeat = "Survey orbit reached. Debris ring telemetry is stable enough to continue.";
            m_orbitalRuntimeState.ruleText = "Relay holding track remains gated until the survey orbit pass is complete.";
            m_missionRuntimeState.surveyOrbitReached = true;
            m_missionRuntimeState.phase = MissionPhase::StabilizeRelayTrack;
            ++m_missionRuntimeState.advancementCount;
            m_missionRuntimeState.lastBeat = "Debris Survey Orbit reached. Stabilize the relay holding track next.";
            appendEvent("Orbital navigation: Debris Survey Orbit reached.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            break;

        case OrbitalNodeId::RelayHoldingTrack:
            m_orbitalRuntimeState.relayTrackReached = true;
            m_orbitalRuntimeState.phase = OrbitalTravelPhase::RelayHolding;
            m_orbitalRuntimeState.lastBeat = "Relay holding track stabilized. Dust Frontier docking is now the next live beat.";
            m_orbitalRuntimeState.ruleText = "Relay holding track stable. Dock with Dust Frontier Relay Platform to begin the landing slice.";
            m_missionRuntimeState.relayTrackStabilized = true;
            m_missionRuntimeState.phase = MissionPhase::DockRelayPlatform;
            m_missionRuntimeState.missionComplete = false;
            ++m_missionRuntimeState.advancementCount;
            m_missionRuntimeState.lastBeat = "Relay holding track stabilized. Dock with Dust Frontier Relay Platform next.";
            appendEvent("Orbital navigation: relay holding track stabilized.");
            appendEvent("M44 unlocked: Dust Frontier docking and return-loop continuity are now live.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            break;

        case OrbitalNodeId::RelayPlatformDock:
            m_orbitalRuntimeState.relayPlatformDocked = true;
            m_orbitalRuntimeState.orbitalLayerActive = false;
            m_orbitalRuntimeState.phase = OrbitalTravelPhase::RelayPlatformDocked;
            m_orbitalRuntimeState.lastBeat = "Responder Shuttle Khepri docked cleanly with Dust Frontier Relay Platform.";
            m_orbitalRuntimeState.ruleText = "Dust Frontier dock stable. Interact with Responder Shuttle Khepri to disembark onto the frontier pad.";
            m_shipRuntimeState.docked = true;
            m_shipRuntimeState.frontierSurfaceAccessUnlocked = true;
            m_shipRuntimeState.locationLabel = "dust-frontier-pad";
            setAuthoritativePlayerTile(kFrontierDockShipTile);
            m_missionRuntimeState.relayPlatformDocked = true;
            m_missionRuntimeState.phase = MissionPhase::LandDustFrontier;
            ++m_missionRuntimeState.advancementCount;
            m_missionRuntimeState.lastBeat = "Dust Frontier relay platform docked. Disembark to the surface pad.";
            applyShipRuntimeWorldState();
            appendEvent("Orbital docking: Dust Frontier Relay Platform secured.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            break;

        case OrbitalNodeId::HomeDockAnchor:
            m_orbitalRuntimeState.homeDockReached = true;
            m_orbitalRuntimeState.returnRouteAuthorized = false;
            m_orbitalRuntimeState.orbitalLayerActive = false;
            m_orbitalRuntimeState.phase = OrbitalTravelPhase::HomeDocked;
            m_orbitalRuntimeState.lastBeat = "Responder Shuttle Khepri has redocked with Khepri Dock.";
            m_orbitalRuntimeState.ruleText = "Home dock stable. Disembark to close the first planet-orbit-planet return loop.";
            m_shipRuntimeState.docked = true;
            m_shipRuntimeState.frontierSurfaceActive = false;
            m_shipRuntimeState.locationLabel = "cargo-bay-dock";
            setAuthoritativePlayerTile(kHomeDockShipTile);
            m_missionRuntimeState.homeDockRestored = true;
            m_missionRuntimeState.lastBeat = "Khepri Dock restored. Disembark to complete the return loop.";
            applyShipRuntimeWorldState();
            appendEvent("Orbital docking: Responder Shuttle Khepri redocks with Khepri Dock.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            break;

        default:
            break;
        }

        refreshDiagnosticsFromState();
    }

    void SimulationRuntime::setAuthoritativePlayerTile(TileCoord tile, bool clearMovementState)
    {
        m_authoritativePlayerPosition = m_worldState.world().tileToWorldCenter(tile);
        m_previousAuthoritativePlayerPosition = m_authoritativePlayerPosition;
        m_presentedPlayerPosition = m_authoritativePlayerPosition;
        if (clearMovementState)
        {
            clearMovement();
            m_pendingIntents.clear();
        }
    }

    bool SimulationRuntime::isShipboardInteractionTarget(TileCoord target) const
    {
        const Entity* entity = m_worldState.entities().getAt(target);
        if (entity == nullptr)
        {
            return false;
        }

        return entity->name == kResponderShuttleLabel
            || entity->name == kShuttleHelmTerminalLabel
            || entity->name == kOrbitalNavigationConsoleLabel;
    }

    void SimulationRuntime::evaluateMissionProgressFromIntent(const SimulationIntent& intent)
    {
        switch (intent.type)
        {
        case SimulationIntentType::InspectTile:
            handleMissionInspect(intent.target);
            break;
        case SimulationIntentType::InteractTile:
            handleMissionInteraction(intent.target);
            break;
        default:
            break;
        }
    }

    void SimulationRuntime::handleMissionInspect(TileCoord target)
    {
        if (m_missionRuntimeState.phase != MissionPhase::DiagnoseMedlab)
        {
            return;
        }

        const Entity* entity = m_worldState.entities().getAt(target);
        const WorldAuthoringHotspot* hotspot = m_worldState.authoringHotspotAt(target);

        const bool diagnosticStationInspected =
            entity != nullptr && entity->name == kDiagnosticStationLabel;
        const bool triageHotspotInspected =
            hotspot != nullptr && hotspot->label == kTriageConvergenceLabel;

        if (!diagnosticStationInspected && !triageHotspotInspected)
        {
            return;
        }

        m_missionRuntimeState.medlabDiagnosisComplete = true;
        m_missionRuntimeState.quarantineGateUnlocked = true;
        m_missionRuntimeState.phase = MissionPhase::SecureQuarantineGate;
        ++m_missionRuntimeState.advancementCount;
        m_missionRuntimeState.lastBeat = "MedLab diagnostics identified the quarantine breach route.";

        applyMissionWorldState();
        appendEvent("Mission advanced: MedLab diagnostics identify the quarantine breach route.");
        appendEvent("Gate update: Quarantine Access Gate is now unlocked.");
        appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
        refreshDiagnosticsFromState();
    }

    void SimulationRuntime::handleMissionInteraction(TileCoord target)
    {
        const Entity* entity = m_worldState.entities().getAt(target);
        if (entity == nullptr)
        {
            return;
        }

        if (entity->name == kTransitServiceTerminalLabel)
        {
            if (m_missionRuntimeState.phase == MissionPhase::RecoverTransitData)
            {
                m_missionRuntimeState.transitDataRecovered = true;
                m_missionRuntimeState.phase = MissionPhase::DiagnoseMedlab;
                ++m_missionRuntimeState.advancementCount;
                m_missionRuntimeState.lastBeat = "Transit route data recovered from the service terminal.";

                appendEvent("Mission advanced: Transit route data recovered.");
                appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
                refreshDiagnosticsFromState();
                return;
            }

            appendEvent("Transit Service Terminal confirms the route package is already cached.");
            return;
        }

        if (entity->name == kQuarantineControlTerminalLabel)
        {
            if (m_missionRuntimeState.phase != MissionPhase::RestoreQuarantineControl
                && m_missionRuntimeState.phase != MissionPhase::BoardResponderShuttle
                && m_missionRuntimeState.phase != MissionPhase::ClaimShuttleCommand
                && m_missionRuntimeState.phase != MissionPhase::EnterOrbitalLane
                && m_missionRuntimeState.phase != MissionPhase::ReachSurveyOrbit
                && m_missionRuntimeState.phase != MissionPhase::StabilizeRelayTrack
                && m_missionRuntimeState.phase != MissionPhase::DockRelayPlatform
                && m_missionRuntimeState.phase != MissionPhase::LandDustFrontier
                && m_missionRuntimeState.phase != MissionPhase::SecureFrontierRelay
                && m_missionRuntimeState.phase != MissionPhase::ReturnToShuttle
                && m_missionRuntimeState.phase != MissionPhase::ReturnToHomeDock
                && m_missionRuntimeState.phase != MissionPhase::MissionComplete)
            {
                appendEvent("Mission gate: Quarantine control rejects reroute commands until the access gate is secured.");
                return;
            }

            if (m_missionRuntimeState.quarantineControlRestored)
            {
                appendEvent("Quarantine Control Terminal confirms the evac corridor is already restored.");
                return;
            }

            m_missionRuntimeState.quarantineControlRestored = true;
            m_missionRuntimeState.shipRuntimePrepReady = true;
            m_missionRuntimeState.phase = MissionPhase::BoardResponderShuttle;
            ++m_missionRuntimeState.advancementCount;
            m_missionRuntimeState.lastBeat = "Quarantine corridor restored. Shuttle boarding authorization is now live.";
            m_shipRuntimeState.boardingUnlocked = true;
            m_shipRuntimeState.lastBeat = "Docked responder shuttle is now authorized for boarding.";

            applyMissionWorldState();
            appendEvent("Mission advanced: Quarantine corridor restored.");
            appendEvent("Boarding update: Responder Shuttle Khepri is now authorized for boarding.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            refreshDiagnosticsFromState();
            return;
        }

        if (entity->name == kFrontierRelayBeaconLabel)
        {
            if (!m_shipRuntimeState.frontierSurfaceActive)
            {
                appendEvent("Mission gate: Frontier Relay Beacon remains inaccessible until Dust Frontier landing is complete.");
                return;
            }

            if (m_missionRuntimeState.phase != MissionPhase::SecureFrontierRelay
                && m_missionRuntimeState.phase != MissionPhase::ReturnToShuttle
                && m_missionRuntimeState.phase != MissionPhase::ReturnToHomeDock
                && m_missionRuntimeState.phase != MissionPhase::MissionComplete)
            {
                appendEvent("Frontier Relay Beacon has no actionable route handoff for the current mission beat.");
                return;
            }

            if (m_missionRuntimeState.frontierRelaySecured)
            {
                appendEvent("Frontier Relay Beacon confirms the surface handoff package is already secured.");
                return;
            }

            m_missionRuntimeState.frontierRelaySecured = true;
            m_missionRuntimeState.phase = MissionPhase::ReturnToShuttle;
            ++m_missionRuntimeState.advancementCount;
            m_missionRuntimeState.lastBeat = "Dust Frontier relay package secured. Return to the shuttle and launch home.";
            m_orbitalRuntimeState.returnRouteAuthorized = true;
            m_orbitalRuntimeState.ruleText = "Return route to Khepri Dock authorized. Re-board the shuttle and use the helm to launch home.";
            m_orbitalRuntimeState.lastBeat = "Surface relay secured. Homeward route authority granted.";
            m_shipRuntimeState.lastBeat = "Dust Frontier surface handoff complete. Shuttle return is now authorized.";

            applyShipRuntimeWorldState();
            appendEvent("Mission advanced: Dust Frontier relay package secured.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            refreshDiagnosticsFromState();
            return;
        }

        if (entity->name == kResponderShuttleLabel)
        {
            if (m_shipRuntimeState.playerBoarded)
            {
                if (m_missionRuntimeState.phase == MissionPhase::LandDustFrontier
                    && m_orbitalRuntimeState.relayPlatformDocked)
                {
                    m_shipRuntimeState.playerBoarded = false;
                    m_shipRuntimeState.occupancyState = ShipOccupancyState::PlanetSurface;
                    m_shipRuntimeState.frontierSurfaceActive = true;
                    m_shipRuntimeState.lastBeat = "Disembarked from Responder Shuttle Khepri onto Dust Frontier Landing Pad.";
                    setAuthoritativePlayerTile(kFrontierPadPlayerTile);
                    m_missionRuntimeState.dustFrontierLanded = true;
                    m_missionRuntimeState.phase = MissionPhase::SecureFrontierRelay;
                    ++m_missionRuntimeState.advancementCount;
                    m_missionRuntimeState.lastBeat = "Dust Frontier landing confirmed. Secure the relay beacon.";
                    applyShipRuntimeWorldState();
                    appendEvent("Landing: you disembark onto Dust Frontier Landing Pad.");
                    appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
                    refreshDiagnosticsFromState();
                    return;
                }

                if (m_missionRuntimeState.phase == MissionPhase::ReturnToHomeDock
                    && m_orbitalRuntimeState.homeDockReached)
                {
                    m_shipRuntimeState.playerBoarded = false;
                    m_shipRuntimeState.occupancyState = ShipOccupancyState::PlanetSurface;
                    m_shipRuntimeState.frontierSurfaceActive = false;
                    m_shipRuntimeState.lastBeat = "Disembarked from Responder Shuttle Khepri at Khepri Dock.";
                    setAuthoritativePlayerTile(kHomeDockPlayerTile);
                    m_missionRuntimeState.returnLoopComplete = true;
                    m_missionRuntimeState.missionComplete = true;
                    m_missionRuntimeState.phase = MissionPhase::MissionComplete;
                    ++m_missionRuntimeState.advancementCount;
                    m_missionRuntimeState.lastBeat = "Khepri Dock return loop completed cleanly.";
                    applyShipRuntimeWorldState();
                    appendEvent("Return loop: you disembark at Khepri Dock and close the first planet-orbit-planet continuity lane.");
                    appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
                    refreshDiagnosticsFromState();
                    return;
                }

                appendEvent("Responder Shuttle Khepri confirms you are already aboard the active vessel.");
                return;
            }

            const bool initialBoardingAllowed =
                m_shipRuntimeState.boardingUnlocked
                && (m_missionRuntimeState.phase == MissionPhase::BoardResponderShuttle
                    || m_missionRuntimeState.phase == MissionPhase::ClaimShuttleCommand
                    || m_missionRuntimeState.phase == MissionPhase::EnterOrbitalLane
                    || m_missionRuntimeState.phase == MissionPhase::ReachSurveyOrbit
                    || m_missionRuntimeState.phase == MissionPhase::StabilizeRelayTrack
                    || m_missionRuntimeState.phase == MissionPhase::DockRelayPlatform
                    || m_missionRuntimeState.phase == MissionPhase::ReturnToHomeDock
                    || m_missionRuntimeState.phase == MissionPhase::MissionComplete);

            const bool frontierReboardingAllowed =
                m_shipRuntimeState.frontierSurfaceActive
                && (m_missionRuntimeState.phase == MissionPhase::ReturnToShuttle
                    || m_missionRuntimeState.phase == MissionPhase::ReturnToHomeDock
                    || m_missionRuntimeState.phase == MissionPhase::MissionComplete);

            if (!initialBoardingAllowed && !frontierReboardingAllowed)
            {
                appendEvent("Mission gate: shuttle boarding remains locked until the corridor is restored.");
                return;
            }

            m_shipRuntimeState.playerBoarded = true;
            m_shipRuntimeState.occupancyState = ShipOccupancyState::AboardDockedShip;
            m_shipRuntimeState.powerOnline = true;
            m_shipRuntimeState.airlockPressurized = true;
            ++m_shipRuntimeState.boardingCount;
            setAuthoritativePlayerTile(
                m_shipRuntimeState.frontierSurfaceActive ? kFrontierDockShipTile : kHomeDockShipTile);

            if (frontierReboardingAllowed)
            {
                m_shipRuntimeState.frontierSurfaceActive = false;
                m_shipRuntimeState.lastBeat = "Re-boarded Responder Shuttle Khepri from Dust Frontier Landing Pad.";
                applyShipRuntimeWorldState();
                appendEvent("Ship runtime: you re-board Responder Shuttle Khepri from Dust Frontier Landing Pad.");
                appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
                refreshDiagnosticsFromState();
                return;
            }

            m_shipRuntimeState.lastBeat = "Boarded the docked responder shuttle through the cargo collar.";
            m_missionRuntimeState.phase = MissionPhase::ClaimShuttleCommand;
            ++m_missionRuntimeState.advancementCount;
            m_missionRuntimeState.lastBeat = "Responder Shuttle Khepri boarded. Helm authority is the next beat.";

            applyShipRuntimeWorldState();
            appendEvent("Ship runtime: you board Responder Shuttle Khepri.");
            appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
            refreshDiagnosticsFromState();
            return;
        }

        if (entity->name == kShuttleHelmTerminalLabel)
        {
            if (!m_shipRuntimeState.playerBoarded)
            {
                appendEvent("Mission gate: Shuttle Helm Terminal will not grant command authority until you are aboard.");
                return;
            }

            if (!m_shipRuntimeState.commandClaimed)
            {
                if (m_missionRuntimeState.phase != MissionPhase::ClaimShuttleCommand)
                {
                    appendEvent("Mission gate: helm authorization is not yet the active beat.");
                    return;
                }

                m_shipRuntimeState.commandClaimed = true;
                m_shipRuntimeState.launchPrepReady = true;
                m_shipRuntimeState.ownershipState = ShipOwnershipState::PlayerCommand;
                m_shipRuntimeState.lastBeat = "Shuttle helm accepted your command credentials for departure prep.";

                m_missionRuntimeState.shipRuntimePrepReady = true;
                m_missionRuntimeState.phase = MissionPhase::EnterOrbitalLane;
                ++m_missionRuntimeState.advancementCount;
                m_missionRuntimeState.lastBeat = "Command authority claimed. Depart into local orbit next.";

                m_orbitalRuntimeState.orbitalLayerUnlocked = true;
                m_orbitalRuntimeState.departureAuthorized = true;
                m_orbitalRuntimeState.ruleText = "Use Shuttle Helm Terminal once more to clear the docking collar and enter local orbit.";
                m_orbitalRuntimeState.lastBeat = "Orbital layer unlocked from shuttle helm authority.";

                applyShipRuntimeWorldState();
                appendEvent("Ship runtime: Shuttle Helm Terminal transfers command authority to you.");
                appendEvent("M43 unlocked: local orbital departure is now authorized.");
                appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
                refreshDiagnosticsFromState();
                return;
            }

            if (m_missionRuntimeState.phase == MissionPhase::EnterOrbitalLane)
            {
                if (m_orbitalRuntimeState.travelInProgress)
                {
                    appendEvent("Orbital navigation: departure transfer already underway.");
                    return;
                }
                if (m_orbitalRuntimeState.orbitalLayerActive)
                {
                    appendEvent("Shuttle helm confirms the traffic separation lane is already stable. Use Orbital Navigation Console for the next route.");
                    return;
                }

                beginOrbitalTransfer(OrbitalNodeId::TrafficSeparationLane, OrbitalTravelPhase::UndockingTransfer, 80u);
                m_orbitalRuntimeState.lastBeat = "Undocking burn committed from the shuttle helm.";
                m_orbitalRuntimeState.ruleText = "During transfer, shipboard movement remains locked and route changes are unavailable.";
                appendEvent("Orbital transition: undocking burn committed from Shuttle Helm Terminal.");
                refreshDiagnosticsFromState();
                return;
            }

            if (m_missionRuntimeState.phase == MissionPhase::ReturnToShuttle)
            {
                if (!m_orbitalRuntimeState.returnRouteAuthorized)
                {
                    appendEvent("Mission gate: the frontier relay package must be secured before the homeward launch is authorized.");
                    return;
                }

                if (m_orbitalRuntimeState.travelInProgress)
                {
                    appendEvent("Orbital navigation: return launch already underway.");
                    return;
                }

                m_missionRuntimeState.returnLaunchComplete = true;
                m_missionRuntimeState.phase = MissionPhase::ReturnToHomeDock;
                ++m_missionRuntimeState.advancementCount;
                m_missionRuntimeState.lastBeat = "Homeward launch committed from Dust Frontier.";
                m_orbitalRuntimeState.lastBeat = "Homeward burn committed from Dust Frontier shuttle helm.";
                m_orbitalRuntimeState.ruleText = "Home dock transfer active. Hold course until Khepri Dock capture is confirmed.";
                beginOrbitalTransfer(OrbitalNodeId::HomeDockAnchor, OrbitalTravelPhase::HomeDockTransfer, 140u);
                appendEvent("Orbital transition: homeward launch committed from Dust Frontier shuttle helm.");
                appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
                refreshDiagnosticsFromState();
                return;
            }

            appendEvent("Shuttle helm confirms command authority remains stable.");
            return;
        }

        if (entity->name == kOrbitalNavigationConsoleLabel)
        {
            if (!m_shipRuntimeState.commandClaimed)
            {
                appendEvent("Mission gate: Orbital Navigation Console remains locked until helm command authority is claimed.");
                return;
            }

            if (m_orbitalRuntimeState.travelInProgress)
            {
                appendEvent("Orbital navigation: active transfer in progress. Wait for the current route to resolve.");
                return;
            }

            if (m_missionRuntimeState.phase == MissionPhase::EnterOrbitalLane)
            {
                appendEvent("Mission gate: use Shuttle Helm Terminal to enter the traffic separation lane before plotting further routes.");
                return;
            }

            if (m_missionRuntimeState.phase == MissionPhase::ReachSurveyOrbit)
            {
                beginOrbitalTransfer(OrbitalNodeId::DebrisSurveyOrbit, OrbitalTravelPhase::SurveyTransfer, 120u);
                m_orbitalRuntimeState.lastBeat = "Survey route plotted toward the debris ring observation lane.";
                m_orbitalRuntimeState.ruleText = "Survey transfer active. Relay holding track remains unavailable until arrival is confirmed.";
                appendEvent("Orbital navigation: transfer to Debris Survey Orbit plotted.");
                refreshDiagnosticsFromState();
                return;
            }

            if (m_missionRuntimeState.phase == MissionPhase::StabilizeRelayTrack)
            {
                beginOrbitalTransfer(OrbitalNodeId::RelayHoldingTrack, OrbitalTravelPhase::RelayTransfer, 100u);
                m_orbitalRuntimeState.lastBeat = "Relay holding track stabilization run initiated from the navigation console.";
                m_orbitalRuntimeState.ruleText = "Relay transfer active. Hold course until the orbital lane stabilizes.";
                appendEvent("Orbital navigation: relay holding track stabilization run initiated.");
                refreshDiagnosticsFromState();
                return;
            }

            if (m_missionRuntimeState.phase == MissionPhase::DockRelayPlatform)
            {
                beginOrbitalTransfer(OrbitalNodeId::RelayPlatformDock, OrbitalTravelPhase::DockingTransfer, 110u);
                m_orbitalRuntimeState.lastBeat = "Docking run plotted toward Dust Frontier Relay Platform.";
                m_orbitalRuntimeState.ruleText = "Dust Frontier docking transfer active. Hold course until the platform capture is confirmed.";
                appendEvent("Orbital docking: transfer to Dust Frontier Relay Platform plotted.");
                refreshDiagnosticsFromState();
                return;
            }

            if (m_missionRuntimeState.phase == MissionPhase::MissionComplete)
            {
                appendEvent("Orbital Navigation Console confirms the first return loop is stable and ready for M45 hardening.");
                return;
            }
        }
    }

    void SimulationRuntime::handleMissionEncounterVictory(const std::string& encounterLabel)
    {
        if (encounterLabel != kQuarantineAccessGateLabel
            || m_missionRuntimeState.phase != MissionPhase::SecureQuarantineGate)
        {
            return;
        }

        m_missionRuntimeState.quarantineEncounterResolved = true;
        m_missionRuntimeState.phase = MissionPhase::RestoreQuarantineControl;
        ++m_missionRuntimeState.advancementCount;
        m_missionRuntimeState.lastBeat = "Quarantine Access Gate secured after the six-second combat beat.";

        applyMissionWorldState();
        appendEvent("Mission advanced: Quarantine Access Gate secured.");
        appendEvent(std::string("Objective: ") + missionObjectiveText(m_missionRuntimeState));
        refreshDiagnosticsFromState();
    }

    void SimulationRuntime::processSurvivalState(float stepSeconds)
    {
        const TileCoord playerTile = m_worldState.world().worldToTile(m_authoritativePlayerPosition);
        const TerrainHazardTile* hazard = m_worldState.terrainHazardAt(playerTile);

        m_diagnostics.playerInHazard = false;
        if (hazard == nullptr)
        {
            m_diagnostics.currentHazardLabel = "none";
            if (!m_combatEncounterState.active)
            {
                m_diagnostics.currentTerrainConsequence = "stable";
            }
            return;
        }

        m_diagnostics.playerInHazard = true;
        ++m_diagnostics.hazardTicks;
        m_diagnostics.currentHazardLabel = hazard->label;

        switch (hazard->type)
        {
        case TerrainHazardType::VacuumLeak:
            m_playerActorState.oxygenSecondsRemaining = (std::max)(0.0f, m_playerActorState.oxygenSecondsRemaining - stepSeconds * static_cast<float>(hazard->severity));
            m_diagnostics.currentTerrainConsequence = "pressure loss draining oxygen";
            if (m_playerActorState.oxygenSecondsRemaining <= 0.0f && (m_diagnostics.simulationTicks % 20ull) == 0ull)
            {
                m_playerActorState.health = (std::max)(0, m_playerActorState.health - hazard->severity);
                appendEvent("Vacuum exposure is now causing direct injury.");
            }
            break;

        case TerrainHazardType::RadiationLeak:
            if ((m_diagnostics.simulationTicks % 20ull) == 0ull)
            {
                m_playerActorState.radiationDose += hazard->severity;
                if (m_playerActorState.radiationDose % 20 == 0)
                {
                    m_playerActorState.health = (std::max)(0, m_playerActorState.health - 1);
                }
            }
            m_diagnostics.currentTerrainConsequence = "radiation exposure accumulating";
            break;

        case TerrainHazardType::ToxicSpill:
            if ((m_diagnostics.simulationTicks % 20ull) == 0ull)
            {
                m_playerActorState.toxicExposure += hazard->severity;
                if (m_playerActorState.toxicExposure >= 10)
                {
                    m_playerActorState.health = (std::max)(0, m_playerActorState.health - 1);
                }
            }
            m_diagnostics.currentTerrainConsequence = "toxic contamination spreading";
            break;

        case TerrainHazardType::ElectricalArc:
            if ((m_diagnostics.simulationTicks % 20ull) == 0ull)
            {
                m_playerActorState.suitIntegrity = (std::max)(0, m_playerActorState.suitIntegrity - hazard->severity * 2);
                m_playerActorState.health = (std::max)(0, m_playerActorState.health - hazard->severity);
            }
            m_diagnostics.currentTerrainConsequence = "electrical arcing damaging suit systems";
            break;

        default:
            break;
        }

        if (m_playerActorState.suitIntegrity <= 0)
        {
            m_diagnostics.currentTerrainConsequence = "suit integrity collapsed";
        }

        tryAutoUseMedicalRecovery();
    }

    void SimulationRuntime::evaluateEncounterTriggers()
    {
        if (m_combatEncounterState.active)
        {
            return;
        }

        const TileCoord playerTile = m_worldState.world().worldToTile(m_authoritativePlayerPosition);
        const WorldAuthoringHotspot* hotspot = m_worldState.authoringHotspotAt(playerTile);
        if (hotspot != nullptr
            && hotspot->type == WorldAuthoringHotspotType::Encounter
            && hotspot->encounterReady)
        {
            startEncounterAtHotspot(*hotspot);
        }
    }

    void SimulationRuntime::startEncounterAtHotspot(const WorldAuthoringHotspot& hotspot)
    {
        m_combatEncounterState = CombatEncounterState{};
        m_combatEncounterState.active = true;
        m_combatEncounterState.encounterId = hotspot.label;
        m_combatEncounterState.label = hotspot.label;
        m_combatEncounterState.anchorTile = hotspot.tile;
        m_combatEncounterState.roundNumber = 1;
        m_combatEncounterState.roundTicksRemaining = kCombatRoundTicks;
        m_combatEncounterState.lastOutcome = "engaged";

        if (hotspot.region == WorldRegionTagId::CommandDeck)
        {
            m_combatEncounterState.hostileLabel = "Command deck raider";
            m_combatEncounterState.hostileHealth = 42;
            m_combatEncounterState.hostileMaxHealth = 42;
            m_combatEncounterState.hostileArmor = 3;
            m_combatEncounterState.hostileMinDamage = 5;
            m_combatEncounterState.hostileMaxDamage = 9;
        }
        else
        {
            m_combatEncounterState.hostileLabel = "Containment scavenger";
            m_combatEncounterState.hostileHealth = 36;
            m_combatEncounterState.hostileMaxHealth = 36;
            m_combatEncounterState.hostileArmor = 2;
            m_combatEncounterState.hostileMinDamage = 4;
            m_combatEncounterState.hostileMaxDamage = 8;
        }

        clearMovement();
        appendEvent(std::string("Combat engaged: ") + hotspot.label + ". First six-second exchange underway.");
        m_diagnostics.currentTerrainConsequence = "hostile contact established";
        ++m_diagnostics.terrainConsequenceEvents;
        refreshDiagnosticsFromState();
    }

    void SimulationRuntime::updateCombatEncounter()
    {
        if (!m_combatEncounterState.active)
        {
            return;
        }

        if (m_combatEncounterState.roundTicksRemaining > 0)
        {
            --m_combatEncounterState.roundTicksRemaining;
        }

        if (m_combatEncounterState.roundTicksRemaining == 0)
        {
            resolveCombatRound();
        }
    }

    void SimulationRuntime::resolveCombatRound()
    {
        if (!m_combatEncounterState.active)
        {
            return;
        }

        const int playerAttack = deterministicRoll(
            m_diagnostics.simulationTicks + m_combatEncounterState.roundNumber,
            inventoryItemWeaponMinDamage(m_playerActorState.equipment.weapon),
            inventoryItemWeaponMaxDamage(m_playerActorState.equipment.weapon));
        const int enemyMitigated = (std::max)(1, playerAttack - m_combatEncounterState.hostileArmor);
        m_combatEncounterState.hostileHealth = (std::max)(0, m_combatEncounterState.hostileHealth - enemyMitigated);

        appendEvent(
            std::string("Round ")
            + std::to_string(m_combatEncounterState.roundNumber)
            + ": you hit "
            + m_combatEncounterState.hostileLabel
            + " for "
            + std::to_string(enemyMitigated)
            + ".");

        ++m_diagnostics.combatRoundsResolved;

        if (m_combatEncounterState.hostileHealth <= 0)
        {
            finishEncounterVictory();
            return;
        }

        const int rawEnemyDamage = deterministicRoll(
            (m_diagnostics.simulationTicks * 5ull) + m_combatEncounterState.roundNumber,
            m_combatEncounterState.hostileMinDamage,
            m_combatEncounterState.hostileMaxDamage);

        const int totalArmor = m_playerActorState.armor + inventoryItemArmorBonus(m_playerActorState.equipment.suit);
        const int mitigatedEnemyDamage = (std::max)(1, rawEnemyDamage - (totalArmor / 6));
        m_playerActorState.health = (std::max)(0, m_playerActorState.health - mitigatedEnemyDamage);
        m_playerActorState.armor = (std::max)(0, m_playerActorState.armor - 1);
        m_playerActorState.suitIntegrity = (std::max)(0, m_playerActorState.suitIntegrity - 2);

        appendEvent(
            std::string(m_combatEncounterState.hostileLabel)
            + " hits you for "
            + std::to_string(mitigatedEnemyDamage)
            + ".");

        tryAutoUseMedicalRecovery();

        if (m_playerActorState.health <= 0)
        {
            finishEncounterDefeat();
            return;
        }

        ++m_combatEncounterState.roundNumber;
        m_combatEncounterState.roundTicksRemaining = kCombatRoundTicks;
        m_combatEncounterState.lastOutcome = "round resolved";
        refreshDiagnosticsFromState();
    }

    void SimulationRuntime::finishEncounterVictory()
    {
        m_combatEncounterState.active = false;
        m_combatEncounterState.lastOutcome = "victory";
        ++m_playerActorState.encounterWins;
        ++m_playerActorState.encountersSurvived;

        if (WorldAuthoringHotspot* hotspot = m_worldState.authoringHotspotAtMutable(m_combatEncounterState.anchorTile))
        {
            hotspot->encounterReady = false;
        }

        appendEvent(
            std::string("Encounter resolved: ")
            + m_combatEncounterState.label
            + " secured.");
        m_diagnostics.currentTerrainConsequence = "combat encounter secured";
        ++m_diagnostics.terrainConsequenceEvents;

        handleMissionEncounterVictory(m_combatEncounterState.label);
        refreshDiagnosticsFromState();
    }

    void SimulationRuntime::finishEncounterDefeat()
    {
        m_combatEncounterState.active = false;
        m_combatEncounterState.lastOutcome = "forced retreat";
        m_playerActorState.health = 1;
        m_playerActorState.armor = (std::max)(0, m_playerActorState.armor - 3);
        m_playerActorState.suitIntegrity = (std::max)(0, m_playerActorState.suitIntegrity - 5);

        appendEvent(
            std::string("Critical injury. You stagger clear of ")
            + m_combatEncounterState.label
            + " and cling to life.");
        m_diagnostics.currentTerrainConsequence = "forced retreat after combat injury";
        ++m_diagnostics.terrainConsequenceEvents;
        clearMovement();
        refreshDiagnosticsFromState();
    }

    void SimulationRuntime::tryAutoUseMedicalRecovery()
    {
        if (m_playerActorState.health > 25)
        {
            return;
        }

        if (consumeInventoryItem(m_playerActorState, InventoryItemId::MedInjector, 1))
        {
            m_playerActorState.health = (std::min)(m_playerActorState.maxHealth, m_playerActorState.health + 20);
            appendEvent("Auto-injector stabilizes you mid-operation.");
            return;
        }

        if (m_playerActorState.suitIntegrity < 40 && consumeInventoryItem(m_playerActorState, InventoryItemId::PatchKit, 1))
        {
            m_playerActorState.suitIntegrity = (std::min)(100, m_playerActorState.suitIntegrity + 20);
            appendEvent("Patch kit restores critical suit integrity.");
        }
    }

    void SimulationRuntime::clearMovement()
    {
        m_currentPath.clear();
        m_pathIndex = 0;
        m_hasMovementTarget = false;
    }
}
