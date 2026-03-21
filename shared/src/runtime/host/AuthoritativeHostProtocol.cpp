#include "engine/host/AuthoritativeHostProtocol.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <windows.h>

namespace war
{
    namespace
    {
        using StringMap = std::unordered_map<std::string, std::string>;

        std::string sanitizeSingleLine(const std::string& value)
        {
            std::string sanitized = value;
            for (char& ch : sanitized)
            {
                if (ch == '\n' || ch == '\r')
                {
                    ch = ' ';
                }
                else if (ch == '|')
                {
                    ch = '/';
                }
            }
            return sanitized;
        }

        bool directoryExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_directory(path, error);
        }

        bool fileExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_regular_file(path, error);
        }

        void ensureDirectory(const std::filesystem::path& path, std::vector<std::string>& issues)
        {
            std::error_code error;
            if (!std::filesystem::exists(path, error))
            {
                std::filesystem::create_directories(path, error);
            }

            if (error)
            {
                issues.push_back(std::string("Failed to create protocol directory: ") + path.generic_string());
            }
        }

        bool readTextFileForAtomicReplace(const std::filesystem::path& path, std::string& outContents)
        {
            outContents.clear();
#if defined(_WIN32)
            HANDLE handle = CreateFileW(
                path.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);
            if (handle == INVALID_HANDLE_VALUE)
            {
                return false;
            }

            LARGE_INTEGER fileSize{};
            if (!GetFileSizeEx(handle, &fileSize) || fileSize.QuadPart < 0)
            {
                CloseHandle(handle);
                return false;
            }

            const DWORD requestedBytes = static_cast<DWORD>(fileSize.QuadPart);
            std::vector<char> buffer(static_cast<size_t>(requestedBytes), '\0');
            DWORD bytesRead = 0;
            const BOOL readOk = requestedBytes == 0
                ? TRUE
                : ReadFile(handle, buffer.data(), requestedBytes, &bytesRead, nullptr);
            CloseHandle(handle);
            if (!readOk)
            {
                return false;
            }

            outContents.assign(buffer.data(), buffer.data() + bytesRead);
            return true;
#else
            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                return false;
            }

            std::ostringstream stream;
            stream << input.rdbuf();
            outContents = stream.str();
            return static_cast<bool>(input) || input.eof();
