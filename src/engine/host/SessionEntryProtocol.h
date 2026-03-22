#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <windows.h>

#include "engine/core/RuntimePaths.h"

namespace war
{
    enum class SessionTicketState
    {
        None = 0,
        Issued,
        Accepted,
        Denied,
        Expired
    };

    struct SessionEntryRequest
    {
        std::string requestId = "unassigned";
        std::string accountId = "anonymous";
        std::string playerIdentity = "unassigned";
        std::string clientInstanceId = "unknown";
        std::string buildIdentity = "unknown";
        std::string environmentName = "local";
        std::string connectTargetName = "localhost-fallback";
        std::string requestedResumeSessionId = "none";
        uint64_t requestedAtEpochMilliseconds = 0;
        uint32_t requestedTicketTtlSeconds = 120;
        bool reconnectRequested = false;
    };

    struct SessionTicket
    {
        std::string ticketId = "unassigned";
        std::string requestId = "unassigned";
        std::string accountId = "anonymous";
        std::string playerIdentity = "unassigned";
        std::string clientInstanceId = "unknown";
        std::string buildIdentity = "unknown";
        std::string environmentName = "local";
        std::string connectTargetName = "localhost-fallback";
        std::string grantedSessionId = "none";
        std::string denialReason = "none";
        std::string resumeSessionId = "none";
        uint64_t issuedAtEpochMilliseconds = 0;
        uint64_t expiresAtEpochMilliseconds = 0;
        uint64_t consumedAtEpochMilliseconds = 0;
        SessionTicketState state = SessionTicketState::None;
    };

    struct ActiveSessionRecord
    {
        std::string sessionId = "none";
        std::string ticketId = "none";
        std::string requestId = "none";
        std::string accountId = "anonymous";
        std::string playerIdentity = "unassigned";
        std::string clientInstanceId = "unknown";
        std::string environmentName = "local";
        std::string connectTargetName = "localhost-fallback";
        std::string state = "active";
        uint64_t issuedAtEpochMilliseconds = 0;
        uint64_t lastValidatedAtEpochMilliseconds = 0;
    };

    struct SessionEntryValidationResult
    {
        bool accepted = false;
        bool denied = false;
        bool expired = false;
        bool reconnectRequested = false;
        std::string denialReason = "none";
    };

    struct SessionEntryProtocolReport
    {
        bool requestQueueReady = false;
        bool issuedTicketDirectoryReady = false;
        bool deniedTicketDirectoryReady = false;
        bool activeSessionDirectoryReady = false;
        bool sessionEntryLaneReady = false;

        std::filesystem::path sessionEntryRootDirectory;
        std::filesystem::path requestQueueDirectory;
        std::filesystem::path issuedTicketDirectory;
        std::filesystem::path deniedTicketDirectory;
        std::filesystem::path activeSessionDirectory;

        std::vector<std::string> issues;
    };

    class SessionEntryProtocol
    {
    public:
        [[nodiscard]] static SessionEntryProtocolReport buildReport(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            SessionEntryProtocolReport report{};
            report.sessionEntryRootDirectory = sessionEntryRootDirectory(runtimeBoundaryReport);
            report.requestQueueDirectory = requestQueueDirectory(runtimeBoundaryReport);
            report.issuedTicketDirectory = issuedTicketDirectory(runtimeBoundaryReport);
            report.deniedTicketDirectory = deniedTicketDirectory(runtimeBoundaryReport);
            report.activeSessionDirectory = activeSessionDirectory(runtimeBoundaryReport);

            report.requestQueueReady = directoryExists(report.requestQueueDirectory);
            report.issuedTicketDirectoryReady = directoryExists(report.issuedTicketDirectory);
            report.deniedTicketDirectoryReady = directoryExists(report.deniedTicketDirectory);
            report.activeSessionDirectoryReady = directoryExists(report.activeSessionDirectory);
            report.sessionEntryLaneReady =
                report.requestQueueReady
                && report.issuedTicketDirectoryReady
                && report.deniedTicketDirectoryReady
                && report.activeSessionDirectoryReady;
            return report;
        }

