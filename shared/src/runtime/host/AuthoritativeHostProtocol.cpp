#include "engine/host/AuthoritativeHostProtocol.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>

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

        StringMap parseKeyValueFile(const std::filesystem::path& path)
        {
            StringMap values{};
            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                return values;
            }

            std::string line;
            while (std::getline(input, line))
            {
                const size_t split = line.find('=');
                if (split == std::string::npos)
                {
                    continue;
                }

                values.emplace(line.substr(0, split), line.substr(split + 1));
            }

            return values;
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
            if (value == "move")
            {
                return SimulationIntentType::MoveToTile;
            }
            if (value == "inspect")
            {
                return SimulationIntentType::InspectTile;
            }
            if (value == "interact")
            {
                return SimulationIntentType::InteractTile;
            }

            return SimulationIntentType::InspectTile;
        }

        SimulationIntentAckResult parseAckResult(const std::string& value)
        {
            if (value == "accepted")
            {
                return SimulationIntentAckResult::Accepted;
            }
            if (value == "rejected")
            {
                return SimulationIntentAckResult::Rejected;
            }

            return SimulationIntentAckResult::None;
        }

        EntityType parseEntityType(const std::string& value)
        {
            if (value == "terminal")
            {
                return EntityType::Terminal;
            }
            if (value == "locker")
            {
                return EntityType::Locker;
            }

            return EntityType::Crate;
        }

        const char* entityTypeText(EntityType type)
        {
            switch (type)
            {
            case EntityType::Crate:
                return "crate";
            case EntityType::Terminal:
                return "terminal";
            case EntityType::Locker:
                return "locker";
            default:
                return "crate";
            }
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
            if (!MoveFileExW(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
                outError = std::string("Failed to publish file atomically: ") + path.generic_string();
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

        std::string serializeWorldSnapshotBody(const AuthoritativeWorldSnapshot& snapshot)
        {
            std::ostringstream output;
            output
                << "valid=" << (snapshot.valid ? "yes" : "no") << "\n"
                << "simulation_ticks=" << snapshot.simulationTicks << "\n"
                << "last_processed_intent_sequence=" << snapshot.lastProcessedIntentSequence << "\n"
                << "player_x=" << snapshot.authoritativePlayerPosition.x << "\n"
                << "player_y=" << snapshot.authoritativePlayerPosition.y << "\n"
                << "movement_target_active=" << (snapshot.movementTargetActive ? "yes" : "no") << "\n"
                << "movement_target_x=" << snapshot.movementTargetTile.x << "\n"
                << "movement_target_y=" << snapshot.movementTargetTile.y << "\n"
                << "path_index=" << snapshot.pathIndex << "\n"
                << "path_count=" << snapshot.currentPath.size() << "\n";

            for (size_t i = 0; i < snapshot.currentPath.size(); ++i)
            {
                output << "path_" << i << "=" << snapshot.currentPath[i].x << "," << snapshot.currentPath[i].y << "\n";
            }

            output << "entity_count=" << snapshot.entities.size() << "\n";
            for (size_t i = 0; i < snapshot.entities.size(); ++i)
            {
                const ReplicatedEntityState& entity = snapshot.entities[i];
                output
                    << "entity_" << i << "="
                    << entity.id << "|"
                    << entityTypeText(entity.type) << "|"
                    << entity.tile.x << "|"
                    << entity.tile.y << "|"
                    << (entity.isOpen ? 1 : 0) << "|"
                    << (entity.isLocked ? 1 : 0) << "|"
                    << (entity.isPowered ? 1 : 0) << "|"
                    << sanitizeSingleLine(entity.name)
                    << "\n";
            }

            output << "event_count=" << snapshot.eventLog.size() << "\n";
            for (size_t i = 0; i < snapshot.eventLog.size(); ++i)
            {
                output << "event_" << i << "=" << sanitizeSingleLine(snapshot.eventLog[i]) << "\n";
            }

            return output.str();
        }

        bool parseWorldSnapshotBody(
            const StringMap& values,
            bool requirePublishedEpoch,
            AuthoritativeWorldSnapshot& snapshot,
            std::string& outError)
        {
            outError.clear();

            uint64_t simulationTicks = 0;
            uint64_t lastProcessedIntentSequence = 0;
            uint64_t publishedEpochMilliseconds = 0;
            uint64_t movementTargetX = 0;
            uint64_t movementTargetY = 0;
            uint64_t pathIndex = 0;
            uint64_t pathCount = 0;
            uint64_t entityCount = 0;
            uint64_t eventCount = 0;
            float playerX = 0.0f;
            float playerY = 0.0f;
            bool valid = false;
            bool movementTargetActive = false;

            if (!tryParseYesNo(values, "valid", valid)
                || !tryParseUnsigned(values, "simulation_ticks", simulationTicks)
                || !tryParseUnsigned(values, "last_processed_intent_sequence", lastProcessedIntentSequence)
                || !tryParseFloat(values, "player_x", playerX)
                || !tryParseFloat(values, "player_y", playerY)
                || !tryParseUnsigned(values, "path_count", pathCount)
                || !tryParseUnsigned(values, "entity_count", entityCount))
            {
                outError = "Snapshot missing required fields.";
                return false;
            }

            if (requirePublishedEpoch)
            {
                if (!tryParseUnsigned(values, "published_epoch_ms", publishedEpochMilliseconds))
                {
                    outError = "Snapshot missing published epoch.";
                    return false;
                }
            }

            if (!tryParseUnsigned(values, "path_index", pathIndex))
            {
                pathIndex = 0;
            }

            if (!tryParseUnsigned(values, "event_count", eventCount))
            {
                eventCount = 0;
            }

            if (!tryParseYesNo(values, "movement_target_active", movementTargetActive))
            {
                movementTargetActive = false;
            }

            if (!tryParseUnsigned(values, "movement_target_x", movementTargetX))
            {
                movementTargetX = 0;
            }

            if (!tryParseUnsigned(values, "movement_target_y", movementTargetY))
            {
                movementTargetY = 0;
            }

            snapshot.valid = valid;
            snapshot.simulationTicks = simulationTicks;
            snapshot.lastProcessedIntentSequence = lastProcessedIntentSequence;
            snapshot.publishedEpochMilliseconds = publishedEpochMilliseconds;
            snapshot.authoritativePlayerPosition.x = playerX;
            snapshot.authoritativePlayerPosition.y = playerY;
            snapshot.movementTargetActive = movementTargetActive;
            snapshot.movementTargetTile.x = static_cast<int>(movementTargetX);
            snapshot.movementTargetTile.y = static_cast<int>(movementTargetY);
            snapshot.pathIndex = static_cast<size_t>(pathIndex);

            snapshot.currentPath.clear();
            snapshot.currentPath.reserve(static_cast<size_t>(pathCount));
            for (size_t i = 0; i < static_cast<size_t>(pathCount); ++i)
            {
                const std::string key = "path_" + std::to_string(i);
                const auto it = values.find(key);
                if (it == values.end())
                {
                    outError = std::string("Snapshot missing path entry: ") + key;
                    snapshot.valid = false;
                    return false;
                }

                const size_t split = it->second.find(',');
                if (split == std::string::npos)
                {
                    outError = std::string("Snapshot path entry malformed: ") + key;
                    snapshot.valid = false;
                    return false;
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
                    outError = std::string("Snapshot path entry invalid: ") + key;
                    snapshot.valid = false;
                    return false;
                }
            }

            snapshot.entities.clear();
            snapshot.entities.reserve(static_cast<size_t>(entityCount));
            for (size_t i = 0; i < static_cast<size_t>(entityCount); ++i)
            {
                const std::string key = "entity_" + std::to_string(i);
                const auto it = values.find(key);
                if (it == values.end())
                {
                    outError = std::string("Snapshot missing entity entry: ") + key;
                    snapshot.valid = false;
                    return false;
                }

                std::stringstream stream(it->second);
                std::string idText;
                std::string typeText;
                std::string xText;
                std::string yText;
                std::string openText;
                std::string lockedText;
                std::string poweredText;
                std::string nameText;

                if (!std::getline(stream, idText, '|')
                    || !std::getline(stream, typeText, '|')
                    || !std::getline(stream, xText, '|')
                    || !std::getline(stream, yText, '|')
                    || !std::getline(stream, openText, '|')
                    || !std::getline(stream, lockedText, '|')
                    || !std::getline(stream, poweredText, '|')
                    || !std::getline(stream, nameText))
                {
                    outError = std::string("Snapshot malformed entity entry: ") + key;
                    snapshot.valid = false;
                    return false;
                }

                try
                {
                    ReplicatedEntityState entity{};
                    entity.id = std::stoi(idText);
                    entity.type = parseEntityType(typeText);
                    entity.tile.x = std::stoi(xText);
                    entity.tile.y = std::stoi(yText);
                    entity.isOpen = openText == "1";
                    entity.isLocked = lockedText == "1";
                    entity.isPowered = poweredText == "1";
                    entity.name = nameText;
                    snapshot.entities.push_back(entity);
                }
                catch (...)
                {
                    outError = std::string("Snapshot invalid entity entry: ") + key;
                    snapshot.valid = false;
                    return false;
                }
            }

            snapshot.eventLog.clear();
            snapshot.eventLog.reserve(static_cast<size_t>(eventCount));
            for (size_t i = 0; i < static_cast<size_t>(eventCount); ++i)
            {
                const std::string key = "event_" + std::to_string(i);
                const auto it = values.find(key);
                if (it == values.end())
                {
                    outError = std::string("Snapshot missing event entry: ") + key;
                    snapshot.valid = false;
                    return false;
                }

                snapshot.eventLog.push_back(it->second);
            }

            return true;
        }
    }

    AuthoritativeHostProtocolReport AuthoritativeHostProtocol::buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        AuthoritativeHostProtocolReport report{};
        report.intentQueueDirectory = intentQueueDirectory(runtimeBoundaryReport);
        report.acknowledgementQueueDirectory = acknowledgementQueueDirectory(runtimeBoundaryReport);
        report.snapshotPath = snapshotPath(runtimeBoundaryReport);
        report.persistentSavePath = persistentSavePath(runtimeBoundaryReport);

        report.intentQueueReady = directoryExists(report.intentQueueDirectory);
        report.acknowledgementQueueReady = directoryExists(report.acknowledgementQueueDirectory);
        report.snapshotPresent = fileExists(report.snapshotPath);
        report.persistentSavePresent = fileExists(report.persistentSavePath);
        report.authorityLaneReady = report.intentQueueReady && report.acknowledgementQueueReady;
        return report;
    }

    void AuthoritativeHostProtocol::ensureDirectories(AuthoritativeHostProtocolReport& report)
    {
        ensureDirectory(report.intentQueueDirectory, report.issues);
        ensureDirectory(report.acknowledgementQueueDirectory, report.issues);
        std::error_code error;
        std::filesystem::create_directories(report.persistentSavePath.parent_path(), error);
        if (error)
        {
            report.issues.push_back(std::string("Failed to create persistence directory: ") + report.persistentSavePath.parent_path().generic_string());
        }

        report.intentQueueReady = directoryExists(report.intentQueueDirectory);
        report.acknowledgementQueueReady = directoryExists(report.acknowledgementQueueDirectory);
        report.persistentSavePresent = fileExists(report.persistentSavePath);
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
            / ("intent_" + std::to_string(intent.sequence) + ".txt");

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
        const std::vector<std::filesystem::path> files = enumerateFilesByPrefix(
            intentQueueDirectory(runtimeBoundaryReport),
            "intent_");

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
            uint64_t targetX = 0;
            uint64_t targetY = 0;
            std::string typeText;

            if (!tryParseUnsigned(values, "sequence", sequence)
                || !tryParseString(values, "type", typeText)
                || !tryParseUnsigned(values, "target_x", targetX)
                || !tryParseUnsigned(values, "target_y", targetY))
            {
                removeFileQuietly(path);
                continue;
            }

            intent.sequence = sequence;
            intent.type = parseIntentType(typeText);
            intent.target.x = static_cast<int>(targetX);
            intent.target.y = static_cast<int>(targetY);

            if (intent.sequence > 0)
            {
                intents.push_back(intent);
            }

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

        const std::filesystem::path path = report.acknowledgementQueueDirectory
            / ("ack_" + std::to_string(acknowledgement.sequence) + ".txt");

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
        const std::vector<std::filesystem::path> files = enumerateFilesByPrefix(
            acknowledgementQueueDirectory(runtimeBoundaryReport),
            "ack_");

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
            uint64_t targetX = 0;
            uint64_t targetY = 0;

            if (!tryParseUnsigned(values, "sequence", sequence)
                || !tryParseString(values, "type", typeText)
                || !tryParseString(values, "result", resultText)
                || !tryParseUnsigned(values, "host_ticks", hostTicks)
                || !tryParseUnsigned(values, "published_epoch_ms", publishedEpochMilliseconds)
                || !tryParseUnsigned(values, "target_x", targetX)
                || !tryParseUnsigned(values, "target_y", targetY))
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
            acknowledgement.target.x = static_cast<int>(targetX);
            acknowledgement.target.y = static_cast<int>(targetY);

            if (acknowledgement.sequence > 0)
            {
                acknowledgements.push_back(acknowledgement);
            }

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
        outError.clear();
        const std::filesystem::path path = snapshotPath(runtimeBoundaryReport);

        std::ostringstream output;
        output
            << "version=1\n"
            << "published_epoch_ms=" << snapshot.publishedEpochMilliseconds << "\n"
            << serializeWorldSnapshotBody(snapshot);

        return writeTextFileAtomically(path, output.str(), outError);
    }

    AuthoritativeWorldSnapshot AuthoritativeHostProtocol::readAuthoritativeSnapshot(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        std::string& outError)
    {
        outError.clear();
        AuthoritativeWorldSnapshot snapshot{};
        const std::filesystem::path path = snapshotPath(runtimeBoundaryReport);
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

        if (!parseWorldSnapshotBody(values, true, snapshot, outError))
        {
            return snapshot;
        }

        return snapshot;
    }

    bool AuthoritativeHostProtocol::writePersistentWorldSave(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const AuthoritativeWorldSnapshot& snapshot,
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

        const std::filesystem::path path = persistentSavePath(runtimeBoundaryReport);
        std::ostringstream output;
        output
            << "schema_version=2\n"
            << "save_slot=" << sanitizeSingleLine(snapshot.persistenceSlotName.empty() ? std::string("primary") : snapshot.persistenceSlotName) << "\n"
            << "save_epoch_ms=" << snapshot.persistenceEpochMilliseconds << "\n"
            << serializeWorldSnapshotBody(snapshot);

        return writeTextFileAtomically(path, output.str(), outError);
    }

    AuthoritativeWorldSnapshot AuthoritativeHostProtocol::readPersistentWorldSave(
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        uint32_t& outLoadedSchemaVersion,
        uint32_t& outMigratedFromSchemaVersion,
        std::string& outError)
    {
        outLoadedSchemaVersion = 0;
        outMigratedFromSchemaVersion = 0;
        outError.clear();

        AuthoritativeWorldSnapshot snapshot{};
        const std::filesystem::path path = persistentSavePath(runtimeBoundaryReport);
        if (!fileExists(path))
        {
            outError = "Persistent save not present.";
            return snapshot;
        }

        const StringMap values = parseKeyValueFile(path);
        if (values.empty())
        {
            outError = "Persistent save file could not be parsed.";
            return snapshot;
        }

        uint64_t schemaVersion = 0;
        if (!tryParseUnsigned(values, "schema_version", schemaVersion))
        {
            outError = "Persistent save missing schema version.";
            return snapshot;
        }

        if (schemaVersion != 1ull && schemaVersion != 2ull)
        {
            outError = std::string("Unsupported persistence schema version: ") + std::to_string(schemaVersion);
            return snapshot;
        }

        if (!parseWorldSnapshotBody(values, false, snapshot, outError))
        {
            return snapshot;
        }

        snapshot.persistenceSchemaVersion = static_cast<uint32_t>(schemaVersion);
        outLoadedSchemaVersion = static_cast<uint32_t>(schemaVersion);

        std::string saveSlot;
        if (tryParseString(values, "save_slot", saveSlot) && !saveSlot.empty())
        {
            snapshot.persistenceSlotName = saveSlot;
        }
        else
        {
            snapshot.persistenceSlotName = "primary";
        }

        uint64_t saveEpochMilliseconds = 0;
        if (tryParseUnsigned(values, "save_epoch_ms", saveEpochMilliseconds))
        {
            snapshot.persistenceEpochMilliseconds = saveEpochMilliseconds;
        }
        else
        {
            snapshot.persistenceEpochMilliseconds = 0;
        }

        if (schemaVersion == 1ull)
        {
            snapshot.persistenceSchemaVersion = 2;
            snapshot.persistenceMigratedFromSchemaVersion = 1;
            outMigratedFromSchemaVersion = 1;
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

    std::filesystem::path AuthoritativeHostProtocol::persistentSavePath(const RuntimeBoundaryReport& runtimeBoundaryReport)
    {
        return runtimeBoundaryReport.savesDirectory / "authoritative_world_primary.txt";
    }
}
