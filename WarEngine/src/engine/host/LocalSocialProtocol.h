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
    enum class LocalSocialMessageKind
    {
        Say,
        Emote
    };

    struct LocalSocialMessage
    {
        uint64_t sentEpochMilliseconds = 0;
        std::string senderSessionId;
        std::string senderIdentity;
        std::string locationKey;
        LocalSocialMessageKind kind = LocalSocialMessageKind::Say;
        std::string text;
    };

    class LocalSocialProtocol
    {
    public:
        [[nodiscard]] static std::filesystem::path lanePath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.logsDirectory / "local_social_lane.txt";
        }

        [[nodiscard]] static std::filesystem::path statePath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& clientSlot = {})
        {
            return scopedConfigPath(runtimeBoundaryReport, "client_social_state.txt", clientSlot);
        }

        [[nodiscard]] static const char* kindText(LocalSocialMessageKind kind)
        {
            switch (kind)
            {
            case LocalSocialMessageKind::Say:
                return "say";
            case LocalSocialMessageKind::Emote:
                return "emote";
            default:
                return "say";
            }
        }

        static bool appendMessage(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const LocalSocialMessage& message,
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
                outError = "could-not-open-social-lane";
                return false;
            }

            output
                << message.sentEpochMilliseconds << '|'
                << sanitizeField(message.senderSessionId) << '|'
                << sanitizeField(message.senderIdentity) << '|'
                << sanitizeField(message.locationKey) << '|'
                << kindText(message.kind) << '|'
                << sanitizeField(message.text)
                << '\n';

            output.close();
            if (!output)
            {
                outError = "could-not-write-social-lane";
                return false;
            }

            return true;
        }

        [[nodiscard]] static std::vector<LocalSocialMessage> readMessages(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            std::string& outError)
        {
            outError.clear();
            std::vector<LocalSocialMessage> messages{};

            std::ifstream input(lanePath(runtimeBoundaryReport), std::ios::in);
            if (!input.is_open())
            {
                return messages;
            }

            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                LocalSocialMessage message{};
                if (tryParseLine(line, message))
                {
                    messages.push_back(std::move(message));
                }
            }

            if (!input.good() && !input.eof())
            {
                outError = "could-not-read-social-lane";
            }

            return messages;
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

        [[nodiscard]] static bool tryParseLine(const std::string& line, LocalSocialMessage& outMessage)
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
                outMessage.sentEpochMilliseconds = static_cast<uint64_t>(std::stoull(fields[0]));
            }
            catch (...)
            {
                return false;
            }

            outMessage.senderSessionId = fields[1];
            outMessage.senderIdentity = fields[2];
            outMessage.locationKey = fields[3];
            outMessage.kind = fields[4] == "emote"
                ? LocalSocialMessageKind::Emote
                : LocalSocialMessageKind::Say;
            outMessage.text = fields[5];
            return true;
        }
    };
}