        static void ensureDirectories(SessionEntryProtocolReport& report)
        {
            ensureDirectory(report.sessionEntryRootDirectory, report.issues);
            ensureDirectory(report.requestQueueDirectory, report.issues);
            ensureDirectory(report.issuedTicketDirectory, report.issues);
            ensureDirectory(report.deniedTicketDirectory, report.issues);
            ensureDirectory(report.activeSessionDirectory, report.issues);

            report.requestQueueReady = directoryExists(report.requestQueueDirectory);
            report.issuedTicketDirectoryReady = directoryExists(report.issuedTicketDirectory);
            report.deniedTicketDirectoryReady = directoryExists(report.deniedTicketDirectory);
            report.activeSessionDirectoryReady = directoryExists(report.activeSessionDirectory);
            report.sessionEntryLaneReady =
                report.requestQueueReady
                && report.issuedTicketDirectoryReady
                && report.deniedTicketDirectoryReady
                && report.activeSessionDirectoryReady;
        }

        [[nodiscard]] static bool writeEntryRequest(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SessionEntryRequest& request,
            std::string& outError)
        {
            outError.clear();

            SessionEntryProtocolReport report = buildReport(runtimeBoundaryReport);
            if (!report.requestQueueReady)
            {
                outError = "Session entry request queue is not ready.";
                return false;
            }

            const std::filesystem::path outputPath =
                report.requestQueueDirectory / ("request_" + sanitizeIdentifier(request.requestId) + ".txt");

            std::ostringstream contents;
            contents
                << "version=1\n"
                << "request_id=" << sanitizeIdentifier(request.requestId) << "\n"
                << "account_id=" << sanitizeIdentifier(request.accountId) << "\n"
                << "player_identity=" << sanitizeIdentifier(request.playerIdentity) << "\n"
                << "client_instance_id=" << sanitizeIdentifier(request.clientInstanceId) << "\n"
                << "build_identity=" << sanitizeSingleLine(request.buildIdentity) << "\n"
                << "environment_name=" << sanitizeIdentifier(request.environmentName) << "\n"
                << "connect_target_name=" << sanitizeIdentifier(request.connectTargetName) << "\n"
                << "requested_resume_session_id=" << sanitizeIdentifier(request.requestedResumeSessionId) << "\n"
                << "requested_at_epoch_ms=" << request.requestedAtEpochMilliseconds << "\n"
                << "requested_ticket_ttl_seconds=" << request.requestedTicketTtlSeconds << "\n"
                << "reconnect_requested=" << (request.reconnectRequested ? "yes" : "no") << "\n";

            return writeTextFileAtomically(outputPath, contents.str(), outError);
        }

        [[nodiscard]] static std::vector<SessionEntryRequest> collectPendingEntryRequestsForHost(
            const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            std::vector<SessionEntryRequest> requests{};
            const std::vector<std::filesystem::path> files =
                enumerateFilesByPrefix(requestQueueDirectory(runtimeBoundaryReport), "request_");

            for (const std::filesystem::path& path : files)
            {
                const KeyValueMap values = parseKeyValueFile(path);
                if (values.empty())
                {
                    removeFileQuietly(path);
                    continue;
                }

                SessionEntryRequest request{};
                uint64_t version = 0;
                uint64_t requestedAtEpochMilliseconds = 0;
                uint64_t requestedTicketTtlSeconds = 0;
                bool reconnectRequested = false;

                if (!tryParseUnsigned(values, "version", version)
                    || !tryParseString(values, "request_id", request.requestId)
                    || !tryParseString(values, "account_id", request.accountId)
                    || !tryParseString(values, "player_identity", request.playerIdentity)
                    || !tryParseString(values, "client_instance_id", request.clientInstanceId)
                    || !tryParseString(values, "build_identity", request.buildIdentity)
                    || !tryParseString(values, "environment_name", request.environmentName)
                    || !tryParseString(values, "connect_target_name", request.connectTargetName)
                    || !tryParseString(values, "requested_resume_session_id", request.requestedResumeSessionId)
                    || !tryParseUnsigned(values, "requested_at_epoch_ms", requestedAtEpochMilliseconds)
                    || !tryParseUnsigned(values, "requested_ticket_ttl_seconds", requestedTicketTtlSeconds)
                    || !tryParseYesNo(values, "reconnect_requested", reconnectRequested))
                {
                    removeFileQuietly(path);
                    continue;
                }

                request.requestedAtEpochMilliseconds = requestedAtEpochMilliseconds;
                request.requestedTicketTtlSeconds = static_cast<uint32_t>(requestedTicketTtlSeconds);
                request.reconnectRequested = reconnectRequested;

                requests.push_back(request);
                removeFileQuietly(path);
            }

            std::sort(requests.begin(), requests.end(), [](const SessionEntryRequest& lhs, const SessionEntryRequest& rhs)
            {
                if (lhs.requestedAtEpochMilliseconds == rhs.requestedAtEpochMilliseconds)
                {
                    return lhs.requestId < rhs.requestId;
                }
                return lhs.requestedAtEpochMilliseconds < rhs.requestedAtEpochMilliseconds;
            });

            return requests;
        }

