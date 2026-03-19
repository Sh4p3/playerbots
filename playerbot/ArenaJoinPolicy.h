#pragma once

#include "BattleGround/BattleGround.h"
#include <string>

class Player;

namespace ai
{
#ifndef MANGOSBOT_ZERO
struct ArenaPushDiagnostics
{
    ArenaType type = ARENA_TYPE_NONE;
    BattleGroundQueueTypeId queueTypeId = BATTLEGROUND_QUEUE_NONE;
    BattleGroundBracketId bracketId = BattleGroundBracketId(0);
    bool hasArenaTeam = false;
    bool hasMatchingTeam = false;
    bool isCaptain = false;
    bool teamQueued = false;
    bool teamInArena = false;
    bool needBots = false;
    bool autoJoinArena = false;
    bool boostEnabled = false;
    bool boostEligible = false;
    uint32 teamId = 0;
    uint32 teamRating = 0;
    uint32 gamesWeek = 0;
    uint32 requiredMembers = 0;
    uint32 readyMembers = 0;
    std::string teamName;
    std::string blockingReason;
};

ArenaPushDiagnostics GetArenaPushDiagnostics(Player* player, BattleGroundQueueTypeId queueTypeId, BattleGroundBracketId bracketId);
#endif
}
