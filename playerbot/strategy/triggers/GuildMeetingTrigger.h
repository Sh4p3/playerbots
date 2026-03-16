#include "playerbot/playerbot.h"
#include "playerbot/strategy/Trigger.h"
#include <string>
#include <ctime>

namespace ai
{
    struct GuildMeetingData
    {
        std::string location;
        time_t startTime = 0;
        time_t endTime = 0;
    };

    bool TryGetGuildMeetingData(Player* bot, GuildMeetingData& outMeeting);

    class GuildMeetingTrigger : public Trigger
    {
    public:
        GuildMeetingTrigger(PlayerbotAI* ai) : Trigger(ai, "guild meeting", 60) {}

        bool IsActive() override;
    };
}