        [[nodiscard]] static SessionEntryValidationResult validateRequest(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SessionEntryRequest& request,
            const std::string& hostBuildIdentity,
            const std::string& hostEnvironmentName,
            const std::string& hostConnectTargetName,
            uint64_t nowEpochMilliseconds)
        {
            SessionEntryValidationResult result{};
            result.reconnectRequested = request.reconnectRequested;

            if (sanitizeIdentifier(request.requestId) == "unassigned"
                || sanitizeIdentifier(request.accountId) == "anonymous"
                || sanitizeIdentifier(request.playerIdentity) == "unassigned"
                || sanitizeIdentifier(request.clientInstanceId) == "unknown"
                || sanitizeSingleLine(request.buildIdentity) == "unknown")
            {
                result.denied = true;
                result.denialReason = "missing-request-identity";
                return result;
            }

            if (request.requestedTicketTtlSeconds == 0u || request.requestedTicketTtlSeconds > 600u)
            {
                result.denied = true;
                result.denialReason = "invalid-ticket-ttl";
                return result;
            }

            const uint64_t expiryEpochMilliseconds =
                request.requestedAtEpochMilliseconds
                + (static_cast<uint64_t>(request.requestedTicketTtlSeconds) * 1000ull);
            if (request.requestedAtEpochMilliseconds == 0ull || nowEpochMilliseconds > expiryEpochMilliseconds)
            {
                result.denied = true;
                result.expired = true;
                result.denialReason = "expired-request";
                return result;
            }

            if (sanitizeIdentifier(request.environmentName) != sanitizeIdentifier(hostEnvironmentName))
            {
                result.denied = true;
                result.denialReason = "environment-mismatch";
                return result;
            }

            if (sanitizeIdentifier(request.connectTargetName) != sanitizeIdentifier(hostConnectTargetName))
            {
                result.denied = true;
                result.denialReason = "connect-target-mismatch";
                return result;
            }

            if (!buildIdentityCompatible(request.buildIdentity, hostBuildIdentity))
            {
                result.denied = true;
                result.denialReason = "build-identity-mismatch";
                return result;
            }

            if (request.reconnectRequested)
            {
                result.denied = true;
                result.denialReason =
                    sanitizeIdentifier(request.requestedResumeSessionId) == "none"
                        ? "resume-session-missing"
                        : "resume-not-yet-enabled";
                return result;
            }

            const std::vector<ActiveSessionRecord> activeSessions = collectActiveSessions(runtimeBoundaryReport);
            for (const ActiveSessionRecord& activeSession : activeSessions)
            {
                if (sanitizeIdentifier(activeSession.accountId) == sanitizeIdentifier(request.accountId)
                    || sanitizeIdentifier(activeSession.clientInstanceId) == sanitizeIdentifier(request.clientInstanceId))
                {
                    result.denied = true;
                    result.denialReason = "active-session-already-bound";
                    return result;
                }
            }

            result.accepted = true;
            result.denialReason = "none";
            return result;
        }

        [[nodiscard]] static SessionTicket buildIssuedTicket(
            const SessionEntryRequest& request,
            const std::string& hostBuildIdentity,
            uint64_t nowEpochMilliseconds)
        {
            SessionTicket ticket{};
            ticket.ticketId =
                std::string("ticket-")
                + sanitizeIdentifier(request.requestId)
                + "-"
                + std::to_string(nowEpochMilliseconds);
            ticket.requestId = sanitizeIdentifier(request.requestId);
            ticket.accountId = sanitizeIdentifier(request.accountId);
            ticket.playerIdentity = sanitizeIdentifier(request.playerIdentity);
            ticket.clientInstanceId = sanitizeIdentifier(request.clientInstanceId);
            ticket.buildIdentity = sanitizeSingleLine(hostBuildIdentity);
            ticket.environmentName = sanitizeIdentifier(request.environmentName);
            ticket.connectTargetName = sanitizeIdentifier(request.connectTargetName);
            ticket.grantedSessionId =
                std::string("session-")
                + sanitizeIdentifier(request.accountId)
                + "-"
                + std::to_string(nowEpochMilliseconds);
            ticket.denialReason = "none";
            ticket.resumeSessionId = sanitizeIdentifier(request.requestedResumeSessionId);
            ticket.issuedAtEpochMilliseconds = nowEpochMilliseconds;
            ticket.expiresAtEpochMilliseconds =
                nowEpochMilliseconds + (static_cast<uint64_t>(request.requestedTicketTtlSeconds) * 1000ull);
            ticket.consumedAtEpochMilliseconds = 0ull;
            ticket.state = SessionTicketState::Issued;
            return ticket;
        }

