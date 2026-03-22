#include "engine/render/DebugOverlayRenderer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

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

        struct PartitionedSessionFeed
        {
            std::vector<std::string> roomEntries;
            std::vector<std::string> socialEntries;
            std::vector<std::string> missionEntries;
            std::vector<std::string> systemEntries;
        };

        std::string tileText(bool hasTile, TileCoord tile)
        {
            if (!hasTile)
            {
                return "none";
            }
            return "(" + std::to_string(tile.x) + ", " + std::to_string(tile.y) + ")";
        }

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

        SessionFeedPartition classifyEntry(const std::string& entry, const std::string& roomTitle, const std::string& roomDescription)
        {
            const std::string lowered = toLowerCopy(entry);
            const std::string loweredRoomTitle = toLowerCopy(roomTitle);
            const std::string loweredRoomDescription = toLowerCopy(roomDescription);

            if (containsToken(lowered, "social |"))
            {
                return SessionFeedPartition::Social;
            }
            if (!loweredRoomTitle.empty() && containsToken(lowered, loweredRoomTitle))
            {
                return SessionFeedPartition::Room;
            }
            if (!loweredRoomDescription.empty() && lowered == loweredRoomDescription)
            {
                return SessionFeedPartition::Room;
            }
            if (containsToken(lowered, "room entry:") || containsToken(lowered, "location:"))
            {
                return SessionFeedPartition::Room;
            }
            if (containsToken(lowered, "mission") || containsToken(lowered, "objective") || containsToken(lowered, "journal")
                || containsToken(lowered, "phase") || containsToken(lowered, "encounter") || containsToken(lowered, "combat")
                || containsToken(lowered, "hazard") || containsToken(lowered, "relay") || containsToken(lowered, "orbit")
                || containsToken(lowered, "frontier"))
            {
                return SessionFeedPartition::Mission;
            }
            return SessionFeedPartition::System;
        }

        PartitionedSessionFeed partitionFeed(const std::vector<std::string>& eventLog, const std::string& roomTitle, const std::string& roomDescription, size_t maxEntriesPerPartition)
        {
            PartitionedSessionFeed feed{};
            for (auto it = eventLog.rbegin(); it != eventLog.rend(); ++it)
            {
                std::vector<std::string>* target = nullptr;
                switch (classifyEntry(*it, roomTitle, roomDescription))
                {
                case SessionFeedPartition::Room: target = &feed.roomEntries; break;
                case SessionFeedPartition::Social: target = &feed.socialEntries; break;
                case SessionFeedPartition::Mission: target = &feed.missionEntries; break;
                case SessionFeedPartition::System: target = &feed.systemEntries; break;
                }
                if (target != nullptr && target->size() < maxEntriesPerPartition)
                {
                    target->push_back(*it);
                }
            }
            std::reverse(feed.roomEntries.begin(), feed.roomEntries.end());
            std::reverse(feed.socialEntries.begin(), feed.socialEntries.end());
            std::reverse(feed.missionEntries.begin(), feed.missionEntries.end());
            std::reverse(feed.systemEntries.begin(), feed.systemEntries.end());
            return feed;
        }

        std::string buildFeedBlock(std::string_view title, const std::vector<std::string>& entries, std::string_view emptyText)
        {
            std::ostringstream output;
            output << title << "\n";
            if (entries.empty())
            {
                output << emptyText;
                return output.str();
            }
            for (const std::string& entry : entries)
            {
                output << "- " << entry << "\n";
            }
            return output.str();
        }
    }

    const char* DebugOverlayRenderer::entityTypeToText(EntityType type) const
    {
        switch (type)
        {
        case EntityType::Crate: return "crate";
        case EntityType::Terminal: return "terminal";
        case EntityType::Locker: return "locker";
        case EntityType::Ship: return "ship";
        default: return "unknown";
        }
    }

    const char* DebugOverlayRenderer::hotspotTypeToText(WorldAuthoringHotspotType type) const
    {
        switch (type)
        {
        case WorldAuthoringHotspotType::Encounter: return "encounter";
        case WorldAuthoringHotspotType::Control: return "control";
        case WorldAuthoringHotspotType::Transit: return "transit";
        case WorldAuthoringHotspotType::Loot: return "loot";
        case WorldAuthoringHotspotType::Hazard: return "hazard";
        default: return "unknown";
        }
    }

    void DebugOverlayRenderer::render(
        HDC dc,
        const WorldState& worldState,
        const Camera2D& camera,
        const Vec2& playerPosition,
        const std::vector<TileCoord>& currentPath,
        size_t pathIndex,
        bool hasHoveredTile,
        TileCoord hoveredTile,
        bool hasSelectedTile,
        TileCoord selectedTile,
        bool hasActionTargetTile,
        TileCoord actionTargetTile,
        const std::vector<std::string>& eventLog,
        float lastDeltaTime,
        const POINT& mousePosition,
        const RuntimeBoundaryReport& runtimeBoundaryReport,
        const LocalDemoDiagnosticsReport& localDemoDiagnosticsReport,
        const SharedSimulationDiagnostics& simulationDiagnostics,
        const HeadlessHostPresenceReport& headlessHostPresenceReport,
        const AuthoritativeHostProtocolReport& authoritativeHostProtocolReport,
        const std::string& roomTitle,
        const std::string& roomDescription,
        const std::string& promptLine,
        const std::string& environmentIdentityText,
        const std::string& textPresentationProfile,
        const std::string& artManifestPath,
        const std::string& commandBarText,
        const std::string& commandEcho,
        uint32_t sameSpacePresenceCount,
        const std::string& sameSpacePresenceSummary,
        const std::string& presenceInterestRule,
        const std::string& reconnectRecoveryState,
        const std::string& reconnectContinuitySummary,
        uint32_t reconnectStaleSessionCount) const
    {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(225, 225, 225));

        const Vec2 mouseWorld = camera.screenToWorld(mousePosition.x, mousePosition.y);
        const TileCoord mouseTile = worldState.world().worldToTile(mouseWorld);
        const TileCoord playerTile = worldState.world().worldToTile(playerPosition);
        const std::string hovered = tileText(hasHoveredTile, hoveredTile);
        const std::string selected = tileText(hasSelectedTile, selectedTile);
        const std::string actionTarget = tileText(hasActionTargetTile, actionTargetTile);
        const std::string pathDestination = currentPath.empty() || pathIndex >= currentPath.size() ? "none" : tileText(true, currentPath.back());

        std::ostringstream info;
        info
            << "WAR Milestone 58\n"
            << "Session shell: art integration / environment identity / text presentation\n"
            << "Connect target: " << localDemoDiagnosticsReport.connectTargetName << "\n"
            << "Transport: " << localDemoDiagnosticsReport.connectTransport << "\n"
            << "Authority lane: " << (simulationDiagnostics.hostAuthorityActive ? "headless host" : "local") << "\n"
            << "Host session: " << headlessHostPresenceReport.sessionId << "\n"
            << "Protocol lane ready: " << (authoritativeHostProtocolReport.authorityLaneReady ? "yes" : "no") << "\n"
            << "Environment identity: " << environmentIdentityText << "\n"
            << "Presentation profile: " << textPresentationProfile << "\n"
            << "Art manifest: " << artManifestPath << "\n"
            << "Mission phase: " << simulationDiagnostics.missionPhaseText << "\n"
            << "Reconnect state: " << reconnectRecoveryState << "\n"
            << "Stale session count: " << reconnectStaleSessionCount << "\n"
            << "Nearby presence: " << sameSpacePresenceCount << "\n"
            << "Interest rule: " << presenceInterestRule << "\n"
            << "Player tile: (" << playerTile.x << ", " << playerTile.y << ")\n"
            << "Mouse tile: (" << mouseTile.x << ", " << mouseTile.y << ")\n"
            << "Hovered tile: " << hovered << "\n"
            << "Selected tile: " << selected << "\n"
            << "Move target: " << actionTarget << "\n"
            << "Path destination: " << pathDestination << "\n"
            << "Runtime mode: " << (runtimeBoundaryReport.runningFromSourceTree ? "source-tree" : "packaged") << "\n"
            << "Frame dt: " << lastDeltaTime;

        const std::string infoText = info.str();
        RECT infoRect{ 16, 16, 760, 500 };
        RECT measureRect = infoRect;
        DrawTextA(dc, infoText.c_str(), -1, &measureRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_CALCRECT);
        DrawTextA(dc, infoText.c_str(), -1, &infoRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK);

        std::ostringstream presentation;
        presentation
            << "Session HUD\n"
            << "Environment Identity\n" << environmentIdentityText << "\n\n"
            << "Text Presentation\n" << textPresentationProfile << "\n\n"
            << "Location\n" << roomTitle << "\n\n" << roomDescription << "\n\n"
            << "Mission\n"
            << "Phase: " << simulationDiagnostics.missionPhaseText << "\n"
            << "Objective: " << simulationDiagnostics.missionObjectiveText << "\n"
            << "Last beat: " << simulationDiagnostics.missionLastBeat << "\n\n"
            << "Prompt / Vitals\n" << promptLine << "\n\n"
            << "Command Bar\n" << commandBarText << "\n\n"
            << "Nearby Presence\n" << sameSpacePresenceCount << " active | " << sameSpacePresenceSummary << "\n\n"
            << "Reconnect Continuity\n" << reconnectContinuitySummary << "\n\n"
            << "Command Help\nhelp | look | say <text> | emote <text> | inv | mission | journal | style\n\n"
            << "Reply\n" << commandEcho;

        const std::string presentationText = presentation.str();
        RECT presentationRect{ 800, 16, 1560, 900 };
        DrawTextA(dc, presentationText.c_str(), -1, &presentationRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK);

        const PartitionedSessionFeed feed = partitionFeed(eventLog, roomTitle, roomDescription, 4u);
        int sectionTop = measureRect.bottom + 24;
        const std::array<std::string, 4> blocks{
            buildFeedBlock("Room Feed", feed.roomEntries, "No room feed entries captured yet."),
            buildFeedBlock("Social Feed", feed.socialEntries, "No social feed entries captured yet."),
            buildFeedBlock("Mission Feed", feed.missionEntries, "No mission feed entries captured yet."),
            buildFeedBlock("System Feed", feed.systemEntries, "No system feed entries captured yet.")
        };
        for (const std::string& block : blocks)
        {
            RECT blockRect{ 16, sectionTop, 760, sectionTop + 150 };
            DrawTextA(dc, block.c_str(), -1, &blockRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK);
            sectionTop += 145;
        }
    }
}
