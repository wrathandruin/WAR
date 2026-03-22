#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <windows.h>

#include "engine/core/RuntimePaths.h"

namespace war
{
    struct SessionContinuitySnapshot
    {
        uint64_t savedEpochMilliseconds = 0;
        std::string grantedSessionId;
        std::string resumeSessionId;
        std::string clientInstanceId;
        std::string locationKey;
        std::string roomTitle;
        std::string roomDescription;
        std::string missionPhaseText;
        std::string missionObjectiveText;
        std::string missionLastBeat;
        std::string shipLocationText;
        std::string shipLastBeat;
        std::string orbitalNodeText;
        std::string orbitalPhaseText;
        std::string socialLocationKey;
        uint32_t sameSpacePresenceCount = 0;
        std::string sameSpacePresenceSummary;
    };

    struct SessionLeaseRecord
    {
        uint64_t heartbeatEpochMilliseconds = 0;
        std::string grantedSessionId;
        std::string clientInstanceId;
        std::string participantId;
        std::string locationKey;
        std::string reconnectState;
    };

    class ReconnectSessionProtocol
    {
    public:
        [[nodiscard]] static std::filesystem::path continuityStatePath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& clientSlot = {})
        {
            return scopedConfigPath(runtimeBoundaryReport, "client_session_continuity.txt", clientSlot);
        }