        [[nodiscard]] static SessionTicket buildDeniedTicket(
            const SessionEntryRequest& request,
            const std::string& hostBuildIdentity,
            uint64_t nowEpochMilliseconds,
            const std::string& denialReason)
        {
            SessionTicket ticket{};
            ticket.ticketId =
                std::string("deny-")
                + sanitizeIdentifier(request.requestId)
                + "-"
                + std::to_string(nowEpochMilliseconds);
            ticket.requestId = sanitizeIdentifier(request.requestId);
            ticket.accountId = sanitizeIdentifier(request.accountId);
            ticket.playerIdentity = sanitizeIdentifier(request.playerIdentity);
            ticket.clientInstanceId = sanitizeIdentifier(request.clientInstanceId);
            ticket.buildIdentity = sanitizeSingleLine(hostBuildIdentity);
            ticket.environmentName = sanitizeIdentifier(request.environmentName);
            ticket.connectTargetName = sanitizeIdentifier(request.connectTargetName);
            ticket.grantedSessionId = "none";
            ticket.denialReason = sanitizeIdentifier(denialReason);
            ticket.resumeSessionId = sanitizeIdentifier(request.requestedResumeSessionId);
            ticket.issuedAtEpochMilliseconds = nowEpochMilliseconds;
            ticket.expiresAtEpochMilliseconds = nowEpochMilliseconds;
            ticket.consumedAtEpochMilliseconds = 0ull;
            ticket.state = SessionTicketState::Denied;
            return ticket;
        }

        [[nodiscard]] static ActiveSessionRecord buildActiveSessionRecord(
            const SessionTicket& ticket,
            uint64_t nowEpochMilliseconds)
        {
            ActiveSessionRecord record{};
            record.sessionId = sanitizeIdentifier(ticket.grantedSessionId);
            record.ticketId = sanitizeIdentifier(ticket.ticketId);
            record.requestId = sanitizeIdentifier(ticket.requestId);
            record.accountId = sanitizeIdentifier(ticket.accountId);
            record.playerIdentity = sanitizeIdentifier(ticket.playerIdentity);
            record.clientInstanceId = sanitizeIdentifier(ticket.clientInstanceId);
            record.environmentName = sanitizeIdentifier(ticket.environmentName);
            record.connectTargetName = sanitizeIdentifier(ticket.connectTargetName);
            record.state = "active";
            record.issuedAtEpochMilliseconds = ticket.issuedAtEpochMilliseconds;
            record.lastValidatedAtEpochMilliseconds = nowEpochMilliseconds;
            return record;
        }



[[nodiscard]] static std::vector<SessionTicket> collectIssuedTickets(
    const RuntimeBoundaryReport& runtimeBoundaryReport)
{
    return collectTicketsFromDirectory(issuedTicketDirectory(runtimeBoundaryReport));
}

[[nodiscard]] static std::vector<SessionTicket> collectDeniedTickets(
    const RuntimeBoundaryReport& runtimeBoundaryReport)
{
    return collectTicketsFromDirectory(deniedTicketDirectory(runtimeBoundaryReport));
}

        [[nodiscard]] static bool writeIssuedTicket(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SessionTicket& ticket,
            std::string& outError)
        {
            return writeTicketFile(issuedTicketDirectory(runtimeBoundaryReport), ticket, outError);
        }

        [[nodiscard]] static bool writeDeniedTicket(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const SessionTicket& ticket,
            std::string& outError)
        {
            return writeTicketFile(deniedTicketDirectory(runtimeBoundaryReport), ticket, outError);
        }

