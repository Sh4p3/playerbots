
#include "playerbot/playerbot.h"
#include "TargetValue.h"

#include <algorithm>
#include "playerbot/ServerFacade.h"
#include "RtiTargetValue.h"
#include "Entities/Unit.h"
#include "LastMovementValue.h"
#include "playerbot/strategy/values/Formations.h"

using namespace ai;

Unit* TargetValue::FindTarget(FindTargetStrategy* strategy)
{
    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit)
            continue;

        ThreatManager &threatManager = sServerFacade.GetThreatManager(unit);
        strategy->CheckAttacker(unit, &threatManager);
    }

    return strategy->GetResult();
}

bool FindNonCcTargetStrategy::IsCcTarget(Unit* attacker)
{
    Group* group = ai->GetBot()->GetGroup();
    if (group)
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player *player = sObjectMgr.GetPlayer(itr->guid);
            if (!player || !sServerFacade.IsAlive(player) || !ai->IsSafe(player))
                continue;

            if (player->GetPlayerbotAI())
            {
                if (PAI_VALUE(Unit*,"rti cc target") == attacker)
                    return true;

                std::string rti = PAI_VALUE(std::string,"rti cc");
                int index = RtiTargetValue::GetRtiIndex(rti);
                if (index != -1)
                {
                    uint64 guid = group->GetTargetIcon(index);
                    if (guid && attacker->GetObjectGuid() == ObjectGuid(guid))
                        return true;
                }
            }
        }

        uint64 guid = group->GetTargetIcon(4);
        if (guid && attacker->GetObjectGuid() == ObjectGuid(guid))
            return true;
    }

    return false;
}

void FindTargetStrategy::GetPlayerCount(Unit* creature, int* tankCount, int* dpsCount)
{
    Player* bot = ai->GetBot();
    if (tankCountCache.find(creature) != tankCountCache.end())
    {
        *tankCount = tankCountCache[creature];
        *dpsCount = dpsCountCache[creature];
        return;
    }

    *tankCount = 0;
    *dpsCount = 0;

    Unit::AttackerSet attackers(creature->getAttackers());
    for (std::set<Unit*>::const_iterator i = attackers.begin(); i != attackers.end(); i++)
    {
        Unit* attacker = *i;
        if (!attacker || !sServerFacade.IsAlive(attacker) || attacker == bot)
            continue;

        Player* player = dynamic_cast<Player*>(attacker);
        if (!player)
            continue;

        if (ai->IsTank(player))
            (*tankCount)++;
        else
            (*dpsCount)++;
    }

    tankCountCache[creature] = *tankCount;
    dpsCountCache[creature] = *dpsCount;
}

TravelTarget* LeaderTravelTargetValue::Calculate()
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    Player* player = ai->GetGroupMaster();
    if (!player || player == bot || !player->GetPlayerbotAI() || !ai->IsSafe(player))
        return target;

    if (bot->GetGroup() && !ai->IsGroupLeader())
        if (!(ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) ||
            ai->HasStrategy("wander", BotState::BOT_STATE_NON_COMBAT)) &&
            !ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) &&
            !ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT))
            return target;

     TravelTarget* leaderTarget = PAI_VALUE(TravelTarget*, "travel target");

     if (!leaderTarget)
         return target;

     return leaderTarget;
}

WorldPosition LastLongMoveValue::Calculate()
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    if (lastMove.lastPath.empty())
        return WorldPosition();

    return lastMove.lastPath.getBack();
}

WorldPosition HomeBindValue::Calculate()
{
    float x, y, z;
    uint32 mapId;
    bot->GetHomebindLocation(x, y, z, mapId);
    return WorldPosition(mapId, x, y, z, 0.0);
}

std::string HomeBindValue::Format()
{
    WorldPosition pos = this->Calculate();
    return chat->formatWorldPosition(pos);
}

void PullTargetValue::Set(Unit* unit)
{
    guid = unit ? unit->GetObjectGuid() : ObjectGuid();
}

