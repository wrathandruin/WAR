#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "engine/core/RuntimePaths.h"

namespace war
{
    struct LocalPresenceHeartbeat
    {
        uint64_t heartbeatEpochMilliseconds = 0;
        std::string participantId;
        std::string sessionId;
        std::string playerIdentity;
        std::string locationKey;
        std::string roomTitle;
        std::string missionPhaseText;
        std::string missionObjectiveText;
        int tileX = 0;
        int tileY = 0;
        float worldX = 0.0f;
        float worldY = 0.0f;
    };

    class LocalPresenceProtocol
    {
    public:
        [[nodiscard]] static std::filesystem::path lanePath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "local_presence_lane.txt";
        }

        [[nodiscard]] static const char* interestRuleText()
        {
            return "same-location-key+fresh-heartbeat";
        }

        static bool appendHeartbeat(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const LocalPresenceHeartbeat& heartbeat,
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

            std::ofstream output(lanePath(runtimeBoundaryReport), std::ios::out | std::ios::app);
            if (!output.is_open())
            {
                outError = "could-not-open-presence-lane";
                return false;
            }

            output
                << heartbeat.heartbeatEpochMilliseconds << '|'
                << sanitizeField(heartbeat.participantId) << '|'
                << sanitizeField(heartbeat.sessionId) << '|'
                << sanitizeField(heartbeat.playerIdentity) << '|'
                << sanitizeField(heartbeat.locationKey) << '|'
                << sanitizeField(heartbeat.roomTitle) << '|'
                << sanitizeField(heartbeat.missionPhaseText) << '|'
                << sanitizeField(heartbeat.missionObjectiveText) << '|'
                << heartbeat.tileX << '|'
                << heartbeat.tileY << '|'
                << heartbeat.worldX << '|'
                << heartbeat.worldY
                << '\n';

            output.close();
            if (!output)
            {
                outError = "could-not-write-presence-lane";
                return false;
            }

            return true;
        }

        [[nodiscard]] static std::vector<LocalPresenceHeartbeat> readHeartbeats(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            std::string& outError)
        {
            outError.clear();
            std::vector<LocalPresenceHeartbeat> heartbeats{};

            std::ifstream input(lanePath(runtimeBoundaryReport), std::ios::in);
            if (!input.is_open())
            {
                return heartbeats;
            }

            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                LocalPresenceHeartbeat heartbeat{};
                if (tryParseLine(line, heartbeat))
                {
                    heartbeats.push_back(std::move(heartbeat));
                }
            }

            if (!input.good() && !input.eof())
            {
                outError = "could-not-read-presence-lane";
            }

            return heartbeats;
        }

    private:
        [[nodiscard]] static std::string sanitizeField(std::string value)
        {
            std::replace_if(value.begin(), value.end(), [](char ch)
            {
                return ch == '\n' || ch == '\r' || ch == '\t' || ch == '|';
            }, ' ');

            return value;
        }

        [[nodiscard]] static bool tryParseLine(const std::string& line, LocalPresenceHeartbeat& outHeartbeat)
        {
            std::vector<std::string> fields{};
            std::string field;
            std::istringstream input(line);
            while (std::getline(input, field, '|'))
            {
                fields.push_back(field);
            }

            if (fields.size() != 8u && fields.size() != 10u && fields.size() != 12u)
            {
                return false;
            }

            try
            {
                outHeartbeat.heartbeatEpochMilliseconds = static_cast<uint64_t>(std::stoull(fields[0]));
            }
            catch (...)
            {
                return false;
            }

            outHeartbeat.participantId = fields[1];
            outHeartbeat.sessionId = fields[2];
            outHeartbeat.playerIdentity = fields[3];
            outHeartbeat.locationKey = fields[4];
            outHeartbeat.roomTitle = fields[5];
            outHeartbeat.missionPhaseText = fields[6];
            outHeartbeat.missionObjectiveText = fields[7];

            if (fields.size() >= 10u)
            {
                try
                {
                    outHeartbeat.tileX = std::stoi(fields[8]);
                    outHeartbeat.tileY = std::stoi(fields[9]);
                }
                catch (...)
                {
                    outHeartbeat.tileX = 0;
                    outHeartbeat.tileY = 0;
                }
            }
            if (fields.size() == 12u)
            {
                try
                {
                    outHeartbeat.worldX = std::stof(fields[10]);
                    outHeartbeat.worldY = std::stof(fields[11]);
                }
                catch (...)
                {
                    outHeartbeat.worldX = 0.0f;
                    outHeartbeat.worldY = 0.0f;
                }
            }
            return true;
        }
    };
}