        [[nodiscard]] static bool writeActiveSession(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const ActiveSessionRecord& record,
            std::string& outError)
        {
            outError.clear();

            const std::filesystem::path directory = activeSessionDirectory(runtimeBoundaryReport);
            if (!directoryExists(directory))
            {
                outError = std::string("Active session directory is not ready: ") + directory.generic_string();
                return false;
            }

            const std::filesystem::path outputPath =
                directory / ("session_" + sanitizeIdentifier(record.sessionId) + ".txt");

            std::ostringstream contents;
            contents
                << "version=1\n"
                << "session_id=" << sanitizeIdentifier(record.sessionId) << "\n"
                << "ticket_id=" << sanitizeIdentifier(record.ticketId) << "\n"
                << "request_id=" << sanitizeIdentifier(record.requestId) << "\n"
                << "account_id=" << sanitizeIdentifier(record.accountId) << "\n"
                << "player_identity=" << sanitizeIdentifier(record.playerIdentity) << "\n"
                << "client_instance_id=" << sanitizeIdentifier(record.clientInstanceId) << "\n"
                << "environment_name=" << sanitizeIdentifier(record.environmentName) << "\n"
                << "connect_target_name=" << sanitizeIdentifier(record.connectTargetName) << "\n"
                << "state=" << sanitizeIdentifier(record.state) << "\n"
                << "issued_at_epoch_ms=" << record.issuedAtEpochMilliseconds << "\n"
                << "last_validated_at_epoch_ms=" << record.lastValidatedAtEpochMilliseconds << "\n";

            return writeTextFileAtomically(outputPath, contents.str(), outError);
        }

        [[nodiscard]] static std::vector<ActiveSessionRecord> collectActiveSessions(
            const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            std::vector<ActiveSessionRecord> sessions{};
            const std::vector<std::filesystem::path> files =
                enumerateFilesByPrefix(activeSessionDirectory(runtimeBoundaryReport), "session_");

            for (const std::filesystem::path& path : files)
            {
                const KeyValueMap values = parseKeyValueFile(path);
                if (values.empty())
                {
                    continue;
                }

                ActiveSessionRecord record{};
                uint64_t version = 0;
                uint64_t issuedAtEpochMilliseconds = 0;
                uint64_t lastValidatedAtEpochMilliseconds = 0;

                if (!tryParseUnsigned(values, "version", version)
                    || !tryParseString(values, "session_id", record.sessionId)
                    || !tryParseString(values, "ticket_id", record.ticketId)
                    || !tryParseString(values, "request_id", record.requestId)
                    || !tryParseString(values, "account_id", record.accountId)
                    || !tryParseString(values, "player_identity", record.playerIdentity)
                    || !tryParseString(values, "client_instance_id", record.clientInstanceId)
                    || !tryParseString(values, "environment_name", record.environmentName)
                    || !tryParseString(values, "connect_target_name", record.connectTargetName)
                    || !tryParseString(values, "state", record.state)
                    || !tryParseUnsigned(values, "issued_at_epoch_ms", issuedAtEpochMilliseconds)
                    || !tryParseUnsigned(values, "last_validated_at_epoch_ms", lastValidatedAtEpochMilliseconds))
                {
                    continue;
                }

                record.issuedAtEpochMilliseconds = issuedAtEpochMilliseconds;
                record.lastValidatedAtEpochMilliseconds = lastValidatedAtEpochMilliseconds;
                sessions.push_back(record);
            }

            std::sort(sessions.begin(), sessions.end(), [](const ActiveSessionRecord& lhs, const ActiveSessionRecord& rhs)
            {
                if (lhs.issuedAtEpochMilliseconds == rhs.issuedAtEpochMilliseconds)
                {
                    return lhs.sessionId < rhs.sessionId;
                }
                return lhs.issuedAtEpochMilliseconds < rhs.issuedAtEpochMilliseconds;
            });

            return sessions;
        }

        [[nodiscard]] static bool readIssuedTicket(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& ticketId,
            SessionTicket& outTicket,
            std::string& outError)
        {
            return readTicketFile(
                issuedTicketDirectory(runtimeBoundaryReport) / ("ticket_" + sanitizeIdentifier(ticketId) + ".txt"),
                outTicket,
                outError);
        }

        [[nodiscard]] static bool readDeniedTicket(
            const RuntimeBoundaryReport& runtimeBoundaryReport,
            const std::string& ticketId,
            SessionTicket& outTicket,
            std::string& outError)
        {
            return readTicketFile(
                deniedTicketDirectory(runtimeBoundaryReport) / ("ticket_" + sanitizeIdentifier(ticketId) + ".txt"),
                outTicket,
                outError);
        }