        [[nodiscard]] static std::filesystem::path leaseLanePath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "reconnect_session_lane.txt";
        }

        [[nodiscard]] static const char* recoveryRuleText()
        {
            return "resume-session-id+stale-lease-eviction";
        }

        [[nodiscard]] static uint64_t staleLeaseThresholdMilliseconds()
        {
            return 15000ull;
        }

        static bool writeContinuitySnapshot(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SessionContinuitySnapshot& snapshot,
            std::string& outError,
            const std::string& clientSlot = {})
        {
            outError.clear();

            std::error_code error;
            std::filesystem::create_directories(runtimeBoundaryReport.configDirectory, error);
            if (error)
            {
                outError = error.message();
                return false;
            }

            const std::filesystem::path path = continuityStatePath(runtimeBoundaryReport, clientSlot);
            const std::filesystem::path tempPath = path.parent_path() / (path.filename().string() + ".tmp");
            std::ofstream output(tempPath, std::ios::out | std::ios::trunc);
            if (!output.is_open())
            {
                outError = "could-not-open-continuity-state";
                return false;
            }

            output
                << "saved_epoch_milliseconds=" << snapshot.savedEpochMilliseconds << "\n"
                << "granted_session_id=" << sanitizeField(snapshot.grantedSessionId) << "\n"
                << "resume_session_id=" << sanitizeField(snapshot.resumeSessionId) << "\n"
                << "client_instance_id=" << sanitizeField(snapshot.clientInstanceId) << "\n"
                << "location_key=" << sanitizeField(snapshot.locationKey) << "\n"
                << "room_title=" << sanitizeField(snapshot.roomTitle) << "\n"
                << "room_description=" << sanitizeField(snapshot.roomDescription) << "\n"
                << "mission_phase_text=" << sanitizeField(snapshot.missionPhaseText) << "\n"
                << "mission_objective_text=" << sanitizeField(snapshot.missionObjectiveText) << "\n"
                << "mission_last_beat=" << sanitizeField(snapshot.missionLastBeat) << "\n"
                << "ship_location_text=" << sanitizeField(snapshot.shipLocationText) << "\n"
                << "ship_last_beat=" << sanitizeField(snapshot.shipLastBeat) << "\n"
                << "orbital_node_text=" << sanitizeField(snapshot.orbitalNodeText) << "\n"
                << "orbital_phase_text=" << sanitizeField(snapshot.orbitalPhaseText) << "\n"
                << "social_location_key=" << sanitizeField(snapshot.socialLocationKey) << "\n"
                << "same_space_presence_count=" << snapshot.sameSpacePresenceCount << "\n"
                << "same_space_presence_summary=" << sanitizeField(snapshot.sameSpacePresenceSummary) << "\n";

            output.close();
            if (!output)
            {
                std::filesystem::remove(tempPath, error);
                outError = "could-not-write-continuity-state";
                return false;
            }

        #if defined(_WIN32)
            if (!MoveFileExW(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
                std::filesystem::remove(tempPath, error);
                outError = "could-not-promote-continuity-state";
                return false;
            }
        #else
            std::filesystem::rename(tempPath, path, error);
            if (error)
            {
                std::filesystem::remove(tempPath, error);
                outError = error.message();
                return false;
            }
        #endif

            return true;
        }

        [[nodiscard]] static bool readContinuitySnapshot(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            SessionContinuitySnapshot& outSnapshot,
            std::string& outError,
            const std::string& clientSlot = {})
        {
            outError.clear();
            outSnapshot = {};

            std::ifstream input(continuityStatePath(runtimeBoundaryReport, clientSlot), std::ios::in);
            if (!input.is_open())
            {
                return false;
            }

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

                const std::string key = line.substr(0, split);
                const std::string value = line.substr(split + 1);

                if (key == "saved_epoch_milliseconds")
                {
                    try { outSnapshot.savedEpochMilliseconds = static_cast<uint64_t>(std::stoull(value)); } catch (...) { outSnapshot.savedEpochMilliseconds = 0; }
                }
                else if (key == "granted_session_id") { outSnapshot.grantedSessionId = value; }
                else if (key == "resume_session_id") { outSnapshot.resumeSessionId = value; }
                else if (key == "client_instance_id") { outSnapshot.clientInstanceId = value; }
                else if (key == "location_key") { outSnapshot.locationKey = value; }
                else if (key == "room_title") { outSnapshot.roomTitle = value; }
                else if (key == "room_description") { outSnapshot.roomDescription = value; }
                else if (key == "mission_phase_text") { outSnapshot.missionPhaseText = value; }
                else if (key == "mission_objective_text") { outSnapshot.missionObjectiveText = value; }
                else if (key == "mission_last_beat") { outSnapshot.missionLastBeat = value; }
                else if (key == "ship_location_text") { outSnapshot.shipLocationText = value; }
                else if (key == "ship_last_beat") { outSnapshot.shipLastBeat = value; }
                else if (key == "orbital_node_text") { outSnapshot.orbitalNodeText = value; }
                else if (key == "orbital_phase_text") { outSnapshot.orbitalPhaseText = value; }
                else if (key == "social_location_key") { outSnapshot.socialLocationKey = value; }
                else if (key == "same_space_presence_count")
                {
                    try { outSnapshot.sameSpacePresenceCount = static_cast<uint32_t>(std::stoul(value)); } catch (...) { outSnapshot.sameSpacePresenceCount = 0; }
                }
                else if (key == "same_space_presence_summary") { outSnapshot.sameSpacePresenceSummary = value; }
            }

            if (!input.good() && !input.eof())
            {
                outError = "could-not-read-continuity-state";
                return false;
            }

            return !outSnapshot.locationKey.empty() || !outSnapshot.grantedSessionId.empty() || !outSnapshot.resumeSessionId.empty();
        }

        static bool appendLeaseRecord(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SessionLeaseRecord& record,
            std::string& outError)
        {
            outError.clear();

            std::error_code error;
            std::filesystem::create_directories(runtimeBoundaryReport.logsDirectory, error);
            if (error)
            {
                outError = error.message();
                return false;
            }

            std::ofstream output(leaseLanePath(runtimeBoundaryReport), std::ios::out | std::ios::app);
            if (!output.is_open())
            {
                outError = "could-not-open-reconnect-lease-lane";
                return false;
            }

            output
                << record.heartbeatEpochMilliseconds << '|'
                << sanitizeField(record.grantedSessionId) << '|'
                << sanitizeField(record.clientInstanceId) << '|'
                << sanitizeField(record.participantId) << '|'
                << sanitizeField(record.locationKey) << '|'
                << sanitizeField(record.reconnectState)
                << '\n';

            output.close();
            if (!output)
            {
                outError = "could-not-write-reconnect-lease-lane";
                return false;
            }

            return true;
        }

        [[nodiscard]] static std::vector<SessionLeaseRecord> readLeaseRecords(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            std::string& outError)
        {
            outError.clear();
            std::vector<SessionLeaseRecord> records{};

            std::ifstream input(leaseLanePath(runtimeBoundaryReport), std::ios::in);
            if (!input.is_open())
            {
                return records;
            }

            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                SessionLeaseRecord record{};
                if (tryParseLeaseLine(line, record))
                {
                    records.push_back(std::move(record));
                }
            }

            if (!input.good() && !input.eof())
            {
                outError = "could-not-read-reconnect-lease-lane";
            }

            return records;
        }

    private:
        [[nodiscard]] static std::filesystem::path scopedConfigPath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const char* filename,
            const std::string& clientSlot)
        {
            if (clientSlot.empty())
            {
                return runtimeBoundaryReport.configDirectory / filename;
            }

            const std::filesystem::path basePath = runtimeBoundaryReport.configDirectory / filename;
            return basePath.parent_path()
                / (basePath.stem().string() + "_" + clientSlot + basePath.extension().string());
        }

        [[nodiscard]] static std::string sanitizeField(std::string value)
        {
            std::replace_if(value.begin(), value.end(), [](char ch)
            {
                return ch == '\n' || ch == '\r' || ch == '\t' || ch == '|';
            }, ' ');
            return value;
        }

        [[nodiscard]] static bool tryParseLeaseLine(const std::string& line, SessionLeaseRecord& outRecord)
        {
            std::vector<std::string> fields{};
            std::string field;
            std::istringstream input(line);
            while (std::getline(input, field, '|'))
            {
                fields.push_back(field);
            }

            if (fields.size() != 6u)
            {
                return false;
            }

            try
            {
                outRecord.heartbeatEpochMilliseconds = static_cast<uint64_t>(std::stoull(fields[0]));
            }
            catch (...)
            {
                return false;
            }

            outRecord.grantedSessionId = fields[1];
            outRecord.clientInstanceId = fields[2];
            outRecord.participantId = fields[3];
            outRecord.locationKey = fields[4];
            outRecord.reconnectState = fields[5];
            return true;
        }
    };
}
