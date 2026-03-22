#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace war
{
    struct SessionMvpRoomAuthoring
    {
        std::string key;
        std::string title;
        std::string description;
    };

    struct SessionMvpMissionAuthoring
    {
        std::string phaseKey;
        std::string objective;
        std::string journal;
    };

    struct SessionMvpAuthoringBundle
    {
        uint32_t schemaVersion = 1u;
        bool valid = false;
        std::string sourcePath;
        std::unordered_map<std::string, SessionMvpRoomAuthoring> roomByKey{};
        std::unordered_map<std::string, SessionMvpMissionAuthoring> missionByPhaseKey{};
        std::vector<std::string> validationErrors{};

        [[nodiscard]] const SessionMvpRoomAuthoring* findRoom(const std::string& key) const
        {
            const auto it = roomByKey.find(key);
            return it == roomByKey.end() ? nullptr : &it->second;
        }

        [[nodiscard]] const SessionMvpMissionAuthoring* findMission(const std::string& phaseKey) const
        {
            const auto it = missionByPhaseKey.find(phaseKey);
            return it == missionByPhaseKey.end() ? nullptr : &it->second;
        }
    };

    class SessionMvpAuthoringPipeline
    {
    public:
        [[nodiscard]] static SessionMvpAuthoringBundle loadFromFile(const std::filesystem::path& path)
        {
            SessionMvpAuthoringBundle bundle{};
            bundle.sourcePath = path.generic_string();

            std::ifstream input(path, std::ios::in);
            if (!input.is_open())
            {
                bundle.validationErrors.push_back("authoring file could not be opened");
                return bundle;
            }

            enum class SectionType
            {
                None,
                Room,
                Mission
            };

            SectionType sectionType = SectionType::None;
            std::string sectionKey;
            std::string line;

            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                const std::string trimmed = trimCopy(line);
                if (trimmed.empty() || trimmed[0] == '#')
                {
                    continue;
                }

                if (trimmed.front() == '[' && trimmed.back() == ']')
                {
                    const std::string body = trimCopy(trimmed.substr(1, trimmed.size() - 2));
                    const size_t split = body.find(' ');
                    const std::string sectionName = split == std::string::npos
                        ? toLowerCopy(body)
                        : toLowerCopy(body.substr(0, split));
                    sectionKey = split == std::string::npos
                        ? std::string()
                        : trimCopy(body.substr(split + 1));

                    if (sectionName == "room")
                    {
                        sectionType = SectionType::Room;
                        SessionMvpRoomAuthoring room{};
                        room.key = sectionKey;
                        bundle.roomByKey[sectionKey] = room;
                    }
                    else if (sectionName == "mission")
                    {
                        sectionType = SectionType::Mission;
                        SessionMvpMissionAuthoring mission{};
                        mission.phaseKey = sectionKey;
                        bundle.missionByPhaseKey[sectionKey] = mission;
                    }
                    else
                    {
                        sectionType = SectionType::None;
                        sectionKey.clear();
                    }

                    continue;
                }

                const size_t split = trimmed.find('=');
                if (split == std::string::npos)
                {
                    bundle.validationErrors.push_back("invalid authoring line: " + trimmed);
                    continue;
                }

                const std::string key = toLowerCopy(trimCopy(trimmed.substr(0, split)));
                const std::string value = trimCopy(trimmed.substr(split + 1));

                if (sectionType == SectionType::None)
                {
                    if (key == "schema_version")
                    {
                        bundle.schemaVersion = static_cast<uint32_t>(std::strtoul(value.c_str(), nullptr, 10));
                    }
                    continue;
                }

                if (sectionType == SectionType::Room)
                {
                    SessionMvpRoomAuthoring& room = bundle.roomByKey[sectionKey];
                    if (key == "title")
                    {
                        room.title = value;
                    }
                    else if (key == "description")
                    {
                        room.description = value;
                    }
                    continue;
                }

                if (sectionType == SectionType::Mission)
                {
                    SessionMvpMissionAuthoring& mission = bundle.missionByPhaseKey[sectionKey];
                    if (key == "objective")
                    {
                        mission.objective = value;
                    }
                    else if (key == "journal")
                    {
                        mission.journal = value;
                    }
                }
            }

            for (const auto& [key, room] : bundle.roomByKey)
            {
                if (key.empty() || room.title.empty() || room.description.empty())
                {
                    bundle.validationErrors.push_back("room authoring incomplete for key: " + key);
                }
            }

            for (const auto& [key, mission] : bundle.missionByPhaseKey)
            {
                if (key.empty() || mission.objective.empty() || mission.journal.empty())
                {
                    bundle.validationErrors.push_back("mission authoring incomplete for phase: " + key);
                }
            }

            bundle.valid = bundle.validationErrors.empty();
            return bundle;
        }

    private:
        [[nodiscard]] static std::string trimCopy(std::string value)
        {
            value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }));
            value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }).base(), value.end());
            return value;
        }

        [[nodiscard]] static std::string toLowerCopy(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
            {
                return static_cast<char>(std::tolower(ch));
            });
            return value;
        }
    };
}
