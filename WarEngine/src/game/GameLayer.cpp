#include "game/GameLayer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <windows.h>

#include "engine/host/HeadlessHostPresence.h"
#include "engine/host/ReconnectSessionProtocol.h"
#include "engine/render/BgfxRenderDevice.h"
#include "engine/render/GdiRenderDevice.h"

namespace war
{
    namespace
    {
        constexpr uint32_t kExpectedProtocolVersion = 2u;
        constexpr uint64_t kConnectFailureTimeoutMilliseconds = 2500ull;

        bool configsEqual(const ReplicationHarnessConfig& lhs, const ReplicationHarnessConfig& rhs)
        {
            return lhs.enabled == rhs.enabled
                && lhs.intentLatencyMilliseconds == rhs.intentLatencyMilliseconds
                && lhs.acknowledgementLatencyMilliseconds == rhs.acknowledgementLatencyMilliseconds
                && lhs.snapshotLatencyMilliseconds == rhs.snapshotLatencyMilliseconds
                && lhs.jitterMilliseconds == rhs.jitterMilliseconds;
        }

        std::string sanitizeSingleLine(const std::string& value)
        {
            std::string sanitized = value;
            for (char& ch : sanitized)
            {
                if (ch == '\n' || ch == '\r' || ch == '\t')
                {
                    ch = ' ';
                }
            }

            return sanitized;
        }

        std::string harnessSummary(const ReplicationHarnessConfig& config)
        {
            std::ostringstream summary;
            summary
                << (config.enabled ? "enabled" : "disabled")
                << " | latency ms: "
                << config.intentLatencyMilliseconds << "/"
                << config.acknowledgementLatencyMilliseconds << "/"
                << config.snapshotLatencyMilliseconds
                << " | jitter ms: " << config.jitterMilliseconds;
            return summary.str();
        }

        bool writeTextFileAtomically(const std::filesystem::path& path, const std::string& contents)
        {
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error)
            {
                return false;
            }

            const std::filesystem::path tempPath = path.parent_path() / (path.filename().string() + ".tmp");
            std::ofstream output(tempPath, std::ios::out | std::ios::trunc);
            if (!output.is_open())
            {
                return false;
            }

            output << contents;
            output.close();
            if (!output)
            {
                std::filesystem::remove(tempPath, error);
                return false;
            }

        #if defined(_WIN32)
            if (!MoveFileExW(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
                std::filesystem::remove(tempPath, error);
                return false;
            }
        #else
            std::filesystem::rename(tempPath, path, error);
            if (error)
            {
                std::filesystem::remove(tempPath, error);
                return false;
            }
        #endif

            return true;
        }

        bool sameSnapshotIdentity(
            const AuthoritativeWorldSnapshot& snapshot,
            uint64_t publishedEpochMilliseconds,
            uint64_t simulationTicks,
            uint64_t lastProcessedIntentSequence)
        {
            return snapshot.publishedEpochMilliseconds == publishedEpochMilliseconds
                && snapshot.simulationTicks == simulationTicks
                && snapshot.lastProcessedIntentSequence == lastProcessedIntentSequence;
        }

        void appendClientTrace(const RuntimeBoundaryReport& runtimeBoundaryReport, std::string_view line)
        {
            LocalDemoDiagnostics::appendTraceLine(runtimeBoundaryReport, "client_runtime_trace.txt", line);
        }

        std::string buildClientIdentity(const char* prefix)
        {
            return std::string(prefix)
                + "-"
                + std::to_string(GetCurrentProcessId())
                + "-"
                + std::to_string(ReplicationHarness::currentEpochMilliseconds());
        }

        constexpr std::array<const char*, 8u> kOperatorAliasOrder{
            "alpha",
            "bravo",
            "charlie",
            "delta",
            "echo",
            "foxtrot",
            "golf",
            "hotel"
        };

        std::filesystem::path clientSlotClaimPath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& clientSlot)
        {
            return runtimeBoundaryReport.configDirectory / "ClientSlots" / (clientSlot + ".lock");
        }

        bool parseSimpleKeyValueFile(const std::filesystem::path& path, std::unordered_map<std::string, std::string>& outValues);

        std::wstring readEnvironmentWide(const wchar_t* variableName)
        {
            const DWORD requiredLength = GetEnvironmentVariableW(variableName, nullptr, 0);
            if (requiredLength == 0)
            {
                return {};
            }

            std::wstring buffer(static_cast<size_t>(requiredLength), L'\0');
            const DWORD writtenLength = GetEnvironmentVariableW(variableName, buffer.data(), requiredLength);
            if (writtenLength == 0)
            {
                return {};
            }

            buffer.resize(static_cast<size_t>(writtenLength));
            return buffer;
        }

        std::string narrowText(const std::wstring& value)
        {
            if (value.empty())
            {
                return {};
            }

            const int requiredBytes = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.c_str(),
                static_cast<int>(value.size()),
                nullptr,
                0,
                nullptr,
                nullptr);
            if (requiredBytes <= 0)
            {
                return {};
            }