        [[nodiscard]] static size_t pendingRequestCount(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return countFilesByPrefix(requestQueueDirectory(runtimeBoundaryReport), "request_");
        }

        [[nodiscard]] static size_t issuedTicketCount(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return countFilesByPrefix(issuedTicketDirectory(runtimeBoundaryReport), "ticket_");
        }

        [[nodiscard]] static size_t deniedTicketCount(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return countFilesByPrefix(deniedTicketDirectory(runtimeBoundaryReport), "ticket_");
        }

        [[nodiscard]] static size_t activeSessionCount(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return countFilesByPrefix(activeSessionDirectory(runtimeBoundaryReport), "session_");
        }

        [[nodiscard]] static std::filesystem::path sessionEntryRootDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return runtimeBoundaryReport.hostDirectory / "SessionEntry";
        }

        [[nodiscard]] static std::filesystem::path requestQueueDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return sessionEntryRootDirectory(runtimeBoundaryReport) / "Requests";
        }

        [[nodiscard]] static std::filesystem::path issuedTicketDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return sessionEntryRootDirectory(runtimeBoundaryReport) / "IssuedTickets";
        }

        [[nodiscard]] static std::filesystem::path deniedTicketDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return sessionEntryRootDirectory(runtimeBoundaryReport) / "DeniedTickets";
        }

        [[nodiscard]] static std::filesystem::path activeSessionDirectory(const RuntimeBoundaryReport& runtimeBoundaryReport)
        {
            return sessionEntryRootDirectory(runtimeBoundaryReport) / "ActiveSessions";
        }

        [[nodiscard]] static std::string sanitizeIdentifier(std::string value)
        {
            if (value.empty())
            {
                return "unassigned";
            }

            std::string sanitized;
            sanitized.reserve(value.size());

            for (const unsigned char raw : value)
            {
                const char ch = static_cast<char>(raw);
                if ((ch >= 'a' && ch <= 'z')
                    || (ch >= 'A' && ch <= 'Z')
                    || (ch >= '0' && ch <= '9')
                    || ch == '_'
                    || ch == '-'
                    || ch == '.')
                {
                    sanitized.push_back(static_cast<char>(std::tolower(raw)));
                }
                else if (std::isspace(raw) != 0)
                {
                    sanitized.push_back('_');
                }
            }

            while (!sanitized.empty() && (sanitized.front() == '.' || sanitized.front() == '_'))
            {
                sanitized.erase(sanitized.begin());
            }

            if (sanitized.empty())
            {
                sanitized = "unassigned";
            }

            return sanitized;
        }

        [[nodiscard]] static std::string sanitizeSingleLine(const std::string& value)
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

        [[nodiscard]] static std::string buildIdentityFlavor(const std::string& value)
        {
            const std::string sanitized = sanitizeSingleLine(value);
            const size_t split = sanitized.find('/');
            if (split == std::string::npos)
            {
                return sanitized;
            }

            return sanitized.substr(0, split);
        }

        [[nodiscard]] static bool buildIdentityCompatible(
            const std::string& requestBuildIdentity,
            const std::string& hostBuildIdentity)
        {
            const std::string sanitizedRequest = sanitizeSingleLine(requestBuildIdentity);
            const std::string sanitizedHost = sanitizeSingleLine(hostBuildIdentity);
            if (sanitizedRequest == sanitizedHost)
            {
                return true;
            }

            return buildIdentityFlavor(sanitizedRequest) == buildIdentityFlavor(sanitizedHost);
        }

        [[nodiscard]] static uint64_t currentEpochMilliseconds()
        {
            FILETIME fileTime{};
            GetSystemTimeAsFileTime(&fileTime);

            ULARGE_INTEGER rawTime{};
            rawTime.LowPart = fileTime.dwLowDateTime;
            rawTime.HighPart = fileTime.dwHighDateTime;

            constexpr uint64_t kWindowsEpochToUnixEpoch100ns = 116444736000000000ull;
            constexpr uint64_t kHundredNanosecondsPerMillisecond = 10000ull;

            if (rawTime.QuadPart <= kWindowsEpochToUnixEpoch100ns)
            {
                return 0ull;
            }

            return (rawTime.QuadPart - kWindowsEpochToUnixEpoch100ns) / kHundredNanosecondsPerMillisecond;
        }

        [[nodiscard]] static const char* sessionTicketStateText(SessionTicketState state)
        {
            switch (state)
            {
            case SessionTicketState::Issued: return "issued";
            case SessionTicketState::Accepted: return "accepted";
            case SessionTicketState::Denied: return "denied";
            case SessionTicketState::Expired: return "expired";
            default: return "none";
            }
        }

        [[nodiscard]] static SessionTicketState parseSessionTicketState(const std::string& value)
        {
            if (value == "issued") return SessionTicketState::Issued;
            if (value == "accepted") return SessionTicketState::Accepted;
            if (value == "denied") return SessionTicketState::Denied;
            if (value == "expired") return SessionTicketState::Expired;
            return SessionTicketState::None;
        }

    private:
        using KeyValueMap = std::unordered_map<std::string, std::string>;

        [[nodiscard]] static bool directoryExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_directory(path, error);
        }

        [[nodiscard]] static bool fileExists(const std::filesystem::path& path)
        {
            std::error_code error;
            return std::filesystem::exists(path, error) && std::filesystem::is_regular_file(path, error);
        }

        static void ensureDirectory(const std::filesystem::path& path, std::vector<std::string>& issues)
        {
            std::error_code error;
            if (!std::filesystem::exists(path, error))
            {
                std::filesystem::create_directories(path, error);
            }

            if (error)
            {
                issues.push_back(std::string("Failed to create session-entry directory: ") + path.generic_string());
            }
        }

        [[nodiscard]] static std::vector<std::filesystem::path> enumerateFilesByPrefix(
            const std::filesystem::path& directory,
            const char* prefix)
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

        [[nodiscard]] static size_t countFilesByPrefix(
            const std::filesystem::path& directory,
            const char* prefix)
        {
            return enumerateFilesByPrefix(directory, prefix).size();
        }

        static void removeFileQuietly(const std::filesystem::path& path)
        {
            std::error_code error;
            std::filesystem::remove(path, error);
        }

        [[nodiscard]] static bool readTextFileForAtomicReplace(const std::filesystem::path& path, std::string& outContents)
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

        [[nodiscard]] static KeyValueMap parseKeyValueFile(const std::filesystem::path& path)
        {
            std::string contents;
            if (!readTextFileForAtomicReplace(path, contents))
            {
                return {};
            }

            KeyValueMap values{};
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

        [[nodiscard]] static bool tryParseUnsigned(const KeyValueMap& values, const char* key, uint64_t& outValue)
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

        [[nodiscard]] static bool tryParseString(const KeyValueMap& values, const char* key, std::string& outValue)
        {
            const auto it = values.find(key);
            if (it == values.end())
            {
                return false;
            }

            outValue = it->second;
            return true;
        }

        [[nodiscard]] static bool tryParseYesNo(const KeyValueMap& values, const char* key, bool& outValue)
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

        [[nodiscard]] static bool writeTextFileAtomically(
            const std::filesystem::path& path,
            const std::string& contents,
            std::string& outError)
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
                outError = std::string("Failed to publish session-entry file atomically: ")
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
                outError = std::string("Failed to publish session-entry file atomically: ") + path.generic_string();
                removeFileQuietly(tempPath);
                return false;
            }
        #endif

            return true;
        }



