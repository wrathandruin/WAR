#include "engine/render/BgfxDebugFrameRenderer.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

#if defined(__has_include)
#  if __has_include(<bgfx/bgfx.h>)
#    define WAR_HAS_BGFX 1
#  else
#    define WAR_HAS_BGFX 0
#  endif
#else
#  define WAR_HAS_BGFX 0
#endif

#if WAR_HAS_BGFX
#  include <bgfx/bgfx.h>
#endif

namespace war
{
    namespace
    {
        enum class SessionFeedPartition
        {
            Room,
            Mission,
            System
        };

        std::string toLowerCopy(std::string_view value)
        {
            std::string lowered(value.begin(), value.end());
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch)
            {
                return static_cast<char>(std::tolower(ch));
            });
            return lowered;
        }

        bool containsToken(const std::string& haystack, const std::string& needle)
        {
            return haystack.find(needle) != std::string::npos;
        }

        SessionFeedPartition classifyEntry(const std::string& entry, const std::string& roomTitle)
        {
            const std::string lowered = toLowerCopy(entry);
            const std::string loweredRoomTitle = toLowerCopy(roomTitle);

            if (!loweredRoomTitle.empty() && containsToken(lowered, loweredRoomTitle))
            {
                return SessionFeedPartition::Room;
            }

            if (containsToken(lowered, "room entry:") || containsToken(lowered, "location:"))
            {
                return SessionFeedPartition::Room;
            }

            if (containsToken(lowered, "mission")
                || containsToken(lowered, "objective")
                || containsToken(lowered, "phase")
                || containsToken(lowered, "encounter")
                || containsToken(lowered, "combat")
                || containsToken(lowered, "hazard")
                || containsToken(lowered, "relay")
                || containsToken(lowered, "orbit")
                || containsToken(lowered, "frontier"))
            {
                return SessionFeedPartition::Mission;
            }

            return SessionFeedPartition::System;
        }

        std::string latestPartitionEntry(
            const std::vector<std::string>& eventLog,
            SessionFeedPartition wanted,
            const std::string& roomTitle,
            std::string_view fallback)
        {
            for (auto it = eventLog.rbegin(); it != eventLog.rend(); ++it)
            {
                if (classifyEntry(*it, roomTitle) == wanted)
                {
                    return *it;
                }
            }

            return std::string(fallback);
        }
    }

    void BgfxDebugFrameRenderer::render(
        const WorldState& worldState,
        const Vec2& playerPosition,
        const std::vector<std::string>& eventLog,
        float lastDeltaTime,
        const std::string& statusMessage,
        const SharedSimulationDiagnostics& simulationDiagnostics,
        const std::string& roomTitle,
        const std::string& promptLine,
        const std::string& commandBarText) const
    {
#if WAR_HAS_BGFX
        bgfx::dbgTextClear();

        const std::string latestRoom = latestPartitionEntry(eventLog, SessionFeedPartition::Room, roomTitle, "No room feed entries yet.");
        const std::string latestMission = latestPartitionEntry(eventLog, SessionFeedPartition::Mission, roomTitle, "No mission feed entries yet.");
        const std::string latestSystem = latestPartitionEntry(eventLog, SessionFeedPartition::System, roomTitle, "No system feed entries yet.");

        bgfx::dbgTextPrintf(1, 1, 0x0f, "WAR M53 command shell");
        bgfx::dbgTextPrintf(1, 2, 0x0a, "Status: %s", statusMessage.c_str());

        char buffer[200]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "Player: (%.1f, %.1f)  Entities: %zu  dt: %.4f",
            playerPosition.x,
            playerPosition.y,
            worldState.entities().count(),
            lastDeltaTime);
        bgfx::dbgTextPrintf(1, 4, 0x0f, "%s", buffer);
        bgfx::dbgTextPrintf(1, 5, 0x0d, "Location: %s", roomTitle.c_str());
        bgfx::dbgTextPrintf(1, 6, 0x0e, "Mission: %s", simulationDiagnostics.missionPhaseText.c_str());
        bgfx::dbgTextPrintf(1, 7, 0x0e, "Objective: %s", simulationDiagnostics.missionObjectiveText.c_str());
        bgfx::dbgTextPrintf(1, 8, 0x0a, "Vitals: %s", promptLine.c_str());
        bgfx::dbgTextPrintf(1, 9, 0x0b, "Command: %s", commandBarText.c_str());
        bgfx::dbgTextPrintf(1, 10, 0x0b, "Help: help | look | say | emote | inv");

        bgfx::dbgTextPrintf(1, 12, 0x0d, "Room Feed:");
        bgfx::dbgTextPrintf(3, 13, 0x0f, "%s", latestRoom.c_str());
        bgfx::dbgTextPrintf(1, 15, 0x0e, "Mission Feed:");
        bgfx::dbgTextPrintf(3, 16, 0x0f, "%s", latestMission.c_str());
        bgfx::dbgTextPrintf(1, 18, 0x0c, "System Feed:");
        bgfx::dbgTextPrintf(3, 19, 0x0f, "%s", latestSystem.c_str());
#else
        (void)worldState;
        (void)playerPosition;
        (void)eventLog;
        (void)lastDeltaTime;
        (void)statusMessage;
        (void)simulationDiagnostics;
        (void)roomTitle;
        (void)promptLine;
        (void)commandBarText;
#endif
    }
}
