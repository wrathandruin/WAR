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
            Social,
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
            if (containsToken(lowered, "social |")) return SessionFeedPartition::Social;
            if (!loweredRoomTitle.empty() && containsToken(lowered, loweredRoomTitle)) return SessionFeedPartition::Room;
            if (containsToken(lowered, "room entry:") || containsToken(lowered, "location:")) return SessionFeedPartition::Room;
            if (containsToken(lowered, "mission") || containsToken(lowered, "objective") || containsToken(lowered, "journal")
                || containsToken(lowered, "phase") || containsToken(lowered, "encounter") || containsToken(lowered, "combat")
                || containsToken(lowered, "hazard") || containsToken(lowered, "relay") || containsToken(lowered, "orbit")
                || containsToken(lowered, "frontier"))
            {
                return SessionFeedPartition::Mission;
            }
            return SessionFeedPartition::System;
        }

        std::string latestPartitionEntry(const std::vector<std::string>& eventLog, SessionFeedPartition wanted, const std::string& roomTitle, std::string_view fallback)
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
        const std::string& commandBarText,
        const std::string& environmentIdentityText,
        const std::string& textPresentationProfile,
        uint32_t sameSpacePresenceCount,
        const std::string& sameSpacePresenceSummary,
        const std::string& reconnectRecoveryState,
        const std::string& reconnectContinuitySummary,
        uint32_t reconnectStaleSessionCount) const
    {
#if WAR_HAS_BGFX
        bgfx::dbgTextClear();

        const std::string latestRoom = latestPartitionEntry(eventLog, SessionFeedPartition::Room, roomTitle, "No room feed entries yet.");
        const std::string latestSocial = latestPartitionEntry(eventLog, SessionFeedPartition::Social, roomTitle, "No social feed entries yet.");
        const std::string latestMission = latestPartitionEntry(eventLog, SessionFeedPartition::Mission, roomTitle, "No mission feed entries yet.");
        const std::string latestSystem = latestPartitionEntry(eventLog, SessionFeedPartition::System, roomTitle, "No system feed entries yet.");

        bgfx::dbgTextPrintf(1, 1, 0x0f, "WAR M58 art-integrated session shell");
        bgfx::dbgTextPrintf(1, 2, 0x0a, "Status: %s", statusMessage.c_str());

        char buffer[200]{};
        std::snprintf(buffer, sizeof(buffer), "Player: (%.1f, %.1f)  Entities: %zu  dt: %.4f", playerPosition.x, playerPosition.y, worldState.entities().count(), lastDeltaTime);
        bgfx::dbgTextPrintf(1, 4, 0x0f, "%s", buffer);
        bgfx::dbgTextPrintf(1, 5, 0x0d, "Location: %s", roomTitle.c_str());
        bgfx::dbgTextPrintf(1, 6, 0x0e, "Identity: %s", environmentIdentityText.c_str());
        bgfx::dbgTextPrintf(1, 7, 0x0e, "Profile: %s", textPresentationProfile.c_str());
        bgfx::dbgTextPrintf(1, 8, 0x0a, "Vitals: %s", promptLine.c_str());
        bgfx::dbgTextPrintf(1, 9, 0x0b, "Command: %s", commandBarText.c_str());
        bgfx::dbgTextPrintf(1, 10, 0x0b, "Help: help | look | say | emote | inv | mission | journal");
        bgfx::dbgTextPrintf(1, 11, 0x0b, "More: style | status | session | resume");
        bgfx::dbgTextPrintf(1, 12, 0x0c, "Nearby: %u | %s", sameSpacePresenceCount, sameSpacePresenceSummary.c_str());
        bgfx::dbgTextPrintf(1, 13, 0x0c, "Reconnect: %s | stale=%u", reconnectRecoveryState.c_str(), reconnectStaleSessionCount);
        bgfx::dbgTextPrintf(1, 14, 0x0c, "Continuity: %s", reconnectContinuitySummary.c_str());
        bgfx::dbgTextPrintf(1, 15, 0x0e, "Mission: %s", simulationDiagnostics.missionObjectiveText.c_str());

        bgfx::dbgTextPrintf(1, 17, 0x0d, "Room Feed:");
        bgfx::dbgTextPrintf(3, 18, 0x0f, "%s", latestRoom.c_str());
        bgfx::dbgTextPrintf(1, 20, 0x0a, "Social Feed:");
        bgfx::dbgTextPrintf(3, 21, 0x0f, "%s", latestSocial.c_str());
        bgfx::dbgTextPrintf(1, 23, 0x0e, "Mission Feed:");
        bgfx::dbgTextPrintf(3, 24, 0x0f, "%s", latestMission.c_str());
        bgfx::dbgTextPrintf(1, 26, 0x0c, "System Feed:");
        bgfx::dbgTextPrintf(3, 27, 0x0f, "%s", latestSystem.c_str());
#else
        (void)worldState; (void)playerPosition; (void)eventLog; (void)lastDeltaTime; (void)statusMessage;
        (void)simulationDiagnostics; (void)roomTitle; (void)promptLine; (void)commandBarText; (void)environmentIdentityText;
        (void)textPresentationProfile; (void)sameSpacePresenceCount; (void)sameSpacePresenceSummary; (void)reconnectRecoveryState;
        (void)reconnectContinuitySummary; (void)reconnectStaleSessionCount;
#endif
    }
}