            std::string result(static_cast<size_t>(requiredBytes), '\0');
            const int writtenBytes = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.c_str(),
                static_cast<int>(value.size()),
                result.data(),
                requiredBytes,
                nullptr,
                nullptr);
            if (writtenBytes <= 0)
            {
                return {};
            }

            result.resize(static_cast<size_t>(writtenBytes));
            return result;
        }

        std::string sanitizeClientSlot(std::string value)
        {
            value = SessionEntryProtocol::sanitizeIdentifier(value);
            if (value == "unassigned" || value == "none" || value == "unknown")
            {
                return {};
            }
            return value;
        }

        std::string preferredClientSlotFromEnvironment()
        {
            return sanitizeClientSlot(narrowText(readEnvironmentWide(L"WAR_CLIENT_SLOT")));
        }

        void removeFileQuietly(const std::filesystem::path& path)
        {
            std::error_code error;
            std::filesystem::remove(path, error);
        }

        bool processIsStillRunning(const DWORD processId)
        {
            if (processId == 0)
            {
                return false;
            }

            HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
            if (processHandle == nullptr)
            {
                return false;
            }

            DWORD exitCode = 0;
            const bool success = GetExitCodeProcess(processHandle, &exitCode) != 0;
            CloseHandle(processHandle);
            return success && exitCode == STILL_ACTIVE;
        }

        bool readClaimedProcessId(const std::filesystem::path& path, DWORD& outProcessId)
        {
            outProcessId = 0;

            std::unordered_map<std::string, std::string> values{};
            if (!parseSimpleKeyValueFile(path, values))
            {
                return false;
            }

            const auto pidIt = values.find("pid");
            if (pidIt == values.end() || pidIt->second.empty())
            {
                return false;
            }

            try
            {
                outProcessId = static_cast<DWORD>(std::stoul(pidIt->second));
            }
            catch (...)
            {
                outProcessId = 0;
                return false;
            }

            return outProcessId != 0;
        }

        bool tryClaimClientSlot(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& clientSlot,
            const std::string& clientInstanceId,
            std::filesystem::path& outClaimPath)
        {
            outClaimPath.clear();
            if (clientSlot.empty())
            {
                return false;
            }

            const std::filesystem::path claimPath = clientSlotClaimPath(runtimeBoundaryReport, clientSlot);
            std::error_code error;
            std::filesystem::create_directories(claimPath.parent_path(), error);
            if (error)
            {
                return false;
            }

            const auto claimNow = [&]() -> bool
            {
                HANDLE handle = CreateFileW(
                    claimPath.c_str(),
                    GENERIC_WRITE,
                    0,
                    nullptr,
                    CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr);
                if (handle == INVALID_HANDLE_VALUE)
                {
                    return false;
                }

                const std::string contents =
                    std::string("pid=") + std::to_string(GetCurrentProcessId()) + "\n"
                    + "client_instance_id=" + sanitizeSingleLine(clientInstanceId) + "\n"
                    + "claimed_epoch_milliseconds=" + std::to_string(ReplicationHarness::currentEpochMilliseconds()) + "\n";
                DWORD bytesWritten = 0;
                const BOOL writeSucceeded = WriteFile(
                    handle,
                    contents.data(),
                    static_cast<DWORD>(contents.size()),
                    &bytesWritten,
                    nullptr);
                CloseHandle(handle);

                if (!writeSucceeded || bytesWritten != contents.size())
                {
                    removeFileQuietly(claimPath);
                    return false;
                }

                outClaimPath = claimPath;
                return true;
            };

            if (claimNow())
            {
                return true;
            }

            DWORD claimedProcessId = 0;
            if (!readClaimedProcessId(claimPath, claimedProcessId) || !processIsStillRunning(claimedProcessId))
            {
                removeFileQuietly(claimPath);
                return claimNow();
            }

            return false;
        }

        bool containsValue(const std::unordered_set<std::string>& values, const std::string& value)
        {
            return values.find(value) != values.end();
        }

        std::string extractOperatorAlias(const std::string& playerIdentity)
        {
            const std::string sanitized = SessionEntryProtocol::sanitizeIdentifier(playerIdentity);
            constexpr std::string_view prefix = "operator-";
            if (sanitized.rfind(prefix, 0) != 0 || sanitized.size() <= prefix.size())
            {
                return {};
            }
            return sanitized.substr(prefix.size());
        }

        std::string extractAccountAlias(const std::string& accountId)
        {
            const std::string sanitized = SessionEntryProtocol::sanitizeIdentifier(accountId);
            constexpr std::string_view prefix = "internal-alpha-player-";
            if (sanitized.rfind(prefix, 0) != 0 || sanitized.size() <= prefix.size())
            {
                return {};
            }
            return sanitized.substr(prefix.size());
        }

        void addUsedAlias(std::unordered_set<std::string>& usedAliases, const std::string& alias)
        {
            const std::string sanitized = sanitizeClientSlot(alias);
            if (!sanitized.empty())
            {
                usedAliases.insert(sanitized);
            }
        }

        std::string selectAvailableClientSlot(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& clientInstanceId,
            std::filesystem::path& outClaimPath)
        {
            outClaimPath.clear();
            const std::string preferredSlot = preferredClientSlotFromEnvironment();
            if (!preferredSlot.empty()
                && tryClaimClientSlot(runtimeBoundaryReport, preferredSlot, clientInstanceId, outClaimPath))
            {
                return preferredSlot;
            }

            std::unordered_set<std::string> usedAliases{};

            for (const ActiveSessionRecord& activeSession : SessionEntryProtocol::collectActiveSessions(runtimeBoundaryReport))
            {
                addUsedAlias(usedAliases, extractOperatorAlias(activeSession.playerIdentity));
                addUsedAlias(usedAliases, extractAccountAlias(activeSession.accountId));
            }

            for (const SessionTicket& ticket : SessionEntryProtocol::collectIssuedTickets(runtimeBoundaryReport))
            {
                addUsedAlias(usedAliases, extractOperatorAlias(ticket.playerIdentity));
                addUsedAlias(usedAliases, extractAccountAlias(ticket.accountId));
            }

            std::string presenceError;
            const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
            const uint64_t freshnessCutoffEpochMilliseconds = nowEpochMilliseconds > 15000ull
                ? nowEpochMilliseconds - 15000ull
                : 0ull;
            for (const LocalPresenceHeartbeat& heartbeat : LocalPresenceProtocol::readHeartbeats(runtimeBoundaryReport, presenceError))
            {
                if (heartbeat.heartbeatEpochMilliseconds < freshnessCutoffEpochMilliseconds)
                {
                    continue;
                }

                addUsedAlias(usedAliases, extractOperatorAlias(heartbeat.playerIdentity));
            }

            for (const char* alias : kOperatorAliasOrder)
            {
                if (!containsValue(usedAliases, alias)
                    && tryClaimClientSlot(runtimeBoundaryReport, alias, clientInstanceId, outClaimPath))
                {
                    return alias;
                }
            }

            for (const char* alias : kOperatorAliasOrder)
            {
                if (tryClaimClientSlot(runtimeBoundaryReport, alias, clientInstanceId, outClaimPath))
                {
                    return alias;
                }
            }

            return std::string("pid-") + std::to_string(GetCurrentProcessId());
        }

        std::filesystem::path slotScopedConfigPath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const char* filename,
            const std::string& clientSlot)
        {
            const std::filesystem::path basePath = runtimeBoundaryReport.configDirectory / filename;
            if (clientSlot.empty())
            {
                return basePath;
            }

            return basePath.parent_path()
                / (basePath.stem().string() + "_" + clientSlot + basePath.extension().string());
        }

        std::filesystem::path clientResumeIdentityPath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& clientSlot)
        {
            return slotScopedConfigPath(runtimeBoundaryReport, "client_resume_identity.txt", clientSlot);
        }

        std::filesystem::path legacyClientResumeIdentityPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.configDirectory / "client_resume_identity.txt";
        }

        std::filesystem::path clientLocationContextPath(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& clientSlot)
        {
            return slotScopedConfigPath(runtimeBoundaryReport, "client_location_context.txt", clientSlot);
        }

        std::filesystem::path legacyClientLocationContextPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.configDirectory / "client_location_context.txt";
        }

        bool parseSimpleKeyValueFile(const std::filesystem::path& path, std::unordered_map<std::string, std::string>& outValues)
        {
            outValues.clear();
            std::ifstream input(path, std::ios::in);
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

                outValues.emplace(line.substr(0, split), line.substr(split + 1));
            }

            return true;
        }

        std::string trimCopy(std::string value)
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

        std::string toLowerTrim(std::string value)
        {
            value = trimCopy(std::move(value));
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
            {
                return static_cast<char>(std::tolower(ch));
            });

            return value;
        }

        Vec2 remotePresenceOffset(const std::string& participantId, int tileSize)
        {
            const std::array<Vec2, 4u> offsets{
                Vec2{ -0.18f, -0.18f },
                Vec2{ 0.18f, -0.18f },
                Vec2{ -0.18f, 0.18f },
                Vec2{ 0.18f, 0.18f }
            };

            const size_t index = std::hash<std::string>{}(participantId) % offsets.size();
            return Vec2{
                offsets[index].x * static_cast<float>(tileSize),
                offsets[index].y * static_cast<float>(tileSize)
            };
        }

        float clamp01(float value)
        {
            if (value <= 0.0f)
            {
                return 0.0f;
            }

            if (value >= 1.0f)
            {
                return 1.0f;
            }

            return value;
        }

        Vec2 lerpVec2(const Vec2& from, const Vec2& to, float alpha)
        {
            return from + ((to - from) * clamp01(alpha));
        }

        enum class RoutedCommandType
        {
            None,
            Help,
            Look,
            Say,
            Emote,
            Inventory,
            Status,
            Mission,
            Journal,
            Session,
            Entry,
            Resume,
            Inspect,
            Interact,
            Move,
            Clear,
            Style,
            Unknown
        };

        struct RoutedCommand
        {
            RoutedCommandType type = RoutedCommandType::None;
            std::string rawText;
            std::string verb;
            std::string argumentText;
            int moveX = 0;
            int moveY = 0;
            bool syntaxValid = false;
        };

        RoutedCommand parseRoutedCommand(const std::string& commandLine)
        {
            RoutedCommand command{};
            command.rawText = trimCopy(commandLine);
            if (command.rawText.empty())
            {
                return command;
            }

            const size_t split = command.rawText.find_first_of(" \t");
            const std::string rawVerb = split == std::string::npos
                ? command.rawText
                : command.rawText.substr(0, split);
            command.verb = toLowerTrim(rawVerb);
            command.argumentText = split == std::string::npos
                ? std::string()
                : trimCopy(command.rawText.substr(split + 1));

            if (command.verb == "help" || command.verb == "?")
            {
                command.type = RoutedCommandType::Help;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "look" || command.verb == "room" || command.verb == "where" || command.verb == "l")
            {
                command.type = RoutedCommandType::Look;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "say")
            {
                command.type = RoutedCommandType::Say;
                command.syntaxValid = !command.argumentText.empty();
                return command;
            }

            if (command.verb == "emote" || command.verb == "me")
            {
                command.type = RoutedCommandType::Emote;
                command.syntaxValid = !command.argumentText.empty();
                return command;
            }

            if (command.verb == "inv" || command.verb == "inventory" || command.verb == "i")
            {
                command.type = RoutedCommandType::Inventory;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "status" || command.verb == "vitals")
            {
                command.type = RoutedCommandType::Status;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "mission")
            {
                command.type = RoutedCommandType::Mission;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "journal")
            {
                command.type = RoutedCommandType::Journal;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "style" || command.verb == "identity")
            {
                command.type = RoutedCommandType::Style;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "session")
            {
                command.type = RoutedCommandType::Session;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "entry")
            {
                command.type = RoutedCommandType::Entry;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "resume")
            {
                command.type = RoutedCommandType::Resume;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "inspect")
            {
                command.type = RoutedCommandType::Inspect;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "interact")
            {
                command.type = RoutedCommandType::Interact;
                command.syntaxValid = true;
                return command;
            }

            if (command.verb == "move")
            {
                command.type = RoutedCommandType::Move;
                std::istringstream input(command.argumentText);
                int x = 0;
                int y = 0;
                char trailing = '\0';
                if ((input >> x >> y) && !(input >> trailing))
                {
                    command.moveX = x;
                    command.moveY = y;
                    command.syntaxValid = true;
                }
                return command;
            }

            if (command.verb == "clear")
            {
                command.type = RoutedCommandType::Clear;
                command.syntaxValid = true;
                return command;
            }

            command.type = RoutedCommandType::Unknown;
            return command;
        }

        std::string buildCommandHelpText()
        {
            return "Commands: help, look, say <text>, emote <text>, inv, mission, journal, style. say/emote visibility is same-space only. resume restores persisted session continuity. Support: status, session, entry, resume, inspect, interact, move <x> <y>, clear.";
        }

        std::string buildInventoryShellText(const SharedSimulationDiagnostics& diagnostics)
        {
            std::ostringstream inventory;
            inventory
                << "Inventory: " << diagnostics.inventorySummary
                << " | Weapon: " << diagnostics.equippedWeaponText
                << " | Suit: " << diagnostics.equippedSuitText
                << " | Tool: " << diagnostics.equippedToolText;
            return inventory.str();
        }

        std::filesystem::path resolveSessionMvpAuthoringPath(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            const std::vector<std::filesystem::path> candidates{
                runtimeBoundaryReport.executableDirectory / "Content" / "SessionMvp" / "session_mvp_authoring.txt",
                runtimeBoundaryReport.runtimeRoot / "Content" / "SessionMvp" / "session_mvp_authoring.txt",
                runtimeBoundaryReport.repoRoot / "Content" / "SessionMvp" / "session_mvp_authoring.txt",
                runtimeBoundaryReport.assetRoot / "text" / "session_mvp_authoring.txt"
            };

            for (const std::filesystem::path& candidate : candidates)
            {
                std::error_code error;
                if (!candidate.empty() && std::filesystem::exists(candidate, error) && std::filesystem::is_regular_file(candidate, error))
                {
                    return candidate;
                }
            }

            return {};
        }

        std::string buildFallbackMissionJournalText(const SharedSimulationDiagnostics& diagnostics)
        {
            if (!diagnostics.missionLastBeat.empty() && diagnostics.missionLastBeat != "none")
            {
                return diagnostics.missionLastBeat;
            }

            if (!diagnostics.missionObjectiveText.empty())
            {
                return diagnostics.missionObjectiveText;
            }

            return "No mission journal entry is available for the current session phase.";
        }

        std::string joinSummary(const std::vector<std::string>& values, std::string_view separator)
        {
            if (values.empty())
            {
                return "none";
            }

            std::ostringstream joined;
            for (size_t index = 0; index < values.size(); ++index)
            {
                if (index > 0)
                {
                    joined << separator;
                }
                joined << values[index];
            }

            return joined.str();
        }

        ResolvedLocationContext makeLocationContext(
            const std::string& key,
            const std::string& title,
            const std::string& description)
        {
            ResolvedLocationContext context{};
            context.valid = true;
            context.key = key;
            context.title = title;
            context.entryDescription = description;
            return context;
        }

        ResolvedLocationContext resolveOrbitalLocationContext(const SharedSimulationDiagnostics& diagnostics)
        {
            if (diagnostics.orbitalCurrentNodeText == "traffic-separation-lane")
            {
                return makeLocationContext(
                    "orbital.traffic-separation-lane",
                    "Traffic Separation Lane",
                    "The shuttle holds inside Khepri traffic separation, bracketed by traffic-control windows and collision-avoidance protocol.");
            }

            if (diagnostics.orbitalCurrentNodeText == "debris-survey-orbit")
            {
                return makeLocationContext(
                    "orbital.debris-survey-orbit",
                    "Debris Survey Orbit",
                    "Broken survey debris, sparse traffic telemetry, and broad orbital sightlines turn this orbit into a cold operational watchpoint.");
            }

            if (diagnostics.orbitalCurrentNodeText == "relay-holding-track")
            {
                return makeLocationContext(
                    "orbital.relay-holding-track",
                    "Relay Holding Track",
                    "The shuttle paces a narrow relay holding track where approach timing and platform clearance are more important than comfort.");
            }

            if (diagnostics.orbitalCurrentNodeText == "relay-platform-dock"
                || diagnostics.orbitalPhaseText == "relay-platform-docked")
            {
                return makeLocationContext(
                    "orbital.relay-platform-dock",
                    "Relay Platform Dock",
                    "Docking clamps, relay trusswork, and hard-vacuum service lanes crowd the shuttle against the survey platform.");
            }

            if (diagnostics.orbitalCurrentNodeText == "return-traffic-lane")
            {
                return makeLocationContext(
                    "orbital.return-traffic-lane",
                    "Return Traffic Lane",
                    "Return traffic narrows into a controlled recovery lane where every approach window points the shuttle back toward Khepri.");
            }

            if (diagnostics.orbitalCurrentNodeText == "home-dock-anchor"
                || diagnostics.orbitalPhaseText == "home-docked")
            {
                return makeLocationContext(
                    "orbital.home-dock-anchor",
                    "Home Dock Anchor",
                    "Khepri's home-dock anchor feels controlled and procedural, with docking geometry and return vectors replacing the uncertainty of open transfer.");
            }

            std::ostringstream description;
            description << "The shuttle hangs in Khepri's controlled orbital anchor while departure windows are metered through the local traffic shell.";
            if (!diagnostics.orbitalRuleText.empty())
            {
                description << ' ' << diagnostics.orbitalRuleText;
            }

            return makeLocationContext(
                "orbital.khepri-anchor",
                "Khepri Orbital Anchor",
                description.str());
        }

        ResolvedLocationContext resolveShipboardLocationContext(const SharedSimulationDiagnostics& diagnostics)
        {
            if (diagnostics.shipCommandClaimed)
            {
                return makeLocationContext(
                    "ship.responder-shuttle-khepri.command-cabin",
                    "Responder Shuttle Khepri - Command Cabin",
                    "Harness webbing, helm glass, and route controls crowd the shuttle's command cabin into a compact operational nerve center.");
            }

            return makeLocationContext(
                "ship.responder-shuttle-khepri.docked-interior",
                "Responder Shuttle Khepri - Docked Interior",
                "The responder shuttle feels tight, metallic, and mission-worn, with fold-down seating and sealed bulkheads built for short hard transfers.");
        }

        ResolvedLocationContext resolveFrontierLocationContext()
        {
            return makeLocationContext(
                "frontier.dust-frontier-landing-pad",
                "Dust Frontier Landing Pad",
                "Dust-scoured plating, relay scaffold shadows, and a waiting return corridor make the frontier pad feel exposed, temporary, and very far from home.");
        }

        ResolvedLocationContext resolveRuntimeLocationContext(
            const WorldState& worldState,
            const SharedSimulationDiagnostics& diagnostics,
            TileCoord playerTile)
        {
            if (diagnostics.orbitalLayerActive)
            {
                return resolveOrbitalLocationContext(diagnostics);
            }

            if (diagnostics.shipBoarded)
            {
                return resolveShipboardLocationContext(diagnostics);
            }

            if (diagnostics.frontierSurfaceActive)
            {
                return resolveFrontierLocationContext();
            }

            return worldState.resolveInteriorLocation(playerTile);
        }
    }

    void GameLayer::initialize(IWindow& window)
    {
        m_window = &window;
        m_camera.setViewportSize(window.getWidth(), window.getHeight());
        m_camera.setPosition({ 0.0f, 0.0f });

        m_simulationRuntime.initializeForLocalAuthority();

        const TileCoord spawnTile{ 2, 2 };
        m_selectedTile = spawnTile;
        m_hasSelectedTile = true;

        m_runtimeBoundaryReport = RuntimePaths::buildReport();
        RuntimePaths::ensureRuntimeDirectories(m_runtimeBoundaryReport);
        m_localDemoDiagnosticsReport = LocalDemoDiagnostics::buildReport(m_runtimeBoundaryReport);
        LocalDemoDiagnostics::writeStartupReport(m_runtimeBoundaryReport, m_localDemoDiagnosticsReport);
        m_headlessHostPresenceReport = HeadlessHostPresence::buildReport(m_runtimeBoundaryReport);
        m_authoritativeHostProtocolReport = AuthoritativeHostProtocol::buildReport(m_runtimeBoundaryReport);
        m_replicationHarnessConfig = ReplicationHarness::loadConfig(m_runtimeBoundaryReport);
        m_sessionEntryProtocolReport = SessionEntryProtocol::buildReport(m_runtimeBoundaryReport);

        m_expectedProtocolVersion = kExpectedProtocolVersion;
        m_clientStartedEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        m_clientInstanceId = buildClientIdentity("client");
        m_clientSessionId = buildClientIdentity("client-session");
        m_connectState = "connect-pending";
        m_connectFailureReason = "none";
        initializeSessionIdentityDefaults();
        loadPersistedResumeIdentity();
        loadPersistedLocationContext();
        loadPersistedSocialState();
        loadPersistedReconnectContinuity();
        loadSessionMvpAuthoring();
        updateSessionEntryFlow();

        refreshAuthorityMode();
        updateConnectionTelemetry();
        updateReplicationDiagnostics();
        updatePresentationRuntime();
        updateLocalSocialRuntime();
        updateLocalPresenceRuntime();
        updateReconnectRecoveryRuntime();
        m_commandEcho = "Command shell ready. Type 'help' for look, say, emote, and inv.";
        writeClientReplicationStatus();

        pushEvent("Milestone 56 initialized");
        pushEvent(std::string("Client instance: ") + m_clientInstanceId);
        pushEvent(std::string("Client session: ") + m_clientSessionId);
        pushEvent(std::string("Client identity slot: ") + m_clientIdentitySlot);
        pushEvent(std::string("Connect target: ") + m_localDemoDiagnosticsReport.connectTargetName);
        pushEvent(std::string("Transport: ") + m_localDemoDiagnosticsReport.connectTransport);
        pushEvent(std::string("Lane mode: ") + m_localDemoDiagnosticsReport.connectLaneMode);
        pushEvent(std::string("Session account: ") + m_accountId);
        pushEvent(std::string("Session identity: ") + m_playerIdentity);
        pushEvent("Room and interior description runtime active for authored spaces.");
        pushEvent("Typed command routing active: help, look, say <text>, emote <text>, inv, mission, journal.");
        if (m_sessionMvpAuthoringLoaded)
        {
            pushEvent(std::string("Authoring pipeline loaded: ") + m_sessionMvpAuthoringSource);
        }
        else
        {
            pushEvent(std::string("Authoring pipeline fallback active: ") + m_sessionMvpAuthoringError);
        }
        pushEvent("Same-space social visibility active for say and emote.");
        pushEvent("Shared presence awareness active: nearby players surface when location and heartbeat interest rules both match.");
        pushEvent("Reconnect resume and continuity recovery active: room, mission, ship, orbital, and social context are persisted for resume flow.");
        if (m_resumeSessionId != "none")
        {
            pushEvent(std::string("Resume identity detected: ") + m_resumeSessionId);
        }
        if (!m_roomTitle.empty())
        {
            pushEvent(std::string("Restored location context: ") + m_roomTitle);
        }
        if (m_socialLocationKey != "none")
        {
            pushEvent(std::string("Restored social continuity for location key: ") + m_socialLocationKey);
        }
        if (m_reconnectContinuityRestored && !m_reconnectContinuitySummary.empty() && m_reconnectContinuitySummary != "none")
        {
            pushEvent(std::string("Restored reconnect continuity: ") + m_reconnectContinuitySummary);
        }
        pushEvent("Type 'help' and press Enter for shell commands and examples.");

        auto preferred = std::make_unique<BgfxRenderDevice>();
        if (preferred->initialize(m_window->getHandle()))
        {
            m_renderDevice = std::move(preferred);
            return;
        }

        auto fallback = std::make_unique<GdiRenderDevice>();
        fallback->initialize(m_window->getHandle());
        m_renderDevice = std::move(fallback);
    }

    void GameLayer::initializeSessionIdentityDefaults()
    {
        m_clientIdentitySlot = selectAvailableClientSlot(
            m_runtimeBoundaryReport,
            m_clientInstanceId,
            m_clientIdentityClaimPath);
        if (m_clientIdentitySlot.rfind("operator-", 0) == 0)
        {
            m_playerIdentity = m_clientIdentitySlot;
            m_accountId = std::string("internal-alpha-player-") + SessionEntryProtocol::sanitizeIdentifier(m_clientIdentitySlot);
            return;
        }

        m_playerIdentity = std::string("operator-") + m_clientIdentitySlot;
        m_accountId = std::string("internal-alpha-player-") + m_clientIdentitySlot;
    }

    void GameLayer::update(float dt)
    {
        m_lastDeltaTime = dt;
        m_camera.setViewportSize(m_window->getWidth(), m_window->getHeight());

        m_headlessHostPresenceReport = HeadlessHostPresence::buildReport(m_runtimeBoundaryReport);
        m_authoritativeHostProtocolReport = AuthoritativeHostProtocol::buildReport(m_runtimeBoundaryReport);
        updateSessionEntryFlow();
        refreshAuthorityMode();
        updateConnectionTelemetry();
        pollAuthoritativeHostResponses();
        updateReplicationDiagnostics();
        updateInput();
        m_simulationRuntime.advanceFrame(dt);
        updatePresentationRuntime();
        updateLocalSocialRuntime();
        updateLocalPresenceRuntime();
        updateReconnectRecoveryRuntime();

        m_hasActionTargetTile = m_simulationRuntime.hasMovementTarget();
        if (m_hasActionTargetTile)
        {
            m_actionTargetTile = m_simulationRuntime.movementTargetTile();
        }

        writeClientReplicationStatus();
    }

    void GameLayer::render()
    {
        if (!m_renderDevice)
        {
            return;
        }

        const RECT clientRect = getClientRect();
        if (!m_renderDevice->beginFrame(m_window->getHandle(), clientRect))
        {
            return;
        }

        const WorldState& worldState = m_simulationRuntime.worldState();
        const Vec2& playerPosition = m_simulationRuntime.presentedPlayerPosition();
        const std::vector<TileCoord>& currentPath = m_simulationRuntime.currentPath();
        const size_t pathIndex = m_simulationRuntime.pathIndex();
        const std::vector<std::string>& eventLog = m_simulationRuntime.eventLog();
        const SharedSimulationDiagnostics& simulationDiagnostics = m_simulationRuntime.diagnostics();

        HDC dc = m_renderDevice->getDrawContext();
        if (dc != nullptr)
        {
            m_worldRenderer.render(
                dc,
                clientRect,
                worldState,
                m_camera,
                playerPosition,
                m_visiblePresenceRenderProxies,
                currentPath,
                pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile);

            m_debugOverlayRenderer.render(
                dc,
                worldState,
                m_camera,
                playerPosition,
                currentPath,
                pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile,
                eventLog,
                m_lastDeltaTime,
                m_window->getMousePosition(),
                m_runtimeBoundaryReport,
                m_localDemoDiagnosticsReport,
                simulationDiagnostics,
                m_headlessHostPresenceReport,
                m_authoritativeHostProtocolReport,
                m_roomTitle,
                m_roomDescription,
                m_promptLine,
                m_environmentIdentityText,
                m_textPresentationProfile,
                m_artManifestPath,
                buildCommandBarText(),
                m_commandEcho,
                m_sameSpacePresenceCount,
                m_sameSpacePresenceSummary,
                LocalPresenceProtocol::interestRuleText(),
                m_reconnectRecoveryState,
                m_reconnectContinuitySummary,
                m_reconnectStaleSessionCount);
        }
        else
        {
            (void)m_bgfxWorldRenderer.render(
                worldState,
                m_camera,
                playerPosition,
                m_visiblePresenceRenderProxies,
                currentPath,
                pathIndex,
                m_hasHoveredTile,
                m_hoveredTile,
                m_hasSelectedTile,
                m_selectedTile,
                m_hasActionTargetTile,
                m_actionTargetTile,
                m_runtimeBoundaryReport,
                m_localDemoDiagnosticsReport,
                simulationDiagnostics,
                m_headlessHostPresenceReport,
                m_authoritativeHostProtocolReport);

            m_bgfxDebugFrameRenderer.render(
                worldState,
                playerPosition,
                eventLog,
                m_lastDeltaTime,
                m_bgfxWorldRenderer.statusMessage(),
                simulationDiagnostics,
                m_roomTitle,
                m_promptLine,
                buildCommandBarText(),
                m_environmentIdentityText,
                m_textPresentationProfile,
                m_sameSpacePresenceCount,
                m_sameSpacePresenceSummary,
                m_reconnectRecoveryState,
                m_reconnectContinuitySummary,
                m_reconnectStaleSessionCount);
        }

        m_renderDevice->endFrame(m_window->getHandle());
    }

    void GameLayer::shutdown()
    {
        persistLocationContext();
        persistSocialState();
        persistReconnectContinuity();
        if (!m_clientIdentityClaimPath.empty())
        {
            removeFileQuietly(m_clientIdentityClaimPath);
            m_clientIdentityClaimPath.clear();
        }
        m_bgfxWorldRenderer.shutdown();
        if (m_renderDevice)
        {
            m_renderDevice->shutdown();
        }
    }

    void GameLayer::updateInput()
    {
        applyAuthoringHotkeys();
        handleCommandBarInput();

        const WorldState& worldState = m_simulationRuntime.worldState();

        const POINT mouse = m_window->getMousePosition();
        const Vec2 mouseWorld = m_camera.screenToWorld(mouse.x, mouse.y);
        const TileCoord hovered = worldState.world().worldToTile(mouseWorld);

        m_hasHoveredTile = worldState.world().isInBounds(hovered);
        m_hoveredTile = hovered;

        POINT click{};
        if (m_window->consumeLeftClick(click))
        {
            const Vec2 world = m_camera.screenToWorld(click.x, click.y);
            const TileCoord targetTile = worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = worldState.world().isInBounds(targetTile);
            m_actionTargetTile = targetTile;
            m_hasActionTargetTile = m_hasSelectedTile;

            submitTypedIntent(SimulationIntentType::MoveToTile, targetTile, "Failed to queue move intent.");
        }

        POINT rightClick{};
        if (m_window->consumeRightClick(rightClick))
        {
            const Vec2 world = m_camera.screenToWorld(rightClick.x, rightClick.y);
            const TileCoord targetTile = worldState.world().worldToTile(world);

            m_selectedTile = targetTile;
            m_hasSelectedTile = worldState.world().isInBounds(targetTile);

            const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            submitTypedIntent(
                shiftDown ? SimulationIntentType::InspectTile : SimulationIntentType::InteractTile,
                targetTile,
                shiftDown ? "Failed to queue inspect intent." : "Failed to queue interact intent.");
        }

        const int wheel = m_window->consumeMouseWheelDelta();
        if (wheel != 0)
        {
            m_camera.zoomBy(wheel > 0 ? 0.10f : -0.10f);
        }

        if (m_window->isMiddleMouseDown())
        {
            const POINT delta = m_window->consumeMouseDelta();
            const float zoom = m_camera.getZoom();
            m_camera.pan({
                -static_cast<float>(delta.x) / zoom,
                -static_cast<float>(delta.y) / zoom
            });
        }
        else
        {
            const POINT ignoredDelta = m_window->consumeMouseDelta();
            (void)ignoredDelta;
        }
    }

    void GameLayer::pushEvent(const std::string& message)
    {
        m_simulationRuntime.appendEvent(message);
    }

    void GameLayer::applyAuthoringHotkeys()
    {
        const bool overlayDown = (GetAsyncKeyState('O') & 0x8000) != 0;
        const bool hotspotDown = (GetAsyncKeyState('H') & 0x8000) != 0;
        const bool palette7Down = (GetAsyncKeyState('7') & 0x8000) != 0;
        const bool palette8Down = (GetAsyncKeyState('8') & 0x8000) != 0;
        const bool palette9Down = (GetAsyncKeyState('9') & 0x8000) != 0;
        const bool latencyToggleDown = (GetAsyncKeyState('J') & 0x8000) != 0;
        const bool latencyPresetDown = (GetAsyncKeyState('K') & 0x8000) != 0;
        const bool jitterPresetDown = (GetAsyncKeyState('L') & 0x8000) != 0;

        WorldState& worldState = m_simulationRuntime.worldState();
        bool harnessConfigChanged = false;

        if (overlayDown && !m_overlayKeyWasDown)
        {
            const bool newState = !worldState.regionOverlayEnabled();
            worldState.setRegionOverlayEnabled(newState);
            pushEvent(newState ? "Region boundary overlay enabled" : "Region boundary overlay disabled");
        }

        if (hotspotDown && !m_hotspotKeyWasDown)
        {
            const bool newState = !worldState.authoringHotspotsVisible();
            worldState.setAuthoringHotspotsVisible(newState);
            pushEvent(newState ? "Authored hotspot overlay enabled" : "Authored hotspot overlay disabled");
        }

        if (palette7Down && !m_palette7WasDown)
        {
            worldState.setPaletteMode(BgfxThemePaletteMode::Default);
            pushEvent("Palette mode: Default");
        }

        if (palette8Down && !m_palette8WasDown)
        {
            worldState.setPaletteMode(BgfxThemePaletteMode::Muted);
            pushEvent("Palette mode: Muted");
        }

        if (palette9Down && !m_palette9WasDown)
        {
            worldState.setPaletteMode(BgfxThemePaletteMode::Vivid);
            pushEvent("Palette mode: Vivid");
        }

        if (latencyToggleDown && !m_latencyToggleWasDown)
        {
            m_replicationHarnessConfig.enabled = !m_replicationHarnessConfig.enabled;
            harnessConfigChanged = true;
            pushEvent(
                std::string("Replication harness ")
                + (m_replicationHarnessConfig.enabled ? "enabled" : "disabled")
                + " | "
                + harnessSummary(m_replicationHarnessConfig));
        }

        if (latencyPresetDown && !m_latencyPresetWasDown)
        {
            const uint32_t currentLatency = (std::max)(
                (std::max)(
                    m_replicationHarnessConfig.intentLatencyMilliseconds,
                    m_replicationHarnessConfig.acknowledgementLatencyMilliseconds),
                m_replicationHarnessConfig.snapshotLatencyMilliseconds);

            const uint32_t nextLatency = currentLatency == 0u
                ? 100u
                : (currentLatency <= 100u ? 350u : 0u);

            m_replicationHarnessConfig.intentLatencyMilliseconds = nextLatency;
            m_replicationHarnessConfig.acknowledgementLatencyMilliseconds = nextLatency;
            m_replicationHarnessConfig.snapshotLatencyMilliseconds = nextLatency;
            harnessConfigChanged = true;
            pushEvent(std::string("Replication latency preset: ") + std::to_string(nextLatency) + " ms");
        }

        if (jitterPresetDown && !m_jitterPresetWasDown)
        {
            const uint32_t currentJitter = m_replicationHarnessConfig.jitterMilliseconds;
            const uint32_t nextJitter = currentJitter == 0u
                ? 25u
                : (currentJitter <= 25u ? 100u : 0u);

            m_replicationHarnessConfig.jitterMilliseconds = nextJitter;
            harnessConfigChanged = true;
            pushEvent(std::string("Replication jitter preset: ") + std::to_string(nextJitter) + " ms");
        }

        if (harnessConfigChanged)
        {
            persistReplicationHarnessConfig();
            m_simulationRuntime.setReplicationHarnessState(
                m_replicationHarnessConfig.enabled,
                m_replicationHarnessConfig.intentLatencyMilliseconds,
                m_replicationHarnessConfig.acknowledgementLatencyMilliseconds,
                m_replicationHarnessConfig.snapshotLatencyMilliseconds,
                m_replicationHarnessConfig.jitterMilliseconds,
                m_lastSnapshotAgeMilliseconds);
        }

        m_overlayKeyWasDown = overlayDown;
        m_hotspotKeyWasDown = hotspotDown;
        m_palette7WasDown = palette7Down;
        m_palette8WasDown = palette8Down;
        m_palette9WasDown = palette9Down;
        m_latencyToggleWasDown = latencyToggleDown;
        m_latencyPresetWasDown = latencyPresetDown;
        m_jitterPresetWasDown = jitterPresetDown;
    }

    void GameLayer::refreshAuthorityMode()
    {
        std::string compatibilityReason;
        const bool hostCompatible = hostConnectionCompatible(compatibilityReason);
        const bool sessionGranted = m_sessionTicketIssued && m_sessionEntryState == "ticket-issued";
        const bool previousMode = m_useHeadlessHostAuthority;
        m_useHeadlessHostAuthority = hostCompatible && sessionGranted;

        m_simulationRuntime.setAuthorityMode(
            !m_useHeadlessHostAuthority,
            m_useHeadlessHostAuthority,
            m_useHeadlessHostAuthority);

        if (previousMode == m_useHeadlessHostAuthority)
        {
            return;
        }

        if (m_useHeadlessHostAuthority)
        {
            m_lastAppliedSnapshotPublishedEpochMilliseconds = 0;
            m_lastAppliedSnapshotSimulationTicks = 0;
            m_lastAppliedSnapshotSequence = 0;
            pushEvent(std::string("Authority mode: ticket accepted for session ") + m_grantedSessionId + ", hosted headless host authority active");
            return;
        }

        if (hostCompatible && !sessionGranted)
        {
            pushEvent("Authority mode: host online but session-entry ticket not yet granted");
            return;
        }

        pushEvent(std::string("Authority mode: local fallback active (") + compatibilityReason + ")");
    }

    void GameLayer::pollAuthoritativeHostResponses()
    {
        const std::vector<SimulationIntentAck> acknowledgements =
            AuthoritativeHostProtocol::collectAcknowledgementsForClient(
                m_runtimeBoundaryReport,
                m_clientInstanceId);
        for (const SimulationIntentAck& ack : acknowledgements)
        {
            m_simulationRuntime.applyAcknowledgement(ack);
        }

        if (!m_useHeadlessHostAuthority && !m_authoritativeHostProtocolReport.snapshotPresent)
        {
            m_lastSnapshotAgeMilliseconds = 0;
            if (!m_lastSnapshotReadError.empty())
            {
                m_lastSnapshotReadError.clear();
                m_simulationRuntime.clearSnapshotReadFailure();
            }
            return;
        }

        std::string snapshotError;
        const AuthoritativeWorldSnapshot snapshot =
            AuthoritativeHostProtocol::readAuthoritativeSnapshot(m_runtimeBoundaryReport, snapshotError);

        if (!snapshotError.empty())
        {
            m_lastSnapshotAgeMilliseconds = 0;
            if (snapshotError != m_lastSnapshotReadError)
            {
                m_simulationRuntime.recordSnapshotReadFailure(snapshotError);
                pushEvent(std::string("Snapshot read failed: ") + snapshotError);
                m_lastSnapshotReadError = snapshotError;
            }
            return;
        }

        if (!m_lastSnapshotReadError.empty())
        {
            pushEvent("Snapshot read recovered");
            m_lastSnapshotReadError.clear();
        }

        const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        if (snapshot.publishedEpochMilliseconds > 0 && nowEpochMilliseconds >= snapshot.publishedEpochMilliseconds)
        {
            m_lastSnapshotAgeMilliseconds = nowEpochMilliseconds - snapshot.publishedEpochMilliseconds;
        }
        else
        {
            m_lastSnapshotAgeMilliseconds = 0;
        }

        m_simulationRuntime.clearSnapshotReadFailure();

        if (!m_useHeadlessHostAuthority)
        {
            return;
        }

        if (sameSnapshotIdentity(
                snapshot,
                m_lastAppliedSnapshotPublishedEpochMilliseconds,
                m_lastAppliedSnapshotSimulationTicks,
                m_lastAppliedSnapshotSequence))
        {
            return;
        }

        std::string correctionReason;
        const bool corrected =
            m_simulationRuntime.applyAuthoritativeSnapshot(snapshot, m_lastSnapshotAgeMilliseconds, correctionReason);
        m_lastAppliedSnapshotPublishedEpochMilliseconds = snapshot.publishedEpochMilliseconds;
        m_lastAppliedSnapshotSimulationTicks = snapshot.simulationTicks;
        m_lastAppliedSnapshotSequence = snapshot.lastProcessedIntentSequence;
        if (corrected)
        {
            pushEvent(correctionReason);
        }
    }

    void GameLayer::updateSessionEntryFlow()
    {
        m_sessionEntryProtocolReport = SessionEntryProtocol::buildReport(m_runtimeBoundaryReport);

        if (!m_sessionEntryRequestWritten && m_sessionEntryProtocolReport.sessionEntryLaneReady)
        {
            submitSessionEntryRequest(m_reconnectRequested);
        }

        if (!m_sessionEntryRequestWritten)
        {
            m_sessionEntryState = m_sessionEntryProtocolReport.sessionEntryLaneReady
                ? "entry-request-pending-write"
                : "entry-lane-unavailable";
            return;
        }

        SessionTicket deniedTicket{};
        if (!m_sessionTicketIssued && tryResolveDeniedTicket(deniedTicket))
        {
            m_sessionTicketId = deniedTicket.ticketId;
            m_sessionTicketState = "denied";
            m_sessionDenialReason = deniedTicket.denialReason;
            m_sessionEntryState = "entry-denied";
            m_sessionTicketIssued = false;
            if (!m_sessionDenialLogged)
            {
                pushEvent(std::string("Session entry denied: ") + m_sessionDenialReason);
                m_commandEcho = std::string("Entry denied: ") + m_sessionDenialReason + ". Type 'entry' for a fresh request or 'resume' to retry resume identity.";
                m_sessionDenialLogged = true;
            }
            return;
        }

        SessionTicket issuedTicket{};
        if (tryResolveIssuedTicket(issuedTicket))
        {
            const bool firstIssue = !m_sessionTicketIssued || m_sessionTicketId != issuedTicket.ticketId;
            m_sessionTicketIssued = true;
            m_sessionTicketId = issuedTicket.ticketId;
            m_sessionTicketState = "issued";
            m_sessionDenialReason = "none";
            m_grantedSessionId = issuedTicket.grantedSessionId;
            m_resumeSessionId = issuedTicket.grantedSessionId;
            m_sessionTicketIssuedEpochMilliseconds = issuedTicket.issuedAtEpochMilliseconds;
            m_sessionTicketExpiresEpochMilliseconds = issuedTicket.expiresAtEpochMilliseconds;
            m_sessionEntryState = "ticket-issued";
            if (firstIssue)
            {
                persistResumeIdentity();
                pushEvent(std::string("Session ticket issued: ") + m_sessionTicketId);
                pushEvent(std::string("Granted session identity: ") + m_grantedSessionId);
                m_commandEcho = std::string("Entry granted for session ") + m_grantedSessionId + ".";
            }
            return;
        }

        if (m_sessionEntryState != "ticket-issued")
        {
            m_sessionEntryState = m_reconnectRequested ? "entry-pending-resume-ticket" : "entry-pending-ticket";
        }
    }

    void GameLayer::submitSessionEntryRequest(bool reconnectRequested)
    {
        if (!m_sessionEntryProtocolReport.sessionEntryLaneReady)
        {
            return;
        }

        SessionEntryRequest request{};
        request.requestId = std::string("request-") + m_clientInstanceId + "-" + std::to_string(ReplicationHarness::currentEpochMilliseconds());
        request.accountId = m_accountId;
        request.playerIdentity = m_playerIdentity;
        request.clientInstanceId = m_clientInstanceId;
        request.buildIdentity = m_localDemoDiagnosticsReport.buildIdentity;
        request.environmentName = m_localDemoDiagnosticsReport.environmentName;
        request.connectTargetName = m_localDemoDiagnosticsReport.connectTargetName;
        request.requestedResumeSessionId = reconnectRequested ? m_resumeSessionId : std::string("none");
        request.requestedAtEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        request.requestedTicketTtlSeconds = 120u;
        request.reconnectRequested = reconnectRequested;

        std::string error;
        if (!SessionEntryProtocol::writeEntryRequest(m_runtimeBoundaryReport, request, error))
        {
            pushEvent(std::string("Failed to write session entry request: ") + error);
            m_sessionEntryState = "entry-request-write-failed";
            m_sessionDenialReason = error;
            return;
        }

        m_sessionEntryRequestWritten = true;
        m_sessionTicketIssued = false;
        m_reconnectRequested = reconnectRequested;
        m_sessionDenialLogged = false;
        m_sessionRequestId = request.requestId;
        m_sessionTicketId = "none";
        m_sessionTicketState = "none";
        m_sessionDenialReason = "none";
        m_grantedSessionId = "none";
        m_sessionEntryState = reconnectRequested ? "entry-requested-resume" : "entry-requested";
        pushEvent(std::string("Session entry request submitted: ") + m_sessionRequestId);
        if (reconnectRequested)
        {
            pushEvent(std::string("Reconnect requested with resume session ") + m_resumeSessionId);
        }
    }

    void GameLayer::loadPersistedResumeIdentity()
    {
        m_resumeSessionId = "none";
        std::unordered_map<std::string, std::string> values{};
        const std::filesystem::path scopedPath = clientResumeIdentityPath(m_runtimeBoundaryReport, m_clientIdentitySlot);
        const bool loadedScoped = parseSimpleKeyValueFile(scopedPath, values);
        if (!loadedScoped)
        {
            if (m_clientIdentitySlot != "alpha"
                || !parseSimpleKeyValueFile(legacyClientResumeIdentityPath(m_runtimeBoundaryReport), values))
            {
                return;
            }
        }

        const auto sessionIt = values.find("granted_session_id");
        if (sessionIt != values.end() && !sessionIt->second.empty())
        {
            m_resumeSessionId = sessionIt->second;
        }

        if (loadedScoped)
        {
            const auto accountIt = values.find("account_id");
            if (accountIt != values.end() && !accountIt->second.empty())
            {
                m_accountId = accountIt->second;
            }

            const auto playerIt = values.find("player_identity");
            if (playerIt != values.end() && !playerIt->second.empty())
            {
                m_playerIdentity = playerIt->second;
            }
        }
    }

    void GameLayer::persistResumeIdentity() const
    {
        if (m_grantedSessionId == "none")
        {
            return;
        }

        std::ostringstream output;
        output
            << "granted_session_id=" << sanitizeSingleLine(m_grantedSessionId) << "\n"
            << "account_id=" << sanitizeSingleLine(m_accountId) << "\n"
            << "player_identity=" << sanitizeSingleLine(m_playerIdentity) << "\n"
            << "ticket_id=" << sanitizeSingleLine(m_sessionTicketId) << "\n";
        const std::string contents = output.str();
        writeTextFileAtomically(clientResumeIdentityPath(m_runtimeBoundaryReport, m_clientIdentitySlot), contents);
        writeTextFileAtomically(legacyClientResumeIdentityPath(m_runtimeBoundaryReport), contents);
    }

    void GameLayer::loadPersistedLocationContext()
    {
        std::unordered_map<std::string, std::string> values{};
        const std::filesystem::path scopedPath = clientLocationContextPath(m_runtimeBoundaryReport, m_clientIdentitySlot);
        if (!parseSimpleKeyValueFile(scopedPath, values))
        {
            if (m_clientIdentitySlot != "alpha"
                || !parseSimpleKeyValueFile(legacyClientLocationContextPath(m_runtimeBoundaryReport), values))
            {
                return;
            }
        }

        const auto keyIt = values.find("location_key");
        if (keyIt == values.end() || keyIt->second.empty())
        {
            return;
        }

        m_roomSignature = keyIt->second;

        const auto titleIt = values.find("location_title");
        if (titleIt != values.end() && !titleIt->second.empty())
        {
            m_roomTitle = titleIt->second;
        }

        const auto descriptionIt = values.find("location_description");
        if (descriptionIt != values.end() && !descriptionIt->second.empty())
        {
            m_roomDescription = descriptionIt->second;
        }
    }

    void GameLayer::persistLocationContext() const
    {
        if (m_roomSignature.empty() || m_roomTitle.empty() || m_roomDescription.empty())
        {
            return;
        }

        std::ostringstream output;
        output
            << "location_key=" << sanitizeSingleLine(m_roomSignature) << "\n"
            << "location_title=" << sanitizeSingleLine(m_roomTitle) << "\n"
            << "location_description=" << sanitizeSingleLine(m_roomDescription) << "\n";
        writeTextFileAtomically(clientLocationContextPath(m_runtimeBoundaryReport, m_clientIdentitySlot), output.str());
    }

    void GameLayer::loadPersistedSocialState()
    {
        m_lastSeenSocialEpochMilliseconds = 0;
        m_socialLocationKey = "none";

        std::unordered_map<std::string, std::string> values{};
        if (!parseSimpleKeyValueFile(LocalSocialProtocol::statePath(m_runtimeBoundaryReport, m_clientIdentitySlot), values))
        {
            return;
        }

        const auto lastSeenIt = values.find("social_last_seen_epoch_milliseconds");
        if (lastSeenIt != values.end() && !lastSeenIt->second.empty())
        {
            try
            {
                m_lastSeenSocialEpochMilliseconds = static_cast<uint64_t>(std::stoull(lastSeenIt->second));
            }
            catch (...)
            {
                m_lastSeenSocialEpochMilliseconds = 0;
            }
        }

        const auto locationIt = values.find("social_location_key");
        if (locationIt != values.end() && !locationIt->second.empty())
        {
            m_socialLocationKey = locationIt->second;
        }

        const auto participantIt = values.find("social_participant_id");
        if (participantIt != values.end() && !participantIt->second.empty())
        {
            m_socialParticipantId = participantIt->second;
        }
    }

    void GameLayer::persistSocialState() const
    {
        std::ostringstream output;
        output
            << "social_last_seen_epoch_milliseconds=" << m_lastSeenSocialEpochMilliseconds << "\n"
            << "social_location_key=" << sanitizeSingleLine(m_socialLocationKey) << "\n"
            << "social_participant_id=" << sanitizeSingleLine(m_socialParticipantId.empty() ? buildSocialParticipantId() : m_socialParticipantId) << "\n";
        writeTextFileAtomically(LocalSocialProtocol::statePath(m_runtimeBoundaryReport, m_clientIdentitySlot), output.str());
    }


    void GameLayer::loadPersistedReconnectContinuity()
    {
        m_reconnectContinuitySnapshot = {};
        m_reconnectContinuityRestored = false;
        m_reconnectStaleSessionCount = 0;
        m_reconnectRecoveryState = "cold-start";
        m_reconnectRecoveryReason = "none";
        m_reconnectContinuitySummary = "none";

        std::string error;
        if (!ReconnectSessionProtocol::readContinuitySnapshot(
                m_runtimeBoundaryReport,
                m_reconnectContinuitySnapshot,
                error,
                m_clientIdentitySlot))
        {
            if (!error.empty())
            {
                m_reconnectRecoveryState = "continuity-read-failed";
                m_reconnectRecoveryReason = error;
            }
            return;
        }

        m_reconnectContinuityRestored = true;
        if (m_roomTitle.empty() && !m_reconnectContinuitySnapshot.roomTitle.empty())
        {
            m_roomTitle = m_reconnectContinuitySnapshot.roomTitle;
        }
        if (m_roomDescription.empty() && !m_reconnectContinuitySnapshot.roomDescription.empty())
        {
            m_roomDescription = m_reconnectContinuitySnapshot.roomDescription;
        }
        if (m_roomSignature.empty() && !m_reconnectContinuitySnapshot.locationKey.empty())
        {
            m_roomSignature = m_reconnectContinuitySnapshot.locationKey;
        }
        if (m_socialLocationKey == "none" && !m_reconnectContinuitySnapshot.socialLocationKey.empty())
        {
            m_socialLocationKey = m_reconnectContinuitySnapshot.socialLocationKey;
        }
        m_reconnectRecoveryState = "continuity-restored";
        m_reconnectRecoveryReason = "persisted-session-context";
        m_reconnectContinuitySummary = buildReconnectContinuitySummary();
    }

    void GameLayer::persistReconnectContinuity()
    {
        SessionContinuitySnapshot snapshot{};
        snapshot.savedEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        snapshot.grantedSessionId = m_grantedSessionId;
        snapshot.resumeSessionId = m_resumeSessionId;
        snapshot.clientInstanceId = m_clientInstanceId;
        snapshot.locationKey = m_roomSignature;
        snapshot.roomTitle = m_roomTitle;
        snapshot.roomDescription = m_roomDescription;
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        snapshot.missionPhaseText = diagnostics.missionPhaseText;
        snapshot.missionObjectiveText = diagnostics.missionObjectiveText;
        snapshot.missionLastBeat = diagnostics.missionLastBeat;
        snapshot.shipLocationText = diagnostics.shipLocationText;
        snapshot.shipLastBeat = diagnostics.shipLastBeat;
        snapshot.orbitalNodeText = diagnostics.orbitalCurrentNodeText;
        snapshot.orbitalPhaseText = diagnostics.orbitalPhaseText;
        snapshot.socialLocationKey = m_socialLocationKey;
        snapshot.sameSpacePresenceCount = m_sameSpacePresenceCount;
        snapshot.sameSpacePresenceSummary = m_sameSpacePresenceSummary;

        m_reconnectContinuitySnapshot = snapshot;
        std::string error;
        ReconnectSessionProtocol::writeContinuitySnapshot(m_runtimeBoundaryReport, snapshot, error, m_clientIdentitySlot);
    }

    std::string GameLayer::buildReconnectContinuitySummary() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();

        const std::string roomTitle = !m_roomTitle.empty()
            ? m_roomTitle
            : m_reconnectContinuitySnapshot.roomTitle;
        const std::string missionPhase = !diagnostics.missionPhaseText.empty()
            ? diagnostics.missionPhaseText
            : m_reconnectContinuitySnapshot.missionPhaseText;
        const std::string shipLocation = !diagnostics.shipLocationText.empty()
            ? diagnostics.shipLocationText
            : m_reconnectContinuitySnapshot.shipLocationText;
        const std::string orbitalNode = !diagnostics.orbitalCurrentNodeText.empty()
            ? diagnostics.orbitalCurrentNodeText
            : m_reconnectContinuitySnapshot.orbitalNodeText;

        std::vector<std::string> parts{};
        if (!roomTitle.empty())
        {
            parts.push_back(std::string("room=") + roomTitle);
        }
        if (!missionPhase.empty())
        {
            parts.push_back(std::string("mission=") + missionPhase);
        }
        if (!shipLocation.empty())
        {
            parts.push_back(std::string("ship=") + shipLocation);
        }
        if (!orbitalNode.empty())
        {
            parts.push_back(std::string("orbit=") + orbitalNode);
        }
        if (parts.empty())
        {
            return "none";
        }
        return joinSummary(parts, " | ");
    }


    void GameLayer::loadSessionMvpAuthoring()
    {
        m_sessionMvpAuthoringLoaded = false;
        m_sessionMvpAuthoringError = "authoring-file-not-found";
        m_sessionMvpAuthoringSource = "none";
        m_sessionMvpAuthoring = SessionMvpAuthoringBundle{};

        const std::filesystem::path authoringPath = resolveSessionMvpAuthoringPath(m_runtimeBoundaryReport);
        if (authoringPath.empty())
        {
            return;
        }

        m_sessionMvpAuthoring = SessionMvpAuthoringPipeline::loadFromFile(authoringPath);
        m_sessionMvpAuthoringSource = authoringPath.generic_string();
        if (m_sessionMvpAuthoring.valid)
        {
            m_sessionMvpAuthoringLoaded = true;
            m_sessionMvpAuthoringError = "none";
            return;
        }

        m_sessionMvpAuthoringError = m_sessionMvpAuthoring.validationErrors.empty()
            ? std::string("authoring-validation-failed")
            : m_sessionMvpAuthoring.validationErrors.front();
    }

    void GameLayer::updateReconnectRecoveryRuntime()
    {
        persistReconnectContinuity();
        m_reconnectContinuitySummary = buildReconnectContinuitySummary();

        const std::string continuitySessionId = m_grantedSessionId != "none"
            ? m_grantedSessionId
            : m_resumeSessionId;
        const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();

        if (!continuitySessionId.empty() && continuitySessionId != "none")
        {
            const bool publishLease = m_lastReconnectLeaseEpochMilliseconds == 0
                || nowEpochMilliseconds >= m_lastReconnectLeaseEpochMilliseconds + 1000ull
                || m_reconnectRequested;

            if (publishLease)
            {
                SessionLeaseRecord lease{};
                lease.heartbeatEpochMilliseconds = nowEpochMilliseconds;
                lease.grantedSessionId = continuitySessionId;
                lease.clientInstanceId = m_clientInstanceId;
                lease.participantId = buildSocialParticipantId();
                lease.locationKey = m_roomSignature;
                lease.reconnectState = m_reconnectRequested ? std::string("resume-requested") : std::string("active");

                std::string publishError;
                if (!ReconnectSessionProtocol::appendLeaseRecord(m_runtimeBoundaryReport, lease, publishError))
                {
                    if (publishError != m_lastReconnectPollError)
                    {
                        pushEvent(std::string("Reconnect lease write failed: ") + publishError);
                        m_lastReconnectPollError = publishError;
                    }
                }
                else
                {
                    m_lastReconnectLeaseEpochMilliseconds = lease.heartbeatEpochMilliseconds;
                }
            }
        }

        std::string error;
        const std::vector<SessionLeaseRecord> records = ReconnectSessionProtocol::readLeaseRecords(m_runtimeBoundaryReport, error);
        if (!error.empty())
        {
            if (error != m_lastReconnectPollError)
            {
                pushEvent(std::string("Reconnect lease read failed: ") + error);
                m_lastReconnectPollError = error;
            }
            return;
        }

        if (!m_lastReconnectPollError.empty())
        {
            pushEvent("Reconnect lease read recovered");
            m_lastReconnectPollError.clear();
        }

        const uint64_t staleCutoff = nowEpochMilliseconds > ReconnectSessionProtocol::staleLeaseThresholdMilliseconds()
            ? nowEpochMilliseconds - ReconnectSessionProtocol::staleLeaseThresholdMilliseconds()
            : 0ull;

        uint32_t staleCount = 0;
        bool conflictingFreshLease = false;
        for (const SessionLeaseRecord& record : records)
        {
            if (continuitySessionId.empty() || continuitySessionId == "none")
            {
                continue;
            }
            if (record.grantedSessionId != continuitySessionId || record.clientInstanceId == m_clientInstanceId)
            {
                continue;
            }
            if (record.heartbeatEpochMilliseconds < staleCutoff)
            {
                ++staleCount;
            }
            else
            {
                conflictingFreshLease = true;
            }
        }
        m_reconnectStaleSessionCount = staleCount;

        std::string newState = m_reconnectContinuityRestored ? std::string("continuity-restored") : std::string("active");
        std::string newReason = m_reconnectContinuityRestored ? std::string("persisted-session-context") : std::string("live-session");

        if (m_reconnectRequested)
        {
            if (conflictingFreshLease)
            {
                newState = "duplicate-session-detected";
                newReason = "fresh-lease-conflict";
            }
            else if (staleCount > 0)
            {
                newState = "resume-recovered";
                newReason = "stale-session-lease-evicted";
            }
            else
            {
                newState = "resume-requested";
                newReason = "waiting-for-resume-resolution";
            }
        }
        else if (m_sessionEntryState == "ticket-issued" && m_resumeSessionId != "none" && m_resumeSessionId == m_grantedSessionId)
        {
            newState = "resume-stable";
            newReason = "session-ticket-restored";
        }

        if (newState != m_reconnectRecoveryState || newReason != m_reconnectRecoveryReason)
        {
            m_reconnectRecoveryState = newState;
            m_reconnectRecoveryReason = newReason;
            pushEvent(std::string("Reconnect | state=") + m_reconnectRecoveryState + " | reason=" + m_reconnectRecoveryReason);
        }
    }

    std::string GameLayer::buildSocialParticipantId() const
    {
        return m_grantedSessionId != "none"
            ? m_grantedSessionId
            : m_clientSessionId;
    }

    void GameLayer::updateLocalSocialRuntime()
    {
        m_socialParticipantId = buildSocialParticipantId();
        m_socialLocationKey = m_roomSignature.empty() ? std::string("none") : m_roomSignature;

        std::string error;
        const std::vector<LocalSocialMessage> messages = LocalSocialProtocol::readMessages(m_runtimeBoundaryReport, error);
        if (!error.empty())
        {
            if (error != m_lastSocialPollError)
            {
                pushEvent(std::string("Social lane read failed: ") + error);
                m_lastSocialPollError = error;
            }
            return;
        }

        if (!m_lastSocialPollError.empty())
        {
            pushEvent("Social lane read recovered");
            m_lastSocialPollError.clear();
        }

        uint64_t newestSeen = m_lastSeenSocialEpochMilliseconds;
        for (const LocalSocialMessage& message : messages)
        {
            if (message.sentEpochMilliseconds > newestSeen)
            {
                newestSeen = message.sentEpochMilliseconds;
            }

            if (message.sentEpochMilliseconds <= m_lastSeenSocialEpochMilliseconds)
            {
                continue;
            }

            if (m_socialLocationKey == "none" || message.locationKey != m_socialLocationKey)
            {
                continue;
            }

            if (message.senderSessionId == m_socialParticipantId)
            {
                continue;
            }

            const std::string senderIdentity = message.senderIdentity.empty()
                ? std::string("unknown-operator")
                : message.senderIdentity;

            if (message.kind == LocalSocialMessageKind::Say)
            {
                pushEvent(std::string("Social | ") + senderIdentity + " says, \"" + message.text + "\"");
            }
            else
            {
                pushEvent(std::string("Social | ") + senderIdentity + " " + message.text);
            }
        }

        if (newestSeen != m_lastSeenSocialEpochMilliseconds)
        {
            m_lastSeenSocialEpochMilliseconds = newestSeen;
            persistSocialState();
        }
    }

    void GameLayer::updateLocalPresenceRuntime()
    {
        m_socialParticipantId = buildSocialParticipantId();

        const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const WorldState& worldState = m_simulationRuntime.worldState();
        const std::string currentLocationKey = m_roomSignature.empty() ? std::string("none") : m_roomSignature;
        const Vec2 localWorldPosition = m_simulationRuntime.presentedPlayerPosition();
        const TileCoord playerTile = worldState.world().worldToTile(localWorldPosition);
        const float tileSize = static_cast<float>(worldState.world().getTileSize());
        const bool movedSinceLastHeartbeat = m_hasLastPresenceHeartbeatWorldPosition
            && length(localWorldPosition - m_lastPresenceHeartbeatWorldPosition) >= tileSize * 0.08f;
        const uint64_t publishIntervalMilliseconds =
            (m_simulationRuntime.hasMovementTarget() || movedSinceLastHeartbeat) ? 150ull : 1000ull;

        const bool publishHeartbeat = m_lastPresenceHeartbeatEpochMilliseconds == 0
            || currentLocationKey != m_presenceLocationKey
            || nowEpochMilliseconds >= m_lastPresenceHeartbeatEpochMilliseconds + publishIntervalMilliseconds;

        if (publishHeartbeat)
        {
            LocalPresenceHeartbeat heartbeat{};
            heartbeat.heartbeatEpochMilliseconds = nowEpochMilliseconds;
            heartbeat.participantId = buildSocialParticipantId();
            heartbeat.sessionId = m_grantedSessionId != "none" ? m_grantedSessionId : m_clientSessionId;
            heartbeat.playerIdentity = m_playerIdentity.empty() ? std::string("unknown-operator") : m_playerIdentity;
            heartbeat.locationKey = currentLocationKey;
            heartbeat.roomTitle = m_roomTitle;
            heartbeat.missionPhaseText = diagnostics.missionPhaseText;
            heartbeat.missionObjectiveText = diagnostics.missionObjectiveText;
            heartbeat.tileX = playerTile.x;
            heartbeat.tileY = playerTile.y;
            heartbeat.worldX = localWorldPosition.x;
            heartbeat.worldY = localWorldPosition.y;

            std::string publishError;
            if (!LocalPresenceProtocol::appendHeartbeat(m_runtimeBoundaryReport, heartbeat, publishError))
            {
                if (publishError != m_lastPresencePollError)
                {
                    pushEvent(std::string("Presence lane write failed: ") + publishError);
                    m_lastPresencePollError = publishError;
                }
            }
            else
            {
                m_lastPresenceHeartbeatEpochMilliseconds = heartbeat.heartbeatEpochMilliseconds;
                m_lastPresenceHeartbeatWorldPosition = localWorldPosition;
                m_hasLastPresenceHeartbeatWorldPosition = true;
                m_presenceLocationKey = heartbeat.locationKey;
            }
        }

        std::string error;
        const std::vector<LocalPresenceHeartbeat> heartbeats = LocalPresenceProtocol::readHeartbeats(m_runtimeBoundaryReport, error);
        if (!error.empty())
        {
            if (error != m_lastPresencePollError)
            {
                pushEvent(std::string("Presence lane read failed: ") + error);
                m_lastPresencePollError = error;
            }
            return;
        }

        if (!m_lastPresencePollError.empty())
        {
            pushEvent("Presence lane read recovered");
            m_lastPresencePollError.clear();
        }

        std::unordered_map<std::string, LocalPresenceHeartbeat> latestByParticipant{};
        uint64_t newestRosterEpochMilliseconds = 0;
        for (const LocalPresenceHeartbeat& heartbeat : heartbeats)
        {
            if (heartbeat.participantId.empty())
            {
                continue;
            }

            newestRosterEpochMilliseconds = (std::max)(newestRosterEpochMilliseconds, heartbeat.heartbeatEpochMilliseconds);

            auto existing = latestByParticipant.find(heartbeat.participantId);
            if (existing == latestByParticipant.end()
                || heartbeat.heartbeatEpochMilliseconds >= existing->second.heartbeatEpochMilliseconds)
            {
                latestByParticipant[heartbeat.participantId] = heartbeat;
            }
        }

        const uint64_t freshnessCutoffEpochMilliseconds = nowEpochMilliseconds > 15000ull
            ? nowEpochMilliseconds - 15000ull
            : 0ull;

        std::vector<std::string> visibleParticipantIds{};
        std::vector<std::string> visibleParticipantSummaries{};
        std::unordered_map<std::string, std::string> visibleNames{};
        std::unordered_set<std::string> visibleParticipantSet{};
        std::vector<RemotePresenceRenderProxy> visiblePresenceRenderProxies{};

        for (const auto& entry : latestByParticipant)
        {
            const LocalPresenceHeartbeat& heartbeat = entry.second;
            if (heartbeat.participantId == m_socialParticipantId)
            {
                continue;
            }

            if (currentLocationKey == "none" || heartbeat.locationKey != currentLocationKey)
            {
                continue;
            }

            if (heartbeat.heartbeatEpochMilliseconds < freshnessCutoffEpochMilliseconds)
            {
                continue;
            }

            const std::string playerIdentity = heartbeat.playerIdentity.empty()
                ? heartbeat.participantId
                : heartbeat.playerIdentity;

            visibleParticipantIds.push_back(heartbeat.participantId);
            visibleParticipantSummaries.push_back(playerIdentity + " | " + heartbeat.missionPhaseText);
            visibleNames[heartbeat.participantId] = playerIdentity;
            visibleParticipantSet.insert(heartbeat.participantId);

            const TileCoord heartbeatTile{ heartbeat.tileX, heartbeat.tileY };
            if (worldState.world().isInBounds(heartbeatTile))
            {
                const bool hasPreciseWorldPosition =
                    std::fabs(heartbeat.worldX) > 0.001f || std::fabs(heartbeat.worldY) > 0.001f;
                const Vec2 center = worldState.world().tileToWorldCenter(heartbeatTile);
                const Vec2 sampleWorldPosition = hasPreciseWorldPosition
                    ? Vec2{ heartbeat.worldX, heartbeat.worldY }
                    : Vec2{ center.x + remotePresenceOffset(heartbeat.participantId, worldState.world().getTileSize()).x,
                            center.y + remotePresenceOffset(heartbeat.participantId, worldState.world().getTileSize()).y };

                RemotePresenceTrack& track = m_remotePresenceTracks[heartbeat.participantId];
                if (track.latestEpochMilliseconds == 0)
                {
                    track.previousEpochMilliseconds = heartbeat.heartbeatEpochMilliseconds;
                    track.latestEpochMilliseconds = heartbeat.heartbeatEpochMilliseconds;
                    track.previousWorldPosition = sampleWorldPosition;
                    track.latestWorldPosition = sampleWorldPosition;
                    track.presentedWorldPosition = sampleWorldPosition;
                }
                else if (heartbeat.heartbeatEpochMilliseconds > track.latestEpochMilliseconds)
                {
                    track.previousEpochMilliseconds = track.latestEpochMilliseconds;
                    track.previousWorldPosition = track.latestWorldPosition;
                    track.latestEpochMilliseconds = heartbeat.heartbeatEpochMilliseconds;
                    track.latestWorldPosition = sampleWorldPosition;
                    if (length(track.presentedWorldPosition - sampleWorldPosition) > tileSize * 1.5f)
                    {
                        track.presentedWorldPosition = sampleWorldPosition;
                    }
                }

                Vec2 predictedWorldPosition = track.latestWorldPosition;
                if (track.latestEpochMilliseconds > track.previousEpochMilliseconds)
                {
                    const float sampleDeltaSeconds = static_cast<float>(
                        track.latestEpochMilliseconds - track.previousEpochMilliseconds) / 1000.0f;
                    if (sampleDeltaSeconds > 0.001f)
                    {
                        const Vec2 velocity =
                            (track.latestWorldPosition - track.previousWorldPosition) * (1.0f / sampleDeltaSeconds);
                        const uint64_t ageMilliseconds =
                            nowEpochMilliseconds > track.latestEpochMilliseconds
                            ? (nowEpochMilliseconds - track.latestEpochMilliseconds)
                            : 0ull;
                        const float predictionSeconds =
                            (std::min)(0.18f, static_cast<float>(ageMilliseconds) / 1000.0f);
                        predictedWorldPosition = track.latestWorldPosition + (velocity * predictionSeconds);
                    }
                }

                const float smoothingAlpha = 1.0f - std::exp(-12.0f * (std::max)(0.0f, m_lastDeltaTime));
                track.presentedWorldPosition = lerpVec2(
                    track.presentedWorldPosition,
                    predictedWorldPosition,
                    smoothingAlpha);
                visiblePresenceRenderProxies.push_back(RemotePresenceRenderProxy{
                    track.presentedWorldPosition
                });
            }
        }

        std::sort(visibleParticipantIds.begin(), visibleParticipantIds.end());
        std::sort(visibleParticipantSummaries.begin(), visibleParticipantSummaries.end());

        for (const std::string& participantId : visibleParticipantIds)
        {
            if (std::find(m_visiblePresenceParticipantIds.begin(), m_visiblePresenceParticipantIds.end(), participantId)
                == m_visiblePresenceParticipantIds.end())
            {
                const auto nameIt = visibleNames.find(participantId);
                const std::string identity = nameIt != visibleNames.end() ? nameIt->second : participantId;
                pushEvent(std::string("Presence | ") + identity + " is now nearby in " + m_roomTitle + ".");
            }
        }

        for (const std::string& participantId : m_visiblePresenceParticipantIds)
        {
            if (std::find(visibleParticipantIds.begin(), visibleParticipantIds.end(), participantId) == visibleParticipantIds.end())
            {
                pushEvent(std::string("Presence | ") + participantId + " is no longer in same-space awareness.");
            }
        }

        for (auto it = m_remotePresenceTracks.begin(); it != m_remotePresenceTracks.end();)
        {
            if (visibleParticipantSet.find(it->first) == visibleParticipantSet.end())
            {
                it = m_remotePresenceTracks.erase(it);
            }
            else
            {
                ++it;
            }
        }

        m_visiblePresenceParticipantIds = visibleParticipantIds;
        m_visiblePresenceRenderProxies = visiblePresenceRenderProxies;
        m_sameSpacePresenceCount = static_cast<uint32_t>(visibleParticipantIds.size());
        m_sameSpacePresenceSummary = joinSummary(visibleParticipantSummaries, "; ");
        m_lastPresenceRosterEpochMilliseconds = newestRosterEpochMilliseconds;
        m_presenceLocationKey = currentLocationKey;
    }

    void GameLayer::routeLocalSocialCommand(LocalSocialMessageKind kind, const std::string& text)
    {
        if (m_roomSignature.empty())
        {
            m_commandEcho = "Social routing unavailable until location resolves.";
            return;
        }

        LocalSocialMessage message{};
        message.sentEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        message.senderSessionId = buildSocialParticipantId();
        message.senderIdentity = m_playerIdentity.empty()
            ? std::string("unknown-operator")
            : m_playerIdentity;
        message.locationKey = m_roomSignature;
        message.kind = kind;
        message.text = text;

        std::string error;
        if (!LocalSocialProtocol::appendMessage(m_runtimeBoundaryReport, message, error))
        {
            m_commandEcho = std::string("Social routing failed: ") + error;
            pushEvent(m_commandEcho);
            return;
        }

        m_socialParticipantId = message.senderSessionId;
        m_socialLocationKey = message.locationKey;
        m_lastSeenSocialEpochMilliseconds = (std::max)(m_lastSeenSocialEpochMilliseconds, message.sentEpochMilliseconds);
        persistSocialState();

        if (kind == LocalSocialMessageKind::Say)
        {
            pushEvent(std::string("Social | You say, \"") + text + "\"");
            m_commandEcho = "Local speech routed to same-space players.";
            return;
        }

        pushEvent(std::string("Social | You ") + text);
        m_commandEcho = "Local emote routed to same-space players.";
    }

    bool GameLayer::tryResolveIssuedTicket(SessionTicket& outTicket) const
    {
        const std::vector<SessionTicket> tickets = SessionEntryProtocol::collectIssuedTickets(m_runtimeBoundaryReport);
        for (auto it = tickets.rbegin(); it != tickets.rend(); ++it)
        {
            if (it->requestId == SessionEntryProtocol::sanitizeIdentifier(m_sessionRequestId)
                && it->clientInstanceId == SessionEntryProtocol::sanitizeIdentifier(m_clientInstanceId))
            {
                outTicket = *it;
                return true;
            }
        }

        return false;
    }

    bool GameLayer::tryResolveDeniedTicket(SessionTicket& outTicket) const
    {
        const std::vector<SessionTicket> tickets = SessionEntryProtocol::collectDeniedTickets(m_runtimeBoundaryReport);
        for (auto it = tickets.rbegin(); it != tickets.rend(); ++it)
        {
            if (it->requestId == SessionEntryProtocol::sanitizeIdentifier(m_sessionRequestId)
                && it->clientInstanceId == SessionEntryProtocol::sanitizeIdentifier(m_clientInstanceId))
            {
                outTicket = *it;
                return true;
            }
        }

        return false;
    }

    void GameLayer::updateConnectionTelemetry()
    {
        if (!m_connectAttemptLogged)
        {
            const std::string attemptMessage = std::string("Connect attempt: target=")
                + m_localDemoDiagnosticsReport.connectTargetName
                + " | transport="
                + m_localDemoDiagnosticsReport.connectTransport
                + " | lane="
                + m_localDemoDiagnosticsReport.connectLaneMode
                + " | runtime_root="
                + m_localDemoDiagnosticsReport.runtimeRootDisplay;
            pushEvent(attemptMessage);
            appendClientTrace(m_runtimeBoundaryReport, attemptMessage);
            m_lastConnectEvent = "attempt";
            m_connectAttemptLogged = true;
        }

        std::string compatibilityReason;
        const bool hostCompatible = hostConnectionCompatible(compatibilityReason);
        const bool sessionGranted = m_sessionTicketIssued && m_sessionEntryState == "ticket-issued";

        if (hostCompatible && sessionGranted)
        {
            const bool sessionChanged = !m_connectionEstablished
                || m_lastHostSessionId != m_headlessHostPresenceReport.sessionId
                || m_lastConnectedHostInstanceId != m_headlessHostPresenceReport.hostInstanceId;

            m_connectState = "connected-headless-host";
            m_connectFailureReason = "none";
            m_connectionEstablished = true;
            m_connectFailureLogged = false;

            if (sessionChanged)
            {
                const std::string connectedMessage = std::string("Connect success: target=")
                    + m_headlessHostPresenceReport.connectTargetName
                    + " | transport="
                    + m_headlessHostPresenceReport.transportKind
                    + " | host_session="
                    + m_headlessHostPresenceReport.sessionId
                    + " | granted_session="
                    + m_grantedSessionId
                    + " | host_instance="
                    + m_headlessHostPresenceReport.hostInstanceId
                    + " | protocol=v"
                    + std::to_string(m_headlessHostPresenceReport.protocolVersion)
                    + " | build="
                    + m_headlessHostPresenceReport.buildIdentity;
                pushEvent(connectedMessage);
                appendClientTrace(m_runtimeBoundaryReport, connectedMessage);
                m_lastConnectEvent = "connected";
                m_lastHostSessionId = m_headlessHostPresenceReport.sessionId;
                m_lastConnectedHostInstanceId = m_headlessHostPresenceReport.hostInstanceId;
            }
            return;
        }

        if (hostCompatible && !sessionGranted)
        {
            m_connectState = m_reconnectRequested ? "entry-pending-resume-ticket" : "entry-pending-ticket";
            m_connectFailureReason = "none";
            m_connectionEstablished = false;
            return;
        }

        const uint64_t nowEpochMilliseconds = ReplicationHarness::currentEpochMilliseconds();
        const uint64_t elapsedMilliseconds = nowEpochMilliseconds >= m_clientStartedEpochMilliseconds
            ? nowEpochMilliseconds - m_clientStartedEpochMilliseconds
            : 0;

        if (m_connectionEstablished)
        {
            const std::string disconnectMessage = std::string("Disconnect event: reason=")
                + compatibilityReason
                + " | target="
                + m_localDemoDiagnosticsReport.connectTargetName
                + " | falling back to local authority";
            pushEvent(disconnectMessage);
            appendClientTrace(m_runtimeBoundaryReport, disconnectMessage);
            m_connectionEstablished = false;
            m_connectState = "fallback-local";
            m_connectFailureReason = compatibilityReason;
            m_lastDisconnectReason = compatibilityReason;
            m_lastConnectEvent = "disconnected";
            return;
        }

        if (m_sessionEntryState == "entry-denied")
        {
            m_connectState = "entry-denied";
            m_connectFailureReason = m_sessionDenialReason;
            m_lastConnectEvent = "denied";
            return;
        }

        if (elapsedMilliseconds < kConnectFailureTimeoutMilliseconds)
        {
            m_connectState = "connect-pending";
            m_connectFailureReason = "none";
            return;
        }

        m_connectState = "fallback-local";
        m_connectFailureReason = compatibilityReason.empty() ? std::string("host-offline") : compatibilityReason;
        if (!m_connectFailureLogged)
        {
            std::ostringstream failureMessage;
            failureMessage
                << "Connect failure: target=" << m_localDemoDiagnosticsReport.connectTargetName
                << " unavailable (" << m_connectFailureReason << ")"
                << " | transport=" << m_localDemoDiagnosticsReport.connectTransport
                << " | lane=" << m_localDemoDiagnosticsReport.connectLaneMode
                << " | runtime_root=" << m_localDemoDiagnosticsReport.runtimeRootDisplay;

            pushEvent(failureMessage.str());
            appendClientTrace(m_runtimeBoundaryReport, failureMessage.str());
            m_lastConnectEvent = "failed";
            m_connectFailureLogged = true;
        }
    }

    bool GameLayer::hostConnectionCompatible(std::string& outReason) const
    {
        outReason.clear();

        if (!m_headlessHostPresenceReport.statusFilePresent)
        {
            outReason = "host-status-missing";
            return false;
        }

        if (!m_headlessHostPresenceReport.statusParseValid)
        {
            outReason = "host-status-invalid";
            return false;
        }

        if (!m_headlessHostPresenceReport.heartbeatFresh || !m_headlessHostPresenceReport.hostOnline)
        {
            outReason = "host-offline";
            return false;
        }

        if (m_headlessHostPresenceReport.protocolVersion != m_expectedProtocolVersion)
        {
            outReason = "protocol-version-mismatch";
            return false;
        }

        if (m_headlessHostPresenceReport.transportKind != m_localDemoDiagnosticsReport.connectTransport)
        {
            outReason = "transport-mismatch";
            return false;
        }

        if (m_headlessHostPresenceReport.connectTargetName != m_localDemoDiagnosticsReport.connectTargetName)
        {
            outReason = "connect-target-mismatch";
            return false;
        }

        return true;
    }

    void GameLayer::persistReplicationHarnessConfig()
    {
        std::string error;
        if (!ReplicationHarness::saveConfig(m_runtimeBoundaryReport, m_replicationHarnessConfig, error))
        {
            pushEvent(std::string("Failed to persist replication harness config: ") + error);
        }
    }

    void GameLayer::updateReplicationDiagnostics()
    {
        const ReplicationHarnessConfig loadedConfig = ReplicationHarness::loadConfig(m_runtimeBoundaryReport);
        if (!configsEqual(loadedConfig, m_replicationHarnessConfig))
        {
            m_replicationHarnessConfig = loadedConfig;
            pushEvent(std::string("Replication harness config updated externally: ") + harnessSummary(m_replicationHarnessConfig));
        }

        m_simulationRuntime.setReplicationHarnessState(
            m_replicationHarnessConfig.enabled,
            m_replicationHarnessConfig.intentLatencyMilliseconds,
            m_replicationHarnessConfig.acknowledgementLatencyMilliseconds,
            m_replicationHarnessConfig.snapshotLatencyMilliseconds,
            m_replicationHarnessConfig.jitterMilliseconds,
            m_lastSnapshotAgeMilliseconds);
    }

    void GameLayer::writeClientReplicationStatus() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        std::ostringstream output;
        output
            << "version=10\n"
            << "build_identity=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.buildIdentity) << "\n"
            << "build_channel=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.buildChannel) << "\n"
            << "client_instance_id=" << sanitizeSingleLine(m_clientInstanceId) << "\n"
            << "client_session_id=" << sanitizeSingleLine(m_clientSessionId) << "\n"
            << "client_identity_slot=" << sanitizeSingleLine(m_clientIdentitySlot) << "\n"
            << "connect_target_name=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.connectTargetName) << "\n"
            << "connect_transport=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.connectTransport) << "\n"
            << "connect_lane_mode=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.connectLaneMode) << "\n"
            << "runtime_root=" << sanitizeSingleLine(m_localDemoDiagnosticsReport.runtimeRootDisplay) << "\n"
            << "connect_state=" << sanitizeSingleLine(m_connectState) << "\n"
            << "connect_failure_reason=" << sanitizeSingleLine(m_connectFailureReason) << "\n"
            << "last_connect_event=" << sanitizeSingleLine(m_lastConnectEvent) << "\n"
            << "last_disconnect_reason=" << sanitizeSingleLine(m_lastDisconnectReason) << "\n"
            << "session_entry_state=" << sanitizeSingleLine(m_sessionEntryState) << "\n"
            << "session_request_id=" << sanitizeSingleLine(m_sessionRequestId) << "\n"
            << "session_ticket_id=" << sanitizeSingleLine(m_sessionTicketId) << "\n"
            << "session_ticket_state=" << sanitizeSingleLine(m_sessionTicketState) << "\n"
            << "session_denial_reason=" << sanitizeSingleLine(m_sessionDenialReason) << "\n"
            << "granted_session_id=" << sanitizeSingleLine(m_grantedSessionId) << "\n"
            << "resume_session_id=" << sanitizeSingleLine(m_resumeSessionId) << "\n"
            << "reconnect_requested=" << (m_reconnectRequested ? "yes" : "no") << "\n"
            << "account_id=" << sanitizeSingleLine(m_accountId) << "\n"
            << "player_identity=" << sanitizeSingleLine(m_playerIdentity) << "\n"
            << "expected_protocol_version=" << m_expectedProtocolVersion << "\n"
            << "authority_mode=" << (m_useHeadlessHostAuthority ? "headless-host" : "local") << "\n"
            << "host_online=" << (m_headlessHostPresenceReport.hostOnline ? "yes" : "no") << "\n"
            << "host_protocol_version=" << m_headlessHostPresenceReport.protocolVersion << "\n"
            << "host_transport_kind=" << sanitizeSingleLine(m_headlessHostPresenceReport.transportKind) << "\n"
            << "host_connect_target_name=" << sanitizeSingleLine(m_headlessHostPresenceReport.connectTargetName) << "\n"
            << "host_connect_lane_mode=" << sanitizeSingleLine(m_headlessHostPresenceReport.connectLaneMode) << "\n"
            << "host_build_identity=" << sanitizeSingleLine(m_headlessHostPresenceReport.buildIdentity) << "\n"
            << "host_instance_id=" << sanitizeSingleLine(m_headlessHostPresenceReport.hostInstanceId) << "\n"
            << "host_session_id=" << sanitizeSingleLine(m_headlessHostPresenceReport.sessionId) << "\n"
            << "host_restore_state=" << sanitizeSingleLine(m_headlessHostPresenceReport.restoreState) << "\n"
            << "host_persistence_last_load_succeeded=" << (m_headlessHostPresenceReport.persistenceLastLoadSucceeded ? "yes" : "no") << "\n"
            << "protocol_lane_ready=" << (m_authoritativeHostProtocolReport.authorityLaneReady ? "yes" : "no") << "\n"
            << "snapshot_present=" << (m_authoritativeHostProtocolReport.snapshotPresent ? "yes" : "no") << "\n"
            << "room_context_key=" << sanitizeSingleLine(m_roomSignature) << "\n"
            << "room_title=" << sanitizeSingleLine(m_roomTitle) << "\n"
            << "room_description=" << sanitizeSingleLine(m_roomDescription) << "\n"
            << "mission_phase=" << sanitizeSingleLine(diagnostics.missionPhaseText) << "\n"
            << "mission_objective=" << sanitizeSingleLine(m_missionObjectiveText) << "\n"
            << "mission_journal=" << sanitizeSingleLine(m_missionJournalText) << "\n"
            << "mission_last_beat=" << sanitizeSingleLine(diagnostics.missionLastBeat) << "\n"
            << "prompt_line=" << sanitizeSingleLine(m_promptLine) << "\n"
            << "environment_identity=" << sanitizeSingleLine(m_environmentIdentityText) << "\n"
            << "text_presentation_profile=" << sanitizeSingleLine(m_textPresentationProfile) << "\n"
            << "art_manifest_path=" << sanitizeSingleLine(m_artManifestPath) << "\n"
            << "authoring_schema_version=" << m_sessionMvpAuthoring.schemaVersion << "\n"
            << "authoring_content_loaded=" << (m_sessionMvpAuthoringLoaded ? "yes" : "no") << "\n"
            << "authoring_source_path=" << sanitizeSingleLine(m_sessionMvpAuthoringSource) << "\n"
            << "authoring_validation_error=" << sanitizeSingleLine(m_sessionMvpAuthoringError) << "\n"
            << "command_bar=" << sanitizeSingleLine(buildCommandBarText()) << "\n"
            << "command_discoverability=" << sanitizeSingleLine(buildCommandHelpText()) << "\n"
            << "command_echo=" << sanitizeSingleLine(m_commandEcho) << "\n"
            << "social_participant_id=" << sanitizeSingleLine(m_socialParticipantId.empty() ? buildSocialParticipantId() : m_socialParticipantId) << "\n"
            << "social_location_key=" << sanitizeSingleLine(m_socialLocationKey) << "\n"
            << "social_last_seen_epoch_milliseconds=" << m_lastSeenSocialEpochMilliseconds << "\n"
            << "social_visibility_rule=same-location-key\n"
            << "presence_participant_id=" << sanitizeSingleLine(m_socialParticipantId.empty() ? buildSocialParticipantId() : m_socialParticipantId) << "\n"
            << "presence_location_key=" << sanitizeSingleLine(m_presenceLocationKey) << "\n"
            << "presence_same_space_count=" << m_sameSpacePresenceCount << "\n"
            << "presence_same_space_summary=" << sanitizeSingleLine(m_sameSpacePresenceSummary) << "\n"
            << "presence_last_roster_epoch_milliseconds=" << m_lastPresenceRosterEpochMilliseconds << "\n"
            << "presence_interest_rule=" << LocalPresenceProtocol::interestRuleText() << "\n"
            << "continuity_snapshot_epoch_milliseconds=" << m_reconnectContinuitySnapshot.savedEpochMilliseconds << "\n"
            << "continuity_restored=" << (m_reconnectContinuityRestored ? "yes" : "no") << "\n"
            << "continuity_summary=" << sanitizeSingleLine(m_reconnectContinuitySummary) << "\n"
            << "continuity_room_title=" << sanitizeSingleLine(m_reconnectContinuitySnapshot.roomTitle.empty() ? m_roomTitle : m_reconnectContinuitySnapshot.roomTitle) << "\n"
            << "continuity_mission_phase=" << sanitizeSingleLine(m_reconnectContinuitySnapshot.missionPhaseText.empty() ? diagnostics.missionPhaseText : m_reconnectContinuitySnapshot.missionPhaseText) << "\n"
            << "continuity_ship_location=" << sanitizeSingleLine(m_reconnectContinuitySnapshot.shipLocationText.empty() ? diagnostics.shipLocationText : m_reconnectContinuitySnapshot.shipLocationText) << "\n"
            << "continuity_orbital_node=" << sanitizeSingleLine(m_reconnectContinuitySnapshot.orbitalNodeText.empty() ? diagnostics.orbitalCurrentNodeText : m_reconnectContinuitySnapshot.orbitalNodeText) << "\n"
            << "reconnect_last_lease_epoch_milliseconds=" << m_lastReconnectLeaseEpochMilliseconds << "\n"
            << "reconnect_recovery_state=" << sanitizeSingleLine(m_reconnectRecoveryState) << "\n"
            << "reconnect_recovery_reason=" << sanitizeSingleLine(m_reconnectRecoveryReason) << "\n"
            << "reconnect_stale_session_count=" << m_reconnectStaleSessionCount << "\n"
            << "reconnect_recovery_rule=" << ReconnectSessionProtocol::recoveryRuleText() << "\n"
            << "client_prediction_enabled=" << (diagnostics.clientPredictionEnabled ? "yes" : "no") << "\n";

        const std::filesystem::path statusPath = m_runtimeBoundaryReport.logsDirectory / "client_replication_status.txt";
        writeTextFileAtomically(statusPath, output.str());
    }

    void GameLayer::updatePresentationRuntime()
    {
        const std::string newSignature = buildRoomSignature();
        const std::string newTitle = buildRoomTitle();
        const std::string newDescription = buildRoomDescription();

        const bool signatureChanged = !newSignature.empty() && newSignature != m_roomSignature;
        const bool presentationChanged = newTitle != m_roomTitle || newDescription != m_roomDescription;

        const std::string newMissionObjective = buildMissionObjectiveText();
        const std::string newMissionJournal = buildMissionJournalText();
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const std::string missionPhaseKey = diagnostics.missionPhaseText.empty()
            ? std::string("none")
            : diagnostics.missionPhaseText;
        const bool missionPhaseChanged = missionPhaseKey != m_lastMissionJournalPhaseKey;

        m_roomTitle = newTitle;
        m_roomDescription = newDescription;
        m_missionObjectiveText = newMissionObjective;
        m_missionJournalText = newMissionJournal;
        m_promptLine = buildPromptLine();
        m_environmentIdentityText = buildEnvironmentIdentityText(newSignature);
        m_textPresentationProfile = buildTextPresentationProfile(newSignature);

        if (!newSignature.empty() && (signatureChanged || presentationChanged || m_roomSignature.empty()))
        {
            m_roomSignature = newSignature;
            m_socialLocationKey = m_roomSignature;
            persistLocationContext();
            persistSocialState();
        }

        if (signatureChanged)
        {
            pushEvent(std::string("Room entry: ") + m_roomTitle);
            pushEvent(m_roomDescription);
        }

        if (missionPhaseChanged)
        {
            m_lastMissionJournalPhaseKey = missionPhaseKey;
            pushEvent(std::string("Objective: ") + m_missionObjectiveText);
            pushEvent(std::string("Journal: ") + m_missionJournalText);
        }
    }

    void GameLayer::handleCommandBarInput()
    {
        const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        auto appendOemCharacter = [this, shiftDown](int virtualKey, char normalCharacter, char shiftedCharacter)
        {
            if (consumeKeyEdge(virtualKey))
            {
                appendTypedCharacter(shiftDown ? shiftedCharacter : normalCharacter);
            }
        };

        if (consumeKeyEdge(VK_BACK))
        {
            if (!m_commandInput.empty())
            {
                m_commandInput.pop_back();
            }
        }

        if (consumeKeyEdge(VK_SPACE))
        {
            appendTypedCharacter(' ');
        }

        for (int key = 'A'; key <= 'Z'; ++key)
        {
            if (consumeKeyEdge(key))
            {
                appendTypedCharacter(static_cast<char>(std::tolower(key)));
            }
        }

        for (int key = '0'; key <= '9'; ++key)
        {
            if (consumeKeyEdge(key))
            {
                appendTypedCharacter(static_cast<char>(key));
            }
        }

        appendOemCharacter(VK_OEM_PERIOD, '.', '>');
        appendOemCharacter(VK_OEM_COMMA, ',', '<');
        appendOemCharacter(VK_OEM_MINUS, '-', '_');
        appendOemCharacter(VK_OEM_PLUS, '=', '+');
        appendOemCharacter(VK_OEM_1, ';', ':');
        appendOemCharacter(VK_OEM_2, '/', '?');
        appendOemCharacter(VK_OEM_4, '[', '{');
        appendOemCharacter(VK_OEM_6, ']', '}');
        appendOemCharacter(VK_OEM_7, '\'', '"');

        if (consumeKeyEdge(VK_RETURN))
        {
            const std::string submitted = m_commandInput;
            m_commandInput.clear();
            executeCommandLine(submitted);
        }
    }

    void GameLayer::executeCommandLine(const std::string& commandLine)
    {
        const RoutedCommand command = parseRoutedCommand(commandLine);
        if (command.type == RoutedCommandType::None)
        {
            m_commandEcho = "No command entered.";
            return;
        }

        switch (command.type)
        {
        case RoutedCommandType::Help:
            m_commandEcho = buildCommandHelpText();
            pushEvent("Help opened for typed command shell.");
            return;

        case RoutedCommandType::Look:
            m_commandEcho = std::string("Displayed location text for ") + m_roomTitle + ".";
            pushEvent(std::string("Location: ") + m_roomTitle);
            pushEvent(m_roomDescription);
            return;

        case RoutedCommandType::Say:
            if (!command.syntaxValid)
            {
                m_commandEcho = "Say syntax: say <text>";
                return;
            }
            routeLocalSocialCommand(LocalSocialMessageKind::Say, command.argumentText);
            return;

        case RoutedCommandType::Emote:
            if (!command.syntaxValid)
            {
                m_commandEcho = "Emote syntax: emote <text>";
                return;
            }
            routeLocalSocialCommand(LocalSocialMessageKind::Emote, command.argumentText);
            return;

        case RoutedCommandType::Inventory:
            m_commandEcho = buildInventoryShellText(m_simulationRuntime.diagnostics());
            pushEvent(m_commandEcho);
            return;

        case RoutedCommandType::Status:
            m_commandEcho = "Displayed vitals prompt.";
            pushEvent(std::string("Vitals: ") + m_promptLine);
            pushEvent(std::string("Objective: ") + m_missionObjectiveText);
            return;

        case RoutedCommandType::Mission:
            m_commandEcho = "Displayed mission objective from the authored content pipeline.";
            pushEvent(std::string("Objective: ") + m_missionObjectiveText);
            return;

        case RoutedCommandType::Journal:
            m_commandEcho = "Displayed mission journal entry from the authored content pipeline.";
            pushEvent(std::string("Journal: ") + m_missionJournalText);
            return;

        case RoutedCommandType::Style:
            m_commandEcho = std::string("Environment: ") + m_environmentIdentityText + " | Profile: " + m_textPresentationProfile + ".";
            pushEvent(std::string("Presentation: ") + m_environmentIdentityText + " | " + m_textPresentationProfile);
            return;

        case RoutedCommandType::Session:
            m_commandEcho = std::string("Entry state: ") + m_sessionEntryState + ", granted session: " + m_grantedSessionId + ", resume identity: " + m_resumeSessionId + ", reconnect state: " + m_reconnectRecoveryState + ".";
            pushEvent(std::string("Session entry: ") + m_sessionEntryState + " | granted=" + m_grantedSessionId + " | resume=" + m_resumeSessionId);
            return;

        case RoutedCommandType::Entry:
            m_sessionEntryRequestWritten = false;
            m_sessionTicketIssued = false;
            m_reconnectRequested = false;
            m_reconnectRecoveryState = "entry-requested";
            m_reconnectRecoveryReason = "fresh-session-request";
            submitSessionEntryRequest(false);
            m_commandEcho = "Fresh session entry requested.";
            return;

        case RoutedCommandType::Resume:
            if (m_resumeSessionId == "none")
            {
                m_commandEcho = "No persisted resume session is available yet.";
                return;
            }

            m_sessionEntryRequestWritten = false;
            m_sessionTicketIssued = false;
            m_reconnectRequested = true;
            m_reconnectRecoveryState = "resume-requested";
            m_reconnectRecoveryReason = "operator-resume-command";
            submitSessionEntryRequest(true);
            m_commandEcho = std::string("Reconnect requested for resume session ") + m_resumeSessionId + ". Continuity summary: " + buildReconnectContinuitySummary();
            return;

        case RoutedCommandType::Inspect:
        {
            const TileCoord target = m_hasSelectedTile
                ? m_selectedTile
                : m_simulationRuntime.worldState().world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
            submitTypedIntent(SimulationIntentType::InspectTile, target, "Failed to queue inspect intent.");
            m_commandEcho = "Inspect queued.";
            return;
        }

        case RoutedCommandType::Interact:
        {
            const TileCoord target = m_hasSelectedTile
                ? m_selectedTile
                : m_simulationRuntime.worldState().world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
            submitTypedIntent(SimulationIntentType::InteractTile, target, "Failed to queue interact intent.");
            m_commandEcho = "Interact queued.";
            return;
        }

        case RoutedCommandType::Move:
            if (!command.syntaxValid)
            {
                m_commandEcho = "Move syntax: move <x> <y>";
                return;
            }
            m_selectedTile = { command.moveX, command.moveY };
            m_hasSelectedTile = true;
            m_actionTargetTile = m_selectedTile;
            m_hasActionTargetTile = true;
            submitTypedIntent(SimulationIntentType::MoveToTile, m_selectedTile, "Failed to queue move intent.");
            m_commandEcho = "Move queued.";
            return;

        case RoutedCommandType::Clear:
            m_commandEcho = "Command reply cleared.";
            return;

        case RoutedCommandType::Unknown:
            m_commandEcho = std::string("Unknown command: ") + command.verb + ". Type 'help'.";
            return;

        case RoutedCommandType::None:
        default:
            m_commandEcho = "No command entered.";
            return;
        }
    }

    void GameLayer::submitTypedIntent(SimulationIntentType type, TileCoord target, const std::string& queueFailureMessage)
    {
        const uint64_t queuedSequence = m_simulationRuntime.enqueueIntent(type, target);
        if (queuedSequence == 0)
        {
            pushEvent(queueFailureMessage);
            return;
        }

        if (!m_useHeadlessHostAuthority)
        {
            return;
        }

        SimulationIntent intent{};
        intent.sequence = queuedSequence;
        intent.type = type;
        intent.target = target;
        intent.clientInstanceId = m_clientInstanceId;
        intent.sessionId = m_grantedSessionId != "none" ? m_grantedSessionId : m_clientSessionId;

        if (type == SimulationIntentType::MoveToTile)
        {
            return;
        }

        std::string protocolError;
        if (AuthoritativeHostProtocol::writeIntentRequest(m_runtimeBoundaryReport, intent, protocolError))
        {
            return;
        }

        SimulationIntentAck rejection{};
        rejection.sequence = queuedSequence;
        rejection.type = type;
        rejection.target = target;
        rejection.clientInstanceId = intent.clientInstanceId;
        rejection.sessionId = intent.sessionId;
        rejection.result = SimulationIntentAckResult::Rejected;
        rejection.reason = std::string("host queue write failed: ") + protocolError;
        m_simulationRuntime.applyAcknowledgement(rejection);
        pushEvent(std::string("Failed to submit intent to headless host: ") + protocolError);
    }

    bool GameLayer::consumeKeyEdge(int virtualKey)
    {
        if (virtualKey < 0 || virtualKey >= static_cast<int>(m_keyWasDown.size()))
        {
            return false;
        }

        const bool down = (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
        const bool pressed = down && !m_keyWasDown[static_cast<size_t>(virtualKey)];
        m_keyWasDown[static_cast<size_t>(virtualKey)] = down;
        return pressed;
    }

    void GameLayer::appendTypedCharacter(char character)
    {
        if (m_commandInput.size() < 96u)
        {
            m_commandInput.push_back(character);
        }
    }

    std::string GameLayer::buildRoomSignature() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const WorldState& worldState = m_simulationRuntime.worldState();
        const TileCoord playerTile = worldState.world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
        const ResolvedLocationContext context = resolveRuntimeLocationContext(worldState, diagnostics, playerTile);
        return context.valid ? context.key : std::string();
    }

    std::string GameLayer::buildRoomTitle() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const WorldState& worldState = m_simulationRuntime.worldState();
        const TileCoord playerTile = worldState.world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
        const ResolvedLocationContext context = resolveRuntimeLocationContext(worldState, diagnostics, playerTile);

        if (m_sessionMvpAuthoringLoaded && context.valid)
        {
            if (const SessionMvpRoomAuthoring* room = m_sessionMvpAuthoring.findRoom(context.key))
            {
                if (!room->title.empty())
                {
                    return room->title;
                }
            }
        }

        if (context.valid && !context.title.empty())
        {
            return context.title;
        }

        return m_roomTitle.empty() ? std::string("Unknown Interior") : m_roomTitle;
    }

    std::string GameLayer::buildRoomDescription() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const WorldState& worldState = m_simulationRuntime.worldState();
        const TileCoord playerTile = worldState.world().worldToTile(m_simulationRuntime.authoritativePlayerPosition());
        const ResolvedLocationContext context = resolveRuntimeLocationContext(worldState, diagnostics, playerTile);

        std::ostringstream description;
        if (m_sessionMvpAuthoringLoaded && context.valid)
        {
            if (const SessionMvpRoomAuthoring* room = m_sessionMvpAuthoring.findRoom(context.key))
            {
                if (!room->description.empty())
                {
                    description << room->description;
                    return description.str();
                }
            }
        }

        if (context.valid && !context.entryDescription.empty())
        {
            description << context.entryDescription;
        }
        else if (!m_roomDescription.empty())
        {
            description << m_roomDescription;
        }
        else
        {
            description << "The current interior has not been formally authored yet.";
        }

        return description.str();
    }

    std::string GameLayer::buildMissionObjectiveText() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const std::string phaseKey = diagnostics.missionPhaseText.empty()
            ? std::string("none")
            : diagnostics.missionPhaseText;

        if (m_sessionMvpAuthoringLoaded)
        {
            if (const SessionMvpMissionAuthoring* mission = m_sessionMvpAuthoring.findMission(phaseKey))
            {
                if (!mission->objective.empty())
                {
                    return mission->objective;
                }
            }
        }

        return diagnostics.missionObjectiveText.empty()
            ? std::string("No active mission objective.")
            : diagnostics.missionObjectiveText;
    }

    std::string GameLayer::buildMissionJournalText() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        const std::string phaseKey = diagnostics.missionPhaseText.empty()
            ? std::string("none")
            : diagnostics.missionPhaseText;

        if (m_sessionMvpAuthoringLoaded)
        {
            if (const SessionMvpMissionAuthoring* mission = m_sessionMvpAuthoring.findMission(phaseKey))
            {
                if (!mission->journal.empty())
                {
                    return mission->journal;
                }
            }
        }

        return buildFallbackMissionJournalText(diagnostics);
    }

    std::string GameLayer::buildPromptLine() const
    {
        const SharedSimulationDiagnostics& diagnostics = m_simulationRuntime.diagnostics();
        std::ostringstream prompt;
        prompt
            << "HP " << diagnostics.playerHealth << "/" << diagnostics.playerMaxHealth
            << " | ARM " << diagnostics.playerArmor
            << " | O2 " << static_cast<int>(diagnostics.oxygenSecondsRemaining) << "s"
            << " | RAD " << diagnostics.radiationDose
            << " | TOX " << diagnostics.toxicExposure
            << " | NEAR " << m_sameSpacePresenceCount
            << " | ENTRY " << m_sessionEntryState;
        return prompt.str();
    }

    std::string GameLayer::buildCommandBarText() const
    {
        return std::string("> ") + m_commandInput + '_';
    }

    std::string GameLayer::buildEnvironmentIdentityText(const std::string& roomSignature) const
    {
        if (roomSignature.rfind("region.khepri-cargo-bay", 0) == 0) return "Khepri Industrial Dock";
        if (roomSignature.rfind("region.transit-spine", 0) == 0) return "Transit Utility Conduit";
        if (roomSignature.rfind("region.medlab-diagnostics", 0) == 0) return "Clinical Triage White";
        if (roomSignature.rfind("region.command-deck-approach", 0) == 0) return "Command Security Red";
        if (roomSignature.rfind("region.hazard-containment", 0) == 0) return "Containment Hazard Amber";
        if (roomSignature.rfind("ship.", 0) == 0) return "Responder Shuttle Interior";
        if (roomSignature.rfind("orbital.", 0) == 0) return "Orbital Survey Blue";
        if (roomSignature.rfind("frontier.", 0) == 0) return "Dust Frontier Surface";
        return "Session MVP Interior";
    }

    std::string GameLayer::buildTextPresentationProfile(const std::string& roomSignature) const
    {
        if (roomSignature.rfind("region.khepri-cargo-bay", 0) == 0) return "industrial-manifest-shell";
        if (roomSignature.rfind("region.transit-spine", 0) == 0) return "utility-conduit-shell";
        if (roomSignature.rfind("region.medlab-diagnostics", 0) == 0) return "clinical-diagnostic-shell";
        if (roomSignature.rfind("region.command-deck-approach", 0) == 0) return "command-security-shell";
        if (roomSignature.rfind("region.hazard-containment", 0) == 0) return "hazard-warning-shell";
        if (roomSignature.rfind("ship.", 0) == 0) return "shuttle-cabin-shell";
        if (roomSignature.rfind("orbital.", 0) == 0) return "orbital-survey-shell";
        if (roomSignature.rfind("frontier.", 0) == 0) return "frontier-signal-shell";
        return "session-mvp-shell";
    }

    RECT GameLayer::getClientRect() const
    {
        RECT rect{};
        GetClientRect(m_window->getHandle(), &rect);
        return rect;
    }
}