Unit* PullTargetValue::Get()
{
    Unit* unit = nullptr;
    if (!guid.IsEmpty())
    {
        unit = sObjectAccessor.GetUnit(*bot, guid);
    }
    
    return unit;
}

Unit* FollowTargetValue::Calculate()
{
    Unit* followTarget = AI_VALUE(GuidPosition, "manual follow target").GetUnit(bot->GetInstanceId());
    if (followTarget == nullptr)
    {
        Formation* formation = AI_VALUE(Formation*, "formation");
        if (formation && !formation->GetTargetName().empty())
        {
            followTarget = AI_VALUE(Unit*, formation->GetTargetName());
        }
        else
        {
            followTarget = AI_VALUE(Unit*, "master target");
        }
    }

    return followTarget;
}

Unit* ClosestAttackerTargetingMeTargetValue::Calculate()
{
    Unit* result = nullptr;
    float closest = 9999.0f;

    const std::list<ObjectGuid>& attackers = AI_VALUE(std::list<ObjectGuid>, "attackers targeting me");
    for (const ObjectGuid& attackerGuid : attackers)
    {
        Unit* attacker = ai->GetUnit(attackerGuid);
        if (attacker)
        {
            const float distance = bot->GetDistance(attacker, true, DIST_CALC_COMBAT_REACH);
            if (distance < closest)
            {
                closest = distance;
                result = attacker;
            }
        }
    }

    return result;
}

std::list<ObjectGuid> FriendlyManualTargetsValue::Get()
{
    value.remove_if([&](const ObjectGuid& playerGuid)
    {
        Unit* player = ai->GetUnit(playerGuid);
        if (ai->IsSafe(player))
        {
            if (bot->IsInGroup(player))
            {
                return false;
            }
        }

        return true;
    });

    return value;
}

std::list<ObjectGuid> FriendlyManualTargetsValue::LazyGet()
{
    return Get();
}

static bool HasTarget(const std::list<ObjectGuid>& targets, const ObjectGuid& guid)
{
    return std::find(targets.begin(), targets.end(), guid) != targets.end();
}

static void AppendTarget(std::list<ObjectGuid>& targets, PlayerbotAI* ai, Unit* target)
{
    Player* bot = ai->GetBot();
    if (!target || !ai->IsSafe(target))
        return;

    if (target != bot && !bot->IsInGroup(target))
        return;

    const ObjectGuid guid = target->GetObjectGuid();
    if (!HasTarget(targets, guid))
        targets.push_back(guid);
}

std::list<ObjectGuid> PreferredBoostTargetsValue::Get()
{
    std::list<ObjectGuid> targets = FriendlyManualTargetsValue::Get();
    if (!targets.empty())
        return targets;

    if (ai->HasStrategy("focus heal targets", BotState::BOT_STATE_COMBAT))
    {
        const std::list<ObjectGuid> focusHealTargets = AI_VALUE(std::list<ObjectGuid>, "focus heal targets");
        for (const ObjectGuid& targetGuid : focusHealTargets)
        {
            AppendTarget(targets, ai, ai->GetUnit(targetGuid));
        }
    }

    AppendTarget(targets, ai, bot);
    return targets;
}

std::list<ObjectGuid> FearWardTargetsValue::Get()
{
    std::list<ObjectGuid> targets = FriendlyManualTargetsValue::Get();
    if (!targets.empty())
        return targets;

    AppendTarget(targets, ai, AI_VALUE2(Unit*, "party tank without aura", "fear ward"));

    if (ai->HasStrategy("focus heal targets", BotState::BOT_STATE_COMBAT))
    {
        const std::list<ObjectGuid> focusHealTargets = AI_VALUE(std::list<ObjectGuid>, "focus heal targets");
        for (const ObjectGuid& targetGuid : focusHealTargets)
        {
            AppendTarget(targets, ai, ai->GetUnit(targetGuid));
        }
    }

    AppendTarget(targets, ai, bot);
    return targets;
}