[[nodiscard]] static std::vector<SessionTicket> collectTicketsFromDirectory(
    const std::filesystem::path& directory)
{
    std::vector<SessionTicket> tickets{};
    const std::vector<std::filesystem::path> files = enumerateFilesByPrefix(directory, "ticket_");

    for (const std::filesystem::path& path : files)
    {
        SessionTicket ticket{};
        std::string error;
        if (readTicketFile(path, ticket, error))
        {
            tickets.push_back(ticket);
        }
    }

    std::sort(tickets.begin(), tickets.end(), [](const SessionTicket& lhs, const SessionTicket& rhs)
    {
        if (lhs.issuedAtEpochMilliseconds == rhs.issuedAtEpochMilliseconds)
        {
            return lhs.ticketId < rhs.ticketId;
        }
        return lhs.issuedAtEpochMilliseconds < rhs.issuedAtEpochMilliseconds;
    });

    return tickets;
}

        [[nodiscard]] static bool writeTicketFile(
            const std::filesystem::path& directory,
            const SessionTicket& ticket,
            std::string& outError)
        {
            outError.clear();

            if (!directoryExists(directory))
            {
                outError = std::string("Session ticket directory is not ready: ") + directory.generic_string();
                return false;
            }

            const std::filesystem::path outputPath =
                directory / ("ticket_" + sanitizeIdentifier(ticket.ticketId) + ".txt");

            std::ostringstream contents;
            contents
                << "version=1\n"
                << "ticket_id=" << sanitizeIdentifier(ticket.ticketId) << "\n"
                << "request_id=" << sanitizeIdentifier(ticket.requestId) << "\n"
                << "account_id=" << sanitizeIdentifier(ticket.accountId) << "\n"
                << "player_identity=" << sanitizeIdentifier(ticket.playerIdentity) << "\n"
                << "client_instance_id=" << sanitizeIdentifier(ticket.clientInstanceId) << "\n"
                << "build_identity=" << sanitizeSingleLine(ticket.buildIdentity) << "\n"
                << "environment_name=" << sanitizeIdentifier(ticket.environmentName) << "\n"
                << "connect_target_name=" << sanitizeIdentifier(ticket.connectTargetName) << "\n"
                << "granted_session_id=" << sanitizeIdentifier(ticket.grantedSessionId) << "\n"
                << "resume_session_id=" << sanitizeIdentifier(ticket.resumeSessionId) << "\n"
                << "denial_reason=" << sanitizeIdentifier(ticket.denialReason) << "\n"
                << "issued_at_epoch_ms=" << ticket.issuedAtEpochMilliseconds << "\n"
                << "expires_at_epoch_ms=" << ticket.expiresAtEpochMilliseconds << "\n"
                << "consumed_at_epoch_ms=" << ticket.consumedAtEpochMilliseconds << "\n"
                << "state=" << sessionTicketStateText(ticket.state) << "\n";

            return writeTextFileAtomically(outputPath, contents.str(), outError);
        }

        [[nodiscard]] static bool readTicketFile(
            const std::filesystem::path& path,
            SessionTicket& outTicket,
            std::string& outError)
        {
            outError.clear();
            outTicket = SessionTicket{};

            if (!fileExists(path))
            {
                outError = std::string("Session ticket file not present: ") + path.generic_string();
                return false;
            }

            const KeyValueMap values = parseKeyValueFile(path);
            if (values.empty())
            {
                outError = std::string("Session ticket file could not be parsed: ") + path.generic_string();
                return false;
            }

            uint64_t version = 0;
            uint64_t issuedAtEpochMilliseconds = 0;
            uint64_t expiresAtEpochMilliseconds = 0;
            uint64_t consumedAtEpochMilliseconds = 0;
            std::string stateText;

            if (!tryParseUnsigned(values, "version", version)
                || !tryParseString(values, "ticket_id", outTicket.ticketId)
                || !tryParseString(values, "request_id", outTicket.requestId)
                || !tryParseString(values, "account_id", outTicket.accountId)
                || !tryParseString(values, "player_identity", outTicket.playerIdentity)
                || !tryParseString(values, "client_instance_id", outTicket.clientInstanceId)
                || !tryParseString(values, "build_identity", outTicket.buildIdentity)
                || !tryParseString(values, "environment_name", outTicket.environmentName)
                || !tryParseString(values, "connect_target_name", outTicket.connectTargetName)
                || !tryParseString(values, "granted_session_id", outTicket.grantedSessionId)
                || !tryParseString(values, "resume_session_id", outTicket.resumeSessionId)
                || !tryParseString(values, "denial_reason", outTicket.denialReason)
                || !tryParseUnsigned(values, "issued_at_epoch_ms", issuedAtEpochMilliseconds)
                || !tryParseUnsigned(values, "expires_at_epoch_ms", expiresAtEpochMilliseconds)
                || !tryParseUnsigned(values, "consumed_at_epoch_ms", consumedAtEpochMilliseconds)
                || !tryParseString(values, "state", stateText))
            {
                outError = std::string("Session ticket file missing required fields: ") + path.generic_string();
                return false;
            }

            outTicket.issuedAtEpochMilliseconds = issuedAtEpochMilliseconds;
            outTicket.expiresAtEpochMilliseconds = expiresAtEpochMilliseconds;
            outTicket.consumedAtEpochMilliseconds = consumedAtEpochMilliseconds;
            outTicket.state = parseSessionTicketState(stateText);
            return true;
        }
    };
}