#endif
        }

        StringMap parseKeyValueText(const std::string& contents)
        {
            StringMap values{};
            std::istringstream input(contents);
            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                const size_t split = line.find('=');
                if (split == std::string::npos)
                {
                    continue;
                }

                values.emplace(line.substr(0, split), line.substr(split + 1));
            }

            return values;
        }

        StringMap parseKeyValueFile(const std::filesystem::path& path)
        {
            std::string contents;
            if (!readTextFileForAtomicReplace(path, contents))
            {
                return {};
            }

            return parseKeyValueText(contents);
        }

        bool tryParseUnsigned(const StringMap& values, const char* key, uint64_t& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            try
            {
                outValue = static_cast<uint64_t>(std::stoull(it->second));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool tryParseInt(const StringMap& values, const char* key, int& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            try
            {
                outValue = std::stoi(it->second);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool tryParseFloat(const StringMap& values, const char* key, float& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            try
            {
                outValue = std::stof(it->second);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool tryParseString(const StringMap& values, const char* key, std::string& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            outValue = it->second;
            return true;
        }

        bool tryParseYesNo(const StringMap& values, const char* key, bool& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            if (it->second == "yes" || it->second == "true" || it->second == "1")
            {
                outValue = true;
                return true;
            }
            if (it->second == "no" || it->second == "false" || it->second == "0")
            {
                outValue = false;
                return true;
            }

            return false;
        }

        SimulationIntentType parseIntentType(const std::string& value)
        {
            if (value == "move") return SimulationIntentType::MoveToTile;
            if (value == "inspect") return SimulationIntentType::InspectTile;
            if (value == "interact") return SimulationIntentType::InteractTile;
            return SimulationIntentType::InspectTile;
        }

        SimulationIntentAckResult parseAckResult(const std::string& value)
        {
            if (value == "accepted") return SimulationIntentAckResult::Accepted;
            if (value == "rejected") return SimulationIntentAckResult::Rejected;
            return SimulationIntentAckResult::None;
        }

        std::vector<std::filesystem::path> enumerateFilesByPrefix(const std::filesystem::path& directory, const char* prefix)
        {
            std::vector<std::filesystem::path> files{};
            std::error_code error;
            if (!std::filesystem::exists(directory, error))
            {
                return files;
            }

            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory, error))
            {
                if (error || !entry.is_regular_file())
                {
                    continue;
                }

                const std::string filename = entry.path().filename().string();
                if (filename.rfind(prefix, 0) == 0)
                {
                    files.push_back(entry.path());
                }
            }

            std::sort(files.begin(), files.end());
            return files;
        }

        void removeFileQuietly(const std::filesystem::path& path)
        {
            std::error_code error;
            std::filesystem::remove(path, error);
        }

        bool writeTextFileAtomically(const std::filesystem::path& path, const std::string& contents, std::string& outError)
        {
            outError.clear();
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error)
            {
                outError = std::string("Failed to prepare directory: ") + path.parent_path().generic_string();
                return false;
            }

            const std::filesystem::path tempPath = path.parent_path() / (path.filename().string() + ".tmp");
            std::ofstream output(tempPath, std::ios::out | std::ios::trunc);
            if (!output.is_open())
            {
                outError = std::string("Failed to open temp file for write: ") + tempPath.generic_string();
                return false;
            }

            output << contents;
            output.close();
            if (!output)
            {
                outError = std::string("Failed to flush temp file: ") + tempPath.generic_string();
                removeFileQuietly(tempPath);
                return false;
            }

#if defined(_WIN32)
            DWORD lastError = ERROR_SUCCESS;
            bool published = false;
            for (int attempt = 0; attempt < 12; ++attempt)
            {
                if (MoveFileExW(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                {
                    published = true;
                    break;
                }

                lastError = GetLastError();
                if (lastError != ERROR_SHARING_VIOLATION
                    && lastError != ERROR_LOCK_VIOLATION
                    && lastError != ERROR_ACCESS_DENIED)
                {
                    break;
                }

                Sleep(2);
            }

            if (!published)
            {
                outError = std::string("Failed to publish file atomically: ")
                    + path.generic_string()
                    + " (win32="
                    + std::to_string(static_cast<unsigned long>(lastError))
                    + ")";
                removeFileQuietly(tempPath);
                return false;
            }
#else
            std::filesystem::rename(tempPath, path, error);
            if (error)
            {
                outError = std::string("Failed to publish file atomically: ") + path.generic_string();
                removeFileQuietly(tempPath);
                return false;
            }
#endif
            return true;
        }

        std::string serializeEntity(const ReplicatedEntityState& entity)
        {
            std::ostringstream stream;
            stream << entity.id << '|'
                   << static_cast<int>(entity.type) << '|'
                   << entity.tile.x << '|'
                   << entity.tile.y << '|'
                   << (entity.isOpen ? 1 : 0) << '|'
                   << (entity.isLocked ? 1 : 0) << '|'
                   << (entity.isPowered ? 1 : 0) << '|'
                   << (entity.lootClaimed ? 1 : 0) << '|'
                   << sanitizeSingleLine(entity.lootProfileId) << '|'
                   << sanitizeSingleLine(entity.name);
            return stream.str();
        }

        bool parseEntity(const std::string& value, ReplicatedEntityState& outEntity)
        {
            std::stringstream stream(value);
            std::string idText;
            std::string typeText;
            std::string xText;
            std::string yText;
            std::string openText;
            std::string lockedText;
            std::string poweredText;
            std::string claimedText;
            std::string lootProfileText;
            std::string nameText;

            if (!std::getline(stream, idText, '|')
                || !std::getline(stream, typeText, '|')
                || !std::getline(stream, xText, '|')
                || !std::getline(stream, yText, '|')
                || !std::getline(stream, openText, '|')
                || !std::getline(stream, lockedText, '|')
                || !std::getline(stream, poweredText, '|')
                || !std::getline(stream, claimedText, '|')
                || !std::getline(stream, lootProfileText, '|')
                || !std::getline(stream, nameText))
            {
                return false;
            }

            try
            {
                outEntity.id = std::stoi(idText);
                outEntity.type = static_cast<EntityType>(std::stoi(typeText));
                outEntity.tile.x = std::stoi(xText);
                outEntity.tile.y = std::stoi(yText);
                outEntity.isOpen = openText == "1";
                outEntity.isLocked = lockedText == "1";
                outEntity.isPowered = poweredText == "1";
                outEntity.lootClaimed = claimedText == "1";
                outEntity.lootProfileId = lootProfileText;
                outEntity.name = nameText;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
    }

    AuthoritativeHostProtocolReport AuthoritativeHostProtocol::buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        AuthoritativeHostProtocolReport report{};
        report.intentQueueDirectory = intentQueueDirectory(runtimeBoundaryReport);
        report.acknowledgementQueueDirectory = acknowledgementQueueDirectory(runtimeBoundaryReport);
        report.snapshotPath = snapshotPath(runtimeBoundaryReport);

        report.intentQueueReady = directoryExists(report.intentQueueDirectory);
        report.acknowledgementQueueReady = directoryExists(report.acknowledgementQueueDirectory);
        report.snapshotPresent = fileExists(report.snapshotPath);
        report.authorityLaneReady = report.intentQueueReady && report.acknowledgementQueueReady;
        return report;
    }

    void AuthoritativeHostProtocol::ensureDirectories(AuthoritativeHostProtocolReport& report)
    {
        ensureDirectory(report.intentQueueDirectory, report.issues);
        ensureDirectory(report.acknowledgementQueueDirectory, report.issues);

        report.intentQueueReady = directoryExists(report.intentQueueDirectory);
        report.acknowledgementQueueReady = directoryExists(report.acknowledgementQueueDirectory);
        report.authorityLaneReady = report.intentQueueReady && report.acknowledgementQueueReady;
    }

    bool AuthoritativeHostProtocol::writeIntentRequest(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const SimulationIntent& intent,
        std::string& outError)
    {
        outError.clear();
        AuthoritativeHostProtocolReport report = buildReport(runtimeBoundaryReport);
        ensureDirectories(report);
        if (!report.issues.empty())
        {
            outError = report.issues.front();
            return false;
        }

        const std::filesystem::path path = report.intentQueueDirectory / ("intent_" + std::to_string(intent.sequence) + ".txt");
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            outError = std::string("Failed to open host intent request file: ") + path.generic_string();
            return false;
        }

        output
            << "version=1\n"
            << "sequence=" << intent.sequence << "\n"
            << "type=" << simulationIntentTypeText(intent.type) << "\n"
            << "target_x=" << intent.target.x << "\n"
            << "target_y=" << intent.target.y << "\n";
        return true;
    }

    std::vector<SimulationIntent> AuthoritativeHostProtocol::collectPendingIntentRequestsForHost(
        const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        std::vector<SimulationIntent> intents{};
        const std::vector<std::filesystem::path> files = enumerateFilesByPrefix(intentQueueDirectory(runtimeBoundaryReport), "intent_");

        for (const std::filesystem::path& path : files)
        {
            const StringMap values = parseKeyValueFile(path);
            if (values.empty())
            {
                removeFileQuietly(path);
                continue;
            }

            SimulationIntent intent{};
            uint64_t sequence = 0;
            int targetX = 0;
            int targetY = 0;
            std::string typeText;

            if (!tryParseUnsigned(values, "sequence", sequence)
                || !tryParseString(values, "type", typeText)
                || !tryParseInt(values, "target_x", targetX)
                || !tryParseInt(values, "target_y", targetY))
            {
                removeFileQuietly(path);
                continue;
            }

            intent.sequence = sequence;
            intent.type = parseIntentType(typeText);
            intent.target.x = targetX;
            intent.target.y = targetY;
            intents.push_back(intent);
            removeFileQuietly(path);
        }

        std::sort(intents.begin(), intents.end(), [](const SimulationIntent& lhs, const SimulationIntent& rhs)
        {
            return lhs.sequence < rhs.sequence;
        });
        return intents;
    }

    bool AuthoritativeHostProtocol::writeAcknowledgement(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const SimulationIntentAck& acknowledgement,
        std::string& outError)
    {
        outError.clear();
        AuthoritativeHostProtocolReport report = buildReport(runtimeBoundaryReport);
        ensureDirectories(report);
        if (!report.issues.empty())
        {
            outError = report.issues.front();
            return false;
        }

        const std::filesystem::path path = report.acknowledgementQueueDirectory / ("ack_" + std::to_string(acknowledgement.sequence) + ".txt");
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            outError = std::string("Failed to open host acknowledgement file: ") + path.generic_string();
            return false;
        }

        output
            << "version=1\n"
            << "sequence=" << acknowledgement.sequence << "\n"
            << "type=" << simulationIntentTypeText(acknowledgement.type) << "\n"
            << "result=" << simulationIntentAckResultText(acknowledgement.result) << "\n"
            << "reason=" << sanitizeSingleLine(acknowledgement.reason) << "\n"
            << "host_ticks=" << acknowledgement.hostSimulationTicks << "\n"
            << "published_epoch_ms=" << acknowledgement.publishedEpochMilliseconds << "\n"
            << "target_x=" << acknowledgement.target.x << "\n"
            << "target_y=" << acknowledgement.target.y << "\n";
        return true;
    }

    std::vector<SimulationIntentAck> AuthoritativeHostProtocol::collectAcknowledgementsForClient(
        const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        std::vector<SimulationIntentAck> acknowledgements{};
        const std::vector<std::filesystem::path> files = enumerateFilesByPrefix(acknowledgementQueueDirectory(runtimeBoundaryReport), "ack_");

        for (const std::filesystem::path& path : files)
        {
            const StringMap values = parseKeyValueFile(path);
            if (values.empty())
            {
                removeFileQuietly(path);
                continue;
            }

            SimulationIntentAck acknowledgement{};
            std::string typeText;
            std::string resultText;
            uint64_t sequence = 0;
            uint64_t hostTicks = 0;
            uint64_t publishedEpochMilliseconds = 0;
            int targetX = 0;
            int targetY = 0;

            if (!tryParseUnsigned(values, "sequence", sequence)
                || !tryParseString(values, "type", typeText)
                || !tryParseString(values, "result", resultText)
                || !tryParseUnsigned(values, "host_ticks", hostTicks)
                || !tryParseUnsigned(values, "published_epoch_ms", publishedEpochMilliseconds)
                || !tryParseInt(values, "target_x", targetX)
                || !tryParseInt(values, "target_y", targetY))
            {
                removeFileQuietly(path);
                continue;
            }

            acknowledgement.sequence = sequence;
            acknowledgement.type = parseIntentType(typeText);
            acknowledgement.result = parseAckResult(resultText);
            acknowledgement.reason = values.contains("reason") ? values.at("reason") : std::string{};
            acknowledgement.hostSimulationTicks = hostTicks;
            acknowledgement.publishedEpochMilliseconds = publishedEpochMilliseconds;
            acknowledgement.target.x = targetX;
            acknowledgement.target.y = targetY;

            acknowledgements.push_back(acknowledgement);
            removeFileQuietly(path);
        }

        std::sort(acknowledgements.begin(), acknowledgements.end(), [](const SimulationIntentAck& lhs, const SimulationIntentAck& rhs)
        {
            return lhs.sequence < rhs.sequence;
        });
        return acknowledgements;
    }

    bool AuthoritativeHostProtocol::writeAuthoritativeSnapshot(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const AuthoritativeWorldSnapshot& snapshot,
        std::string& outError)
    {
        return writeAuthoritativeSnapshotToPath(snapshotPath(runtimeBoundaryReport), snapshot, outError);
    }

    AuthoritativeWorldSnapshot AuthoritativeHostProtocol::readAuthoritativeSnapshot(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        std::string& outError)
    {
        return readAuthoritativeSnapshotFromPath(snapshotPath(runtimeBoundaryReport), outError);
    }

    bool AuthoritativeHostProtocol::writeAuthoritativeSnapshotToPath(
        const std::filesystem::path& path,
        const AuthoritativeWorldSnapshot& snapshot,
        std::string& outError)
    {
        const uint32_t schemaVersion = snapshot.persistenceSchemaVersion > 0
            ? snapshot.persistenceSchemaVersion
            : 3u;
        const uint32_t migratedFromSchemaVersion = snapshot.persistenceMigratedFromSchemaVersion;
        const uint64_t nextIntentSequence = snapshot.nextIntentSequence > 0
            ? snapshot.nextIntentSequence
            : snapshot.lastProcessedIntentSequence + 1ull;
        const uint64_t saveEpochMilliseconds = snapshot.persistenceEpochMilliseconds > 0
            ? snapshot.persistenceEpochMilliseconds
            : snapshot.playerActorState.lastSaveEpochMilliseconds;
        const std::string saveSlot = sanitizeSingleLine(
            snapshot.persistenceSlotName.empty() ? std::string("primary") : snapshot.persistenceSlotName);

        std::ostringstream output;
        output
            << "version=5\n"
            << "schema_version=" << schemaVersion << "\n"
            << "migrated_from_schema_version=" << migratedFromSchemaVersion << "\n"
            << "persistence_schema_version=" << schemaVersion << "\n"
            << "persistence_loaded_schema_version=" << snapshot.persistenceLoadedSchemaVersion << "\n"
            << "persistence_migrated_from_version=" << migratedFromSchemaVersion << "\n"
            << "save_slot=" << saveSlot << "\n"
            << "save_epoch_ms=" << saveEpochMilliseconds << "\n"
            << "valid=" << (snapshot.valid ? "yes" : "no") << "\n"
            << "simulation_ticks=" << snapshot.simulationTicks << "\n"
            << "last_processed_intent_sequence=" << snapshot.lastProcessedIntentSequence << "\n"
            << "next_intent_sequence=" << nextIntentSequence << "\n"
            << "published_epoch_ms=" << snapshot.publishedEpochMilliseconds << "\n"
            << "player_x=" << snapshot.authoritativePlayerPosition.x << "\n"
            << "player_y=" << snapshot.authoritativePlayerPosition.y << "\n"
            << "movement_target_active=" << (snapshot.movementTargetActive ? "yes" : "no") << "\n"
            << "movement_target_x=" << snapshot.movementTargetTile.x << "\n"
            << "movement_target_y=" << snapshot.movementTargetTile.y << "\n"
            << "path_index=" << snapshot.pathIndex << "\n"
            << "path_count=" << snapshot.currentPath.size() << "\n";

        for (size_t i = 0; i < snapshot.currentPath.size(); ++i)
        {
            output << "path_" << i << '=' << snapshot.currentPath[i].x << ',' << snapshot.currentPath[i].y << "\n";
        }

        output
            << "player_health=" << snapshot.playerActorState.health << "\n"
            << "player_health_current=" << snapshot.playerActorState.health << "\n"
            << "player_max_health=" << snapshot.playerActorState.maxHealth << "\n"
            << "player_health_max=" << snapshot.playerActorState.maxHealth << "\n"
            << "player_armor=" << snapshot.playerActorState.armor << "\n"
            << "suit_integrity=" << snapshot.playerActorState.suitIntegrity << "\n"
            << "oxygen_seconds=" << snapshot.playerActorState.oxygenSecondsRemaining << "\n"
            << "radiation_dose=" << snapshot.playerActorState.radiationDose << "\n"
            << "toxic_exposure=" << snapshot.playerActorState.toxicExposure << "\n"
            << "loot_collections=" << snapshot.playerActorState.lootCollections << "\n"
            << "encounter_wins=" << snapshot.playerActorState.encounterWins << "\n"
            << "encounters_survived=" << snapshot.playerActorState.encountersSurvived << "\n"
            << "last_save_epoch_ms=" << snapshot.playerActorState.lastSaveEpochMilliseconds << "\n"
            << "equip_weapon=" << static_cast<int>(snapshot.playerActorState.equipment.weapon) << "\n"
            << "equipment_weapon=" << static_cast<int>(snapshot.playerActorState.equipment.weapon) << "\n"
            << "equip_suit=" << static_cast<int>(snapshot.playerActorState.equipment.suit) << "\n"
            << "equipment_suit=" << static_cast<int>(snapshot.playerActorState.equipment.suit) << "\n"
            << "equip_tool=" << static_cast<int>(snapshot.playerActorState.equipment.tool) << "\n"
            << "equipment_tool=" << static_cast<int>(snapshot.playerActorState.equipment.tool) << "\n"
            << "inventory_count=" << snapshot.playerActorState.inventory.size() << "\n";

        for (size_t i = 0; i < snapshot.playerActorState.inventory.size(); ++i)
        {
            output << "inventory_" << i << '='
                   << static_cast<int>(snapshot.playerActorState.inventory[i].id)
                   << '|'
                   << snapshot.playerActorState.inventory[i].quantity
                   << "\n";
        }

        output
            << "combat_active=" << (snapshot.combatEncounterState.active ? "yes" : "no") << "\n"
            << "combat_label=" << sanitizeSingleLine(snapshot.combatEncounterState.label) << "\n"
            << "combat_id=" << sanitizeSingleLine(snapshot.combatEncounterState.encounterId) << "\n"
            << "combat_anchor_x=" << snapshot.combatEncounterState.anchorTile.x << "\n"
            << "combat_anchor_y=" << snapshot.combatEncounterState.anchorTile.y << "\n"
            << "combat_round=" << snapshot.combatEncounterState.roundNumber << "\n"
            << "combat_ticks_remaining=" << snapshot.combatEncounterState.roundTicksRemaining << "\n"
            << "combat_hostile_label=" << sanitizeSingleLine(snapshot.combatEncounterState.hostileLabel) << "\n"
            << "combat_hostile_health=" << snapshot.combatEncounterState.hostileHealth << "\n"
            << "combat_hostile_max_health=" << snapshot.combatEncounterState.hostileMaxHealth << "\n"
            << "combat_hostile_armor=" << snapshot.combatEncounterState.hostileArmor << "\n"
            << "combat_hostile_min_damage=" << snapshot.combatEncounterState.hostileMinDamage << "\n"
            << "combat_hostile_max_damage=" << snapshot.combatEncounterState.hostileMaxDamage << "\n"
            << "combat_last_outcome=" << sanitizeSingleLine(snapshot.combatEncounterState.lastOutcome) << "\n"
            << "hazard_ticks=" << snapshot.hazardTicks << "\n"
            << "terrain_consequence_events=" << snapshot.terrainConsequenceEvents << "\n"
            << "current_hazard_label=" << sanitizeSingleLine(snapshot.currentHazardLabel) << "\n"
            << "current_terrain_consequence=" << sanitizeSingleLine(snapshot.currentTerrainConsequence) << "\n"
            << "entity_count=" << snapshot.entities.size() << "\n";

        for (size_t i = 0; i < snapshot.entities.size(); ++i)
        {
            output << "entity_" << i << '=' << serializeEntity(snapshot.entities[i]) << "\n";
        }

        output << "event_count=" << snapshot.eventLog.size() << "\n";
        for (size_t i = 0; i < snapshot.eventLog.size(); ++i)
        {
            output << "event_" << i << '=' << sanitizeSingleLine(snapshot.eventLog[i]) << "\n";
        }

        return writeTextFileAtomically(path, output.str(), outError);
    }

    AuthoritativeWorldSnapshot AuthoritativeHostProtocol::readAuthoritativeSnapshotFromPath(
        const std::filesystem::path& path,
        std::string& outError)
    {
        outError.clear();
        AuthoritativeWorldSnapshot snapshot{};
        if (!fileExists(path))
        {
            outError = "Authoritative snapshot not present.";
            return snapshot;
        }

        const StringMap values = parseKeyValueFile(path);
        if (values.empty())
        {
            outError = "Authoritative snapshot file could not be parsed.";
            return snapshot;
        }

        uint64_t version = 0;
        uint64_t schemaVersion = 0;
        uint64_t migratedFromSchemaVersion = 0;
        uint64_t simulationTicks = 0;
        uint64_t lastProcessedIntentSequence = 0;
        uint64_t nextIntentSequence = 0;
        uint64_t publishedEpochMilliseconds = 0;
        uint64_t saveEpochMilliseconds = 0;
        bool valid = false;
        float playerX = 0.0f;
        float playerY = 0.0f;
        bool movementTargetActive = false;
        int movementTargetX = 0;
        int movementTargetY = 0;
        uint64_t pathIndex = 0;
        uint64_t pathCount = 0;
        std::string saveSlot = "primary";

        if (!tryParseUnsigned(values, "version", version)
            || !tryParseYesNo(values, "valid", valid)
            || !tryParseUnsigned(values, "simulation_ticks", simulationTicks)
            || !tryParseUnsigned(values, "last_processed_intent_sequence", lastProcessedIntentSequence)
            || !tryParseUnsigned(values, "published_epoch_ms", publishedEpochMilliseconds)
            || !tryParseFloat(values, "player_x", playerX)
            || !tryParseFloat(values, "player_y", playerY)
            || !tryParseYesNo(values, "movement_target_active", movementTargetActive)
            || !tryParseInt(values, "movement_target_x", movementTargetX)
            || !tryParseInt(values, "movement_target_y", movementTargetY)
            || !tryParseUnsigned(values, "path_index", pathIndex)
            || !tryParseUnsigned(values, "path_count", pathCount))
        {
            outError = "Authoritative snapshot missing required fields.";
            return snapshot;
        }

        snapshot.valid = valid;
        snapshot.simulationTicks = simulationTicks;
        snapshot.lastProcessedIntentSequence = lastProcessedIntentSequence;
        snapshot.nextIntentSequence = lastProcessedIntentSequence + 1ull;
        snapshot.publishedEpochMilliseconds = publishedEpochMilliseconds;
        snapshot.authoritativePlayerPosition.x = playerX;
        snapshot.authoritativePlayerPosition.y = playerY;
        snapshot.movementTargetActive = movementTargetActive;
        snapshot.movementTargetTile.x = movementTargetX;
        snapshot.movementTargetTile.y = movementTargetY;
        snapshot.pathIndex = static_cast<size_t>(pathIndex);

        const uint32_t loadedSchemaVersion = tryParseUnsigned(values, "schema_version", schemaVersion)
            ? static_cast<uint32_t>(schemaVersion)
            : (version <= 1ull ? 1u : 3u);
        snapshot.persistenceLoadedSchemaVersion = loadedSchemaVersion;
        snapshot.persistenceSchemaVersion = loadedSchemaVersion < 3u ? 3u : loadedSchemaVersion;

        if (tryParseUnsigned(values, "migrated_from_schema_version", migratedFromSchemaVersion)
            || tryParseUnsigned(values, "persistence_migrated_from_version", migratedFromSchemaVersion))
        {
            snapshot.persistenceMigratedFromSchemaVersion = static_cast<uint32_t>(migratedFromSchemaVersion);
        }
        else if (loadedSchemaVersion < snapshot.persistenceSchemaVersion)
        {
            snapshot.persistenceMigratedFromSchemaVersion = loadedSchemaVersion;
        }

        if (tryParseUnsigned(values, "next_intent_sequence", nextIntentSequence) && nextIntentSequence > 0)
        {
            snapshot.nextIntentSequence = nextIntentSequence;
        }

        if (tryParseUnsigned(values, "save_epoch_ms", saveEpochMilliseconds))
        {
            snapshot.persistenceEpochMilliseconds = saveEpochMilliseconds;
        }
        else
        {
            snapshot.persistenceEpochMilliseconds = 0;
        }

        if (tryParseString(values, "save_slot", saveSlot) && !saveSlot.empty())
        {
            snapshot.persistenceSlotName = saveSlot;
        }

        const bool legacyActorSurface = loadedSchemaVersion <= 2u;

        for (size_t i = 0; i < static_cast<size_t>(pathCount); ++i)
        {
            const std::string key = "path_" + std::to_string(i);
            const auto it = values.find(key);
            if (it == values.end())
            {
                outError = std::string("Authoritative snapshot missing path entry: ") + key;
                snapshot.valid = false;
                return snapshot;
            }

            const size_t split = it->second.find(',');
            if (split == std::string::npos)
            {
                outError = std::string("Authoritative snapshot path entry malformed: ") + key;
                snapshot.valid = false;
                return snapshot;
            }

            try
            {
                snapshot.currentPath.push_back({
                    std::stoi(it->second.substr(0, split)),
                    std::stoi(it->second.substr(split + 1))
                });
            }
            catch (...)
            {
                outError = std::string("Authoritative snapshot path entry invalid: ") + key;
                snapshot.valid = false;
                return snapshot;
            }
        }

        auto hasKey = [&](const char* key) -> bool
        {
            return values.find(key) != values.end();
        };

        auto parseOrFailInt = [&](const char* key, int& value) -> bool
        {
            if (!tryParseInt(values, key, value))
            {
                outError = std::string("Authoritative snapshot missing integer field: ") + key;
                return false;
            }
            return true;
        };

        auto parseOrFailUnsigned = [&](const char* key, uint64_t& value) -> bool
        {
            if (!tryParseUnsigned(values, key, value))
            {
                outError = std::string("Authoritative snapshot missing unsigned field: ") + key;
                return false;
            }
            return true;
        };

        auto parseIntAlias = [&](const char* primary, const char* alias, int& outValue) -> bool
        {
            return tryParseInt(values, primary, outValue) || (alias != nullptr && tryParseInt(values, alias, outValue));
        };

        auto parseUnsignedAlias = [&](const char* primary, const char* alias, uint64_t& outValue) -> bool
        {
            return tryParseUnsigned(values, primary, outValue) || (alias != nullptr && tryParseUnsigned(values, alias, outValue));
        };

        auto parseOptionalCombatString = [&](const char* key, std::string& outValue, const std::string& fallback)
        {
            if (!tryParseString(values, key, outValue))
            {
                outValue = fallback;
            }
        };

        if (legacyActorSurface)
        {
            snapshot.playerActorState.health = 100;
            snapshot.playerActorState.maxHealth = 100;
            snapshot.playerActorState.armor = 25;
            snapshot.playerActorState.suitIntegrity = 100;
            snapshot.playerActorState.oxygenSecondsRemaining = 300.0f;
            snapshot.playerActorState.radiationDose = 0;
            snapshot.playerActorState.toxicExposure = 0;
            snapshot.playerActorState.lootCollections = 0;
            snapshot.playerActorState.encounterWins = 0;
            snapshot.playerActorState.encountersSurvived = 0;
            snapshot.playerActorState.lastSaveEpochMilliseconds = snapshot.persistenceEpochMilliseconds;
            snapshot.playerActorState.equipment.weapon = InventoryItemId::None;
            snapshot.playerActorState.equipment.suit = InventoryItemId::UtilitySuit;
            snapshot.playerActorState.equipment.tool = InventoryItemId::None;
            snapshot.playerActorState.inventory.push_back({ InventoryItemId::RationPack, 1u });
        }
        else if (!parseIntAlias("player_health", "player_health_current", snapshot.playerActorState.health)
            || !parseIntAlias("player_max_health", "player_health_max", snapshot.playerActorState.maxHealth)
            || !parseOrFailInt("player_armor", snapshot.playerActorState.armor)
            || !parseOrFailInt("suit_integrity", snapshot.playerActorState.suitIntegrity)
            || !tryParseFloat(values, "oxygen_seconds", snapshot.playerActorState.oxygenSecondsRemaining)
            || !parseOrFailInt("radiation_dose", snapshot.playerActorState.radiationDose)
            || !parseOrFailInt("toxic_exposure", snapshot.playerActorState.toxicExposure)
            || !parseOrFailUnsigned("loot_collections", snapshot.playerActorState.lootCollections)
            || !parseOrFailUnsigned("encounter_wins", snapshot.playerActorState.encounterWins)
            || !parseOrFailUnsigned("encounters_survived", snapshot.playerActorState.encountersSurvived)
            || !parseOrFailUnsigned("last_save_epoch_ms", snapshot.playerActorState.lastSaveEpochMilliseconds))
        {
            outError = "Authoritative snapshot missing actor runtime fields.";
            snapshot.valid = false;
            return snapshot;
        }

        int equipWeapon = 0;
        int equipSuit = 0;
        int equipTool = 0;
        uint64_t inventoryCount = 0;
        if (!legacyActorSurface
            && (!parseIntAlias("equip_weapon", "equipment_weapon", equipWeapon)
                || !parseIntAlias("equip_suit", "equipment_suit", equipSuit)
                || !parseIntAlias("equip_tool", "equipment_tool", equipTool)
                || !parseOrFailUnsigned("inventory_count", inventoryCount)))
        {
            snapshot.valid = false;
            return snapshot;
        }

        if (!legacyActorSurface)
        {
            snapshot.playerActorState.equipment.weapon = static_cast<InventoryItemId>(equipWeapon);
            snapshot.playerActorState.equipment.suit = static_cast<InventoryItemId>(equipSuit);
            snapshot.playerActorState.equipment.tool = static_cast<InventoryItemId>(equipTool);
        }

        for (size_t i = 0; i < static_cast<size_t>(inventoryCount); ++i)
        {
            const std::string key = "inventory_" + std::to_string(i);
            const auto it = values.find(key);
            if (it == values.end())
            {
                outError = std::string("Authoritative snapshot missing inventory entry: ") + key;
                snapshot.valid = false;
                return snapshot;
            }

            const size_t split = it->second.find('|');
            if (split == std::string::npos)
            {
                outError = std::string("Authoritative snapshot inventory entry malformed: ") + key;
                snapshot.valid = false;
                return snapshot;
            }

            try
            {
                InventoryItemStack stack{};
                stack.id = static_cast<InventoryItemId>(std::stoi(it->second.substr(0, split)));
                stack.quantity = static_cast<uint32_t>(std::stoul(it->second.substr(split + 1)));
                snapshot.playerActorState.inventory.push_back(stack);
            }
            catch (...)
            {
                outError = std::string("Authoritative snapshot inventory entry invalid: ") + key;
                snapshot.valid = false;
                return snapshot;
            }
        }

        bool combatActive = false;
        std::string combatLabel;
        std::string combatId;
        int combatAnchorX = 0;
        int combatAnchorY = 0;
        uint64_t combatRound = 0;
        uint64_t combatTicksRemaining = 0;
        std::string hostileLabel;
        int hostileHealth = 0;
        int hostileMaxHealth = 0;
        int hostileArmor = 0;
        int hostileMinDamage = 0;
        int hostileMaxDamage = 0;
        std::string combatLastOutcome;

        (void)tryParseYesNo(values, "combat_active", combatActive);
        parseOptionalCombatString("combat_label", combatLabel, "");
        parseOptionalCombatString("combat_id", combatId, "");
        (void)tryParseInt(values, "combat_anchor_x", combatAnchorX);
        (void)tryParseInt(values, "combat_anchor_y", combatAnchorY);
        (void)tryParseUnsigned(values, "combat_round", combatRound);
        (void)tryParseUnsigned(values, "combat_ticks_remaining", combatTicksRemaining);
        parseOptionalCombatString("combat_hostile_label", hostileLabel, "");
        (void)tryParseInt(values, "combat_hostile_health", hostileHealth);
        (void)tryParseInt(values, "combat_hostile_max_health", hostileMaxHealth);
        (void)tryParseInt(values, "combat_hostile_armor", hostileArmor);
        (void)tryParseInt(values, "combat_hostile_min_damage", hostileMinDamage);
        (void)tryParseInt(values, "combat_hostile_max_damage", hostileMaxDamage);
        parseOptionalCombatString("combat_last_outcome", combatLastOutcome, "");

        snapshot.combatEncounterState.active = combatActive;
        snapshot.combatEncounterState.label = combatLabel;
        snapshot.combatEncounterState.encounterId = combatId;
        snapshot.combatEncounterState.anchorTile = { combatAnchorX, combatAnchorY };
        snapshot.combatEncounterState.roundNumber = static_cast<uint32_t>(combatRound);
        snapshot.combatEncounterState.roundTicksRemaining = static_cast<uint32_t>(combatTicksRemaining);
        snapshot.combatEncounterState.hostileLabel = hostileLabel;
        snapshot.combatEncounterState.hostileHealth = hostileHealth;
        snapshot.combatEncounterState.hostileMaxHealth = hostileMaxHealth;
        snapshot.combatEncounterState.hostileArmor = hostileArmor;
        snapshot.combatEncounterState.hostileMinDamage = hostileMinDamage;
        snapshot.combatEncounterState.hostileMaxDamage = hostileMaxDamage;
        snapshot.combatEncounterState.lastOutcome = combatLastOutcome;

        (void)parseUnsignedAlias("hazard_ticks", nullptr, snapshot.hazardTicks);
        (void)parseUnsignedAlias("terrain_consequence_events", nullptr, snapshot.terrainConsequenceEvents);
        if (!tryParseString(values, "current_hazard_label", snapshot.currentHazardLabel))
        {
            snapshot.currentHazardLabel = "none";
        }
        if (!tryParseString(values, "current_terrain_consequence", snapshot.currentTerrainConsequence))
        {
            snapshot.currentTerrainConsequence = "stable";
        }

        uint64_t entityCount = 0;
        if (!parseOrFailUnsigned("entity_count", entityCount))
        {
            snapshot.valid = false;
            return snapshot;
        }

        for (size_t i = 0; i < static_cast<size_t>(entityCount); ++i)
        {
            const std::string key = "entity_" + std::to_string(i);
            const auto it = values.find(key);
            if (it == values.end())
            {
                outError = std::string("Authoritative snapshot missing entity entry: ") + key;
                snapshot.valid = false;
                return snapshot;
            }

            ReplicatedEntityState entity{};
            if (!parseEntity(it->second, entity))
            {
                outError = std::string("Authoritative snapshot invalid entity entry: ") + key;
                snapshot.valid = false;
                return snapshot;
            }

            snapshot.entities.push_back(entity);
        }

        uint64_t eventCount = 0;
        if (!parseOrFailUnsigned("event_count", eventCount))
        {
            snapshot.valid = false;
            return snapshot;
        }

        for (size_t i = 0; i < static_cast<size_t>(eventCount); ++i)
        {
            const std::string key = "event_" + std::to_string(i);
            const auto it = values.find(key);
            if (it == values.end())
            {
                outError = std::string("Authoritative snapshot missing event entry: ") + key;
                snapshot.valid = false;
                return snapshot;
            }

            snapshot.eventLog.push_back(it->second);
        }

        return snapshot;
    }

    std::filesystem::path AuthoritativeHostProtocol::intentQueueDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        return runtimeBoundaryReport.hostDirectory / "IntentQueue";
    }

    std::filesystem::path AuthoritativeHostProtocol::acknowledgementQueueDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        return runtimeBoundaryReport.hostDirectory / "AckQueue";
    }

    std::filesystem::path AuthoritativeHostProtocol::snapshotPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        return runtimeBoundaryReport.hostDirectory / "authoritative_snapshot.txt";
    }
}
