#include "engine/host/AuthoritativeHostProtocol.h"

#include <algorithm>
#include <cctype>
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
        constexpr uint32_t kCurrentPersistenceSchemaVersion = 7u;

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

        std::string sanitizeFileToken(const std::string& value)
        {
            if (value.empty())
            {
                return "shared";
            }

            std::string sanitized{};
            sanitized.reserve(value.size());
            for (unsigned char ch : value)
            {
                if (std::isalnum(ch) != 0 || ch == '-' || ch == '_')
                {
                    sanitized.push_back(static_cast<char>(ch));
                }
                else
                {
                    sanitized.push_back('-');
                }
            }

            while (!sanitized.empty() && sanitized.back() == '-')
            {
                sanitized.pop_back();
            }

            return sanitized.empty() ? std::string("shared") : sanitized;
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

        const std::filesystem::path path = report.intentQueueDirectory
            / ("intent_" + sanitizeFileToken(intent.clientInstanceId) + "_" + std::to_string(intent.sequence) + ".txt");
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            outError = std::string("Failed to open host intent request file: ") + path.generic_string();
            return false;
        }

        output
            << "version=1\n"
            << "sequence=" << intent.sequence << "\n"
            << "client_instance_id=" << sanitizeSingleLine(intent.clientInstanceId) << "\n"
            << "session_id=" << sanitizeSingleLine(intent.sessionId) << "\n"
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
            std::string clientInstanceId;
            std::string sessionId;

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
            (void)tryParseString(values, "client_instance_id", clientInstanceId);
            (void)tryParseString(values, "session_id", sessionId);
            intent.clientInstanceId = clientInstanceId;
            intent.sessionId = sessionId;
            intent.target.x = targetX;
            intent.target.y = targetY;
            intents.push_back(intent);
            removeFileQuietly(path);
        }

        std::sort(intents.begin(), intents.end(), [](const SimulationIntent& lhs, const SimulationIntent& rhs)
        {
            if (lhs.sequence != rhs.sequence)
            {
                return lhs.sequence < rhs.sequence;
            }

            return lhs.clientInstanceId < rhs.clientInstanceId;
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

        const std::filesystem::path path = report.acknowledgementQueueDirectory
            / ("ack_" + sanitizeFileToken(acknowledgement.clientInstanceId) + "_" + std::to_string(acknowledgement.sequence) + ".txt");
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output.is_open())
        {
            outError = std::string("Failed to open host acknowledgement file: ") + path.generic_string();
            return false;
        }

        output
            << "version=1\n"
            << "sequence=" << acknowledgement.sequence << "\n"
            << "client_instance_id=" << sanitizeSingleLine(acknowledgement.clientInstanceId) << "\n"
            << "session_id=" << sanitizeSingleLine(acknowledgement.sessionId) << "\n"
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
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const std::string& clientInstanceId)
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
            std::string ackClientInstanceId;
            std::string ackSessionId;
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
            (void)tryParseString(values, "client_instance_id", ackClientInstanceId);
            (void)tryParseString(values, "session_id", ackSessionId);
            acknowledgement.clientInstanceId = ackClientInstanceId;
            acknowledgement.sessionId = ackSessionId;
            acknowledgement.target.x = targetX;
            acknowledgement.target.y = targetY;

            if (!clientInstanceId.empty()
                && !acknowledgement.clientInstanceId.empty()
                && acknowledgement.clientInstanceId != clientInstanceId)
            {
                continue;
            }

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
            : kCurrentPersistenceSchemaVersion;
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
            << "version=7\n"
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
            << "mission_id=" << static_cast<int>(snapshot.missionRuntimeState.activeMission) << "\n"
            << "mission_phase=" << static_cast<int>(snapshot.missionRuntimeState.phase) << "\n"
            << "mission_advancement_count=" << snapshot.missionRuntimeState.advancementCount << "\n"
            << "mission_transit_data_recovered=" << (snapshot.missionRuntimeState.transitDataRecovered ? "yes" : "no") << "\n"
            << "mission_medlab_diagnosis_complete=" << (snapshot.missionRuntimeState.medlabDiagnosisComplete ? "yes" : "no") << "\n"
            << "mission_quarantine_gate_unlocked=" << (snapshot.missionRuntimeState.quarantineGateUnlocked ? "yes" : "no") << "\n"
            << "mission_quarantine_encounter_resolved=" << (snapshot.missionRuntimeState.quarantineEncounterResolved ? "yes" : "no") << "\n"
            << "mission_quarantine_control_restored=" << (snapshot.missionRuntimeState.quarantineControlRestored ? "yes" : "no") << "\n"
            << "mission_ship_runtime_prep_ready=" << (snapshot.missionRuntimeState.shipRuntimePrepReady ? "yes" : "no") << "\n"
            << "mission_orbital_departure_complete=" << (snapshot.missionRuntimeState.orbitalDepartureComplete ? "yes" : "no") << "\n"
            << "mission_survey_orbit_reached=" << (snapshot.missionRuntimeState.surveyOrbitReached ? "yes" : "no") << "\n"
            << "mission_relay_track_stabilized=" << (snapshot.missionRuntimeState.relayTrackStabilized ? "yes" : "no") << "\n"
            << "mission_relay_platform_docked=" << (snapshot.missionRuntimeState.relayPlatformDocked ? "yes" : "no") << "\n"
            << "mission_dust_frontier_landed=" << (snapshot.missionRuntimeState.dustFrontierLanded ? "yes" : "no") << "\n"
            << "mission_frontier_relay_secured=" << (snapshot.missionRuntimeState.frontierRelaySecured ? "yes" : "no") << "\n"
            << "mission_return_launch_complete=" << (snapshot.missionRuntimeState.returnLaunchComplete ? "yes" : "no") << "\n"
            << "mission_home_dock_restored=" << (snapshot.missionRuntimeState.homeDockRestored ? "yes" : "no") << "\n"
            << "mission_return_loop_complete=" << (snapshot.missionRuntimeState.returnLoopComplete ? "yes" : "no") << "\n"
            << "mission_complete=" << (snapshot.missionRuntimeState.missionComplete ? "yes" : "no") << "\n"
            << "mission_last_beat=" << sanitizeSingleLine(snapshot.missionRuntimeState.lastBeat) << "\n"
            << "ship_id=" << static_cast<int>(snapshot.shipRuntimeState.activeShip) << "\n"
            << "ship_name=" << sanitizeSingleLine(snapshot.shipRuntimeState.shipName) << "\n"
            << "ship_location=" << sanitizeSingleLine(snapshot.shipRuntimeState.locationLabel) << "\n"
            << "ship_ownership_state=" << static_cast<int>(snapshot.shipRuntimeState.ownershipState) << "\n"
            << "ship_occupancy_state=" << static_cast<int>(snapshot.shipRuntimeState.occupancyState) << "\n"
            << "ship_boarding_count=" << snapshot.shipRuntimeState.boardingCount << "\n"
            << "ship_boarding_unlocked=" << (snapshot.shipRuntimeState.boardingUnlocked ? "yes" : "no") << "\n"
            << "ship_docked=" << (snapshot.shipRuntimeState.docked ? "yes" : "no") << "\n"
            << "ship_player_boarded=" << (snapshot.shipRuntimeState.playerBoarded ? "yes" : "no") << "\n"
            << "ship_power_online=" << (snapshot.shipRuntimeState.powerOnline ? "yes" : "no") << "\n"
            << "ship_airlock_pressurized=" << (snapshot.shipRuntimeState.airlockPressurized ? "yes" : "no") << "\n"
            << "ship_command_claimed=" << (snapshot.shipRuntimeState.commandClaimed ? "yes" : "no") << "\n"
            << "ship_launch_prep_ready=" << (snapshot.shipRuntimeState.launchPrepReady ? "yes" : "no") << "\n"
            << "ship_frontier_surface_access_unlocked=" << (snapshot.shipRuntimeState.frontierSurfaceAccessUnlocked ? "yes" : "no") << "\n"
            << "ship_frontier_surface_active=" << (snapshot.shipRuntimeState.frontierSurfaceActive ? "yes" : "no") << "\n"
            << "ship_last_beat=" << sanitizeSingleLine(snapshot.shipRuntimeState.lastBeat) << "\n"
            << "orbital_layer_unlocked=" << (snapshot.orbitalRuntimeState.orbitalLayerUnlocked ? "yes" : "no") << "\n"
            << "orbital_layer_active=" << (snapshot.orbitalRuntimeState.orbitalLayerActive ? "yes" : "no") << "\n"
            << "orbital_departure_authorized=" << (snapshot.orbitalRuntimeState.departureAuthorized ? "yes" : "no") << "\n"
            << "orbital_survey_orbit_reached=" << (snapshot.orbitalRuntimeState.surveyOrbitReached ? "yes" : "no") << "\n"
            << "orbital_relay_track_reached=" << (snapshot.orbitalRuntimeState.relayTrackReached ? "yes" : "no") << "\n"
            << "orbital_relay_platform_docked=" << (snapshot.orbitalRuntimeState.relayPlatformDocked ? "yes" : "no") << "\n"
            << "orbital_return_route_authorized=" << (snapshot.orbitalRuntimeState.returnRouteAuthorized ? "yes" : "no") << "\n"
            << "orbital_home_dock_reached=" << (snapshot.orbitalRuntimeState.homeDockReached ? "yes" : "no") << "\n"
            << "orbital_travel_in_progress=" << (snapshot.orbitalRuntimeState.travelInProgress ? "yes" : "no") << "\n"
            << "orbital_transfer_count=" << snapshot.orbitalRuntimeState.transferCount << "\n"
            << "orbital_travel_ticks_remaining=" << snapshot.orbitalRuntimeState.travelTicksRemaining << "\n"
            << "orbital_current_node=" << static_cast<int>(snapshot.orbitalRuntimeState.currentNode) << "\n"
            << "orbital_target_node=" << static_cast<int>(snapshot.orbitalRuntimeState.targetNode) << "\n"
            << "orbital_phase=" << static_cast<int>(snapshot.orbitalRuntimeState.phase) << "\n"
            << "orbital_rule_text=" << sanitizeSingleLine(snapshot.orbitalRuntimeState.ruleText) << "\n"
            << "orbital_last_beat=" << sanitizeSingleLine(snapshot.orbitalRuntimeState.lastBeat) << "\n"
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
        snapshot.persistenceSchemaVersion = loadedSchemaVersion < kCurrentPersistenceSchemaVersion
            ? kCurrentPersistenceSchemaVersion
            : loadedSchemaVersion;

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

        const bool missionSurfaceExpected =
            loadedSchemaVersion >= kCurrentPersistenceSchemaVersion
            || hasKey("mission_id")
            || hasKey("mission_phase");

        if (missionSurfaceExpected)
        {
            int missionId = 0;
            int missionPhase = 0;
            bool transitDataRecovered = false;
            bool medlabDiagnosisComplete = false;
            bool quarantineGateUnlocked = false;
            bool quarantineEncounterResolved = false;
            bool quarantineControlRestored = false;
            bool shipRuntimePrepReady = false;
            bool orbitalDepartureComplete = false;
            bool surveyOrbitReached = false;
            bool relayTrackStabilized = false;
            bool relayPlatformDocked = false;
            bool dustFrontierLanded = false;
            bool frontierRelaySecured = false;
            bool returnLaunchComplete = false;
            bool homeDockRestored = false;
            bool returnLoopComplete = false;
            bool missionComplete = false;

            if (!parseOrFailInt("mission_id", missionId)
                || !parseOrFailInt("mission_phase", missionPhase)
                || !parseOrFailUnsigned("mission_advancement_count", snapshot.missionRuntimeState.advancementCount)
                || !tryParseYesNo(values, "mission_transit_data_recovered", transitDataRecovered)
                || !tryParseYesNo(values, "mission_medlab_diagnosis_complete", medlabDiagnosisComplete)
                || !tryParseYesNo(values, "mission_quarantine_gate_unlocked", quarantineGateUnlocked)
                || !tryParseYesNo(values, "mission_quarantine_encounter_resolved", quarantineEncounterResolved)
                || !tryParseYesNo(values, "mission_quarantine_control_restored", quarantineControlRestored)
                || !tryParseYesNo(values, "mission_ship_runtime_prep_ready", shipRuntimePrepReady)
                || !(tryParseYesNo(values, "mission_orbital_departure_complete", orbitalDepartureComplete) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_survey_orbit_reached", surveyOrbitReached) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_relay_track_stabilized", relayTrackStabilized) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_relay_platform_docked", relayPlatformDocked) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_dust_frontier_landed", dustFrontierLanded) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_frontier_relay_secured", frontierRelaySecured) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_return_launch_complete", returnLaunchComplete) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_home_dock_restored", homeDockRestored) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "mission_return_loop_complete", returnLoopComplete) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !tryParseYesNo(values, "mission_complete", missionComplete))
            {
                outError = "Authoritative snapshot missing mission runtime fields.";
                snapshot.valid = false;
                return snapshot;
            }

            snapshot.missionRuntimeState.activeMission = static_cast<MissionId>(missionId);
            snapshot.missionRuntimeState.phase = static_cast<MissionPhase>(missionPhase);
            snapshot.missionRuntimeState.transitDataRecovered = transitDataRecovered;
            snapshot.missionRuntimeState.medlabDiagnosisComplete = medlabDiagnosisComplete;
            snapshot.missionRuntimeState.quarantineGateUnlocked = quarantineGateUnlocked;
            snapshot.missionRuntimeState.quarantineEncounterResolved = quarantineEncounterResolved;
            snapshot.missionRuntimeState.quarantineControlRestored = quarantineControlRestored;
            snapshot.missionRuntimeState.shipRuntimePrepReady = shipRuntimePrepReady;
            snapshot.missionRuntimeState.orbitalDepartureComplete = orbitalDepartureComplete;
            snapshot.missionRuntimeState.surveyOrbitReached = surveyOrbitReached;
            snapshot.missionRuntimeState.relayTrackStabilized = relayTrackStabilized;
            snapshot.missionRuntimeState.relayPlatformDocked = relayPlatformDocked;
            snapshot.missionRuntimeState.dustFrontierLanded = dustFrontierLanded;
            snapshot.missionRuntimeState.frontierRelaySecured = frontierRelaySecured;
            snapshot.missionRuntimeState.returnLaunchComplete = returnLaunchComplete;
            snapshot.missionRuntimeState.homeDockRestored = homeDockRestored;
            snapshot.missionRuntimeState.returnLoopComplete = returnLoopComplete;
            snapshot.missionRuntimeState.missionComplete = missionComplete;
            if (!tryParseString(values, "mission_last_beat", snapshot.missionRuntimeState.lastBeat))
            {
                snapshot.missionRuntimeState.lastBeat = "Mission state restored from authoritative snapshot.";
            }
        }
        else
        {
            snapshot.missionRuntimeState = MissionRuntimeState{};
            snapshot.missionRuntimeState.lastBeat = "Mission state synthesized during schema migration.";
        }

        const bool shipSurfaceExpected =
            loadedSchemaVersion >= kCurrentPersistenceSchemaVersion
            || hasKey("ship_id")
            || hasKey("ship_name");

        if (shipSurfaceExpected)
        {
            int shipId = 0;
            int ownershipState = 0;
            int occupancyState = 0;
            bool boardingUnlocked = false;
            bool docked = false;
            bool playerBoarded = false;
            bool powerOnline = false;
            bool airlockPressurized = false;
            bool commandClaimed = false;
            bool launchPrepReady = false;
            bool frontierSurfaceAccessUnlocked = false;
            bool frontierSurfaceActive = false;

            if (!parseOrFailInt("ship_id", shipId)
                || !parseOrFailInt("ship_ownership_state", ownershipState)
                || !parseOrFailInt("ship_occupancy_state", occupancyState)
                || !parseOrFailUnsigned("ship_boarding_count", snapshot.shipRuntimeState.boardingCount)
                || !tryParseYesNo(values, "ship_boarding_unlocked", boardingUnlocked)
                || !tryParseYesNo(values, "ship_docked", docked)
                || !tryParseYesNo(values, "ship_player_boarded", playerBoarded)
                || !tryParseYesNo(values, "ship_power_online", powerOnline)
                || !tryParseYesNo(values, "ship_airlock_pressurized", airlockPressurized)
                || !tryParseYesNo(values, "ship_command_claimed", commandClaimed)
                || !tryParseYesNo(values, "ship_launch_prep_ready", launchPrepReady)
                || !(tryParseYesNo(values, "ship_frontier_surface_access_unlocked", frontierSurfaceAccessUnlocked) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "ship_frontier_surface_active", frontierSurfaceActive) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion))
            {
                outError = "Authoritative snapshot missing ship runtime fields.";
                snapshot.valid = false;
                return snapshot;
            }

            snapshot.shipRuntimeState.activeShip = static_cast<ShipId>(shipId);
            snapshot.shipRuntimeState.ownershipState = static_cast<ShipOwnershipState>(ownershipState);
            snapshot.shipRuntimeState.occupancyState = static_cast<ShipOccupancyState>(occupancyState);
            snapshot.shipRuntimeState.boardingUnlocked = boardingUnlocked;
            snapshot.shipRuntimeState.docked = docked;
            snapshot.shipRuntimeState.playerBoarded = playerBoarded;
            snapshot.shipRuntimeState.powerOnline = powerOnline;
            snapshot.shipRuntimeState.airlockPressurized = airlockPressurized;
            snapshot.shipRuntimeState.commandClaimed = commandClaimed;
            snapshot.shipRuntimeState.launchPrepReady = launchPrepReady;
            snapshot.shipRuntimeState.frontierSurfaceAccessUnlocked = frontierSurfaceAccessUnlocked;
            snapshot.shipRuntimeState.frontierSurfaceActive = frontierSurfaceActive;
            if (!tryParseString(values, "ship_name", snapshot.shipRuntimeState.shipName))
            {
                snapshot.shipRuntimeState.shipName = "Responder Shuttle Khepri";
            }
            if (!tryParseString(values, "ship_location", snapshot.shipRuntimeState.locationLabel))
            {
                snapshot.shipRuntimeState.locationLabel = "cargo-bay-dock";
            }
            if (!tryParseString(values, "ship_last_beat", snapshot.shipRuntimeState.lastBeat))
            {
                snapshot.shipRuntimeState.lastBeat = "Ship state restored from authoritative snapshot.";
            }
        }
        else
        {
            snapshot.shipRuntimeState = ShipRuntimeState{};
            snapshot.shipRuntimeState.boardingUnlocked = snapshot.missionRuntimeState.shipRuntimePrepReady;
            snapshot.shipRuntimeState.lastBeat = "Ship state synthesized during schema migration.";
        }

        const bool orbitalSurfaceExpected =
            loadedSchemaVersion >= kCurrentPersistenceSchemaVersion
            || hasKey("orbital_layer_unlocked")
            || hasKey("orbital_phase");

        if (orbitalSurfaceExpected)
        {
            bool orbitalLayerUnlocked = false;
            bool orbitalLayerActive = false;
            bool departureAuthorized = false;
            bool surveyOrbitReached = false;
            bool relayTrackReached = false;
            bool relayPlatformDocked = false;
            bool returnRouteAuthorized = false;
            bool homeDockReached = false;
            bool travelInProgress = false;
            uint64_t orbitalTravelTicksRemaining = 0;
            int currentNode = 0;
            int targetNode = 0;
            int orbitalPhase = 0;

            if (!tryParseYesNo(values, "orbital_layer_unlocked", orbitalLayerUnlocked)
                || !tryParseYesNo(values, "orbital_layer_active", orbitalLayerActive)
                || !tryParseYesNo(values, "orbital_departure_authorized", departureAuthorized)
                || !tryParseYesNo(values, "orbital_survey_orbit_reached", surveyOrbitReached)
                || !tryParseYesNo(values, "orbital_relay_track_reached", relayTrackReached)
                || !(tryParseYesNo(values, "orbital_relay_platform_docked", relayPlatformDocked) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "orbital_return_route_authorized", returnRouteAuthorized) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !(tryParseYesNo(values, "orbital_home_dock_reached", homeDockReached) || loadedSchemaVersion < kCurrentPersistenceSchemaVersion)
                || !tryParseYesNo(values, "orbital_travel_in_progress", travelInProgress)
                || !parseOrFailUnsigned("orbital_transfer_count", snapshot.orbitalRuntimeState.transferCount)
                || !parseOrFailUnsigned("orbital_travel_ticks_remaining", orbitalTravelTicksRemaining)
                || !parseOrFailInt("orbital_current_node", currentNode)
                || !parseOrFailInt("orbital_target_node", targetNode)
                || !parseOrFailInt("orbital_phase", orbitalPhase))
            {
                outError = "Authoritative snapshot missing orbital runtime fields.";
                snapshot.valid = false;
                return snapshot;
            }

            snapshot.orbitalRuntimeState.orbitalLayerUnlocked = orbitalLayerUnlocked;
            snapshot.orbitalRuntimeState.orbitalLayerActive = orbitalLayerActive;
            snapshot.orbitalRuntimeState.departureAuthorized = departureAuthorized;
            snapshot.orbitalRuntimeState.surveyOrbitReached = surveyOrbitReached;
            snapshot.orbitalRuntimeState.relayTrackReached = relayTrackReached;
            snapshot.orbitalRuntimeState.relayPlatformDocked = relayPlatformDocked;
            snapshot.orbitalRuntimeState.returnRouteAuthorized = returnRouteAuthorized;
            snapshot.orbitalRuntimeState.homeDockReached = homeDockReached;
            snapshot.orbitalRuntimeState.travelInProgress = travelInProgress;
            snapshot.orbitalRuntimeState.travelTicksRemaining = static_cast<uint32_t>(orbitalTravelTicksRemaining);
            snapshot.orbitalRuntimeState.currentNode = static_cast<OrbitalNodeId>(currentNode);
            snapshot.orbitalRuntimeState.targetNode = static_cast<OrbitalNodeId>(targetNode);
            snapshot.orbitalRuntimeState.phase = static_cast<OrbitalTravelPhase>(orbitalPhase);
            if (!tryParseString(values, "orbital_rule_text", snapshot.orbitalRuntimeState.ruleText))
            {
                snapshot.orbitalRuntimeState.ruleText = "Orbital runtime restored from authoritative snapshot.";
            }
            if (!tryParseString(values, "orbital_last_beat", snapshot.orbitalRuntimeState.lastBeat))
            {
                snapshot.orbitalRuntimeState.lastBeat = "Orbital runtime restored from authoritative snapshot.";
            }
        }
        else
        {
            snapshot.orbitalRuntimeState = OrbitalRuntimeState{};
            if (loadedSchemaVersion < kCurrentPersistenceSchemaVersion && snapshot.shipRuntimeState.commandClaimed)
            {
                snapshot.orbitalRuntimeState.orbitalLayerUnlocked = true;
                snapshot.orbitalRuntimeState.departureAuthorized = true;
                snapshot.orbitalRuntimeState.ruleText = "Migrated shuttle command state into orbital departure readiness.";
                snapshot.orbitalRuntimeState.lastBeat = "Orbital runtime synthesized during schema migration.";
                snapshot.missionRuntimeState.phase = MissionPhase::EnterOrbitalLane;
                snapshot.missionRuntimeState.missionComplete = false;
                snapshot.missionRuntimeState.lastBeat = "Shuttle command carried forward into orbital departure readiness.";
            }
        }

        if (loadedSchemaVersion < kCurrentPersistenceSchemaVersion && snapshot.shipRuntimeState.commandClaimed)
        {
            snapshot.missionRuntimeState.shipRuntimePrepReady = true;

            if (snapshot.missionRuntimeState.relayTrackStabilized
                || snapshot.missionRuntimeState.phase == MissionPhase::DockRelayPlatform)
            {
                snapshot.missionRuntimeState.phase = MissionPhase::DockRelayPlatform;
                snapshot.missionRuntimeState.missionComplete = false;
                snapshot.missionRuntimeState.lastBeat = "M43 relay holding track migrated into M44 docking readiness.";
                snapshot.orbitalRuntimeState.orbitalLayerUnlocked = true;
                snapshot.orbitalRuntimeState.orbitalLayerActive = true;
                snapshot.orbitalRuntimeState.departureAuthorized = true;
                snapshot.orbitalRuntimeState.relayTrackReached = true;
                snapshot.orbitalRuntimeState.currentNode = OrbitalNodeId::RelayHoldingTrack;
                snapshot.orbitalRuntimeState.targetNode = OrbitalNodeId::RelayHoldingTrack;
                snapshot.orbitalRuntimeState.phase = OrbitalTravelPhase::RelayHolding;
                snapshot.orbitalRuntimeState.ruleText = "Relay holding track stable. Dock with Dust Frontier Relay Platform to begin the landing slice.";
                snapshot.orbitalRuntimeState.lastBeat = "Relay holding track stabilized prior to M44 migration.";
                snapshot.shipRuntimeState.frontierSurfaceAccessUnlocked = false;
                snapshot.shipRuntimeState.frontierSurfaceActive = false;
            }
        }

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
