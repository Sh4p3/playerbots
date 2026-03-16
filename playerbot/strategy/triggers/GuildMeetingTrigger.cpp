#include "playerbot/playerbot.h"
#include "GuildMeetingTrigger.h"
#include "Guilds/GuildMgr.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <regex>

using namespace ai;

namespace
{
    bool ParseTimeTokenToHM(const std::string& token, int& outHour, int& outMin)
    {
        std::smatch m;
        std::regex re(R"((\d{1,2}):(\d{2})([AaPp][Mm])?)");
        if (!std::regex_match(token, m, re))
            return false;

        int hour = std::stoi(m[1].str());
        int minute = std::stoi(m[2].str());
        bool hasAmPm = m[3].matched;
        std::string ampm;
        if (hasAmPm)
            ampm = m[3].str();

        if (minute < 0 || minute > 59 || hour < 0 || hour > 23)
            return false;

        if (hasAmPm)
        {
            std::string up = ampm;
            for (char& c : up)
                c = static_cast<char>(toupper(static_cast<unsigned char>(c)));

            if (up == "AM")
            {
                if (hour == 12)
                    hour = 0;
            }
            else if (up == "PM")
            {
                if (hour != 12)
                    hour = (hour % 12) + 12;
            }
        }

        outHour = hour;
        outMin = minute;
        return true;
    }

    bool IsMeridiemToken(const std::string& token)
    {
        std::string upper = token;
        std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return static_cast<char>(toupper(c)); });
        return upper == "AM" || upper == "PM";
    }

    bool PopTimeToken(std::vector<std::string>& tokens, std::string& outToken)
    {
        if (tokens.empty())
            return false;

        outToken = tokens.back();
        tokens.pop_back();

        if (IsMeridiemToken(outToken))
        {
            if (tokens.empty())
                return false;

            outToken = tokens.back() + outToken;
            tokens.pop_back();
        }

        return true;
    }

    bool TryParseGuildMeetingMotd(const std::string& motd, GuildMeetingData& outMeeting)
    {
        if (motd.empty())
            return false;

        const std::string prefix = "Meeting:";
        const std::string::size_type pos = motd.find(prefix);
        if (pos == std::string::npos)
            return false;

        std::string body = motd.substr(pos + prefix.size());
        auto ltrim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        };
        auto rtrim = [](std::string& s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        };

        ltrim(body);
        rtrim(body);
        if (body.empty())
            return false;

        std::vector<std::string> tokens;
        std::istringstream iss(body);
        std::string token;
        while (iss >> token)
            tokens.push_back(token);

        if (tokens.size() < 3)
            return false;

        std::string endToken;
        std::string startToken;
        if (!PopTimeToken(tokens, endToken) || !PopTimeToken(tokens, startToken))
            return false;

        int sh = 0, sm = 0, eh = 0, em = 0;
        if (!ParseTimeTokenToHM(startToken, sh, sm) || !ParseTimeTokenToHM(endToken, eh, em))
            return false;

        std::ostringstream location;
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            if (i)
                location << " ";

            location << tokens[i];
        }

        outMeeting.location = location.str();
        if (outMeeting.location.empty())
            return false;

        time_t now = time(nullptr);
        tm startLocal = *(localtime(&now));
        tm endLocal = startLocal;

        startLocal.tm_hour = sh;
        startLocal.tm_min = sm;
        startLocal.tm_sec = 0;
        endLocal.tm_hour = eh;
        endLocal.tm_min = em;
        endLocal.tm_sec = 0;

        time_t start = mktime(&startLocal);
        time_t end = mktime(&endLocal);
        if (start == (time_t)-1 || end == (time_t)-1)
            return false;

        const time_t twelveHours = 12 * 3600;
        if (start + twelveHours < now)
            start += 24 * 3600;
        if (end + twelveHours < now)
            end += 24 * 3600;
        if (end < start)
            end += 24 * 3600;

        outMeeting.startTime = start;
        outMeeting.endTime = end;
        return true;
    }
}

bool ai::TryGetGuildMeetingData(Player* bot, GuildMeetingData& outMeeting)
{
    if (!bot || !bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    return TryParseGuildMeetingMotd(guild->GetMOTD(), outMeeting);
}

bool GuildMeetingTrigger::IsActive()
{
    GuildMeetingData meeting;
    if (!TryGetGuildMeetingData(bot, meeting))
        return false;

    time_t now = time(nullptr);
    const time_t startWindow = meeting.startTime - 30 * 60;
    const time_t endWindow = meeting.endTime;

    if (now >= startWindow && now <= endWindow)
    {
        this->param = meeting.location;
        return true;
    }

    return false;
}
