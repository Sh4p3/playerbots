
#include "playerbot/playerbot.h"
#include "DruidTriggers.h"
#include "DruidActions.h"

using namespace ai;

namespace
{
    constexpr time_t ARENA_CYCLONE_RECAST_DELAY = 4;
    constexpr time_t ARENA_ROOTS_RECAST_DELAY = 6;
    constexpr time_t ARENA_HIBERNATE_RECAST_DELAY = 8;
}

bool EntanglingRootsTrigger::IsActive()
{
    if (!HasCcTargetTrigger::IsActive())
        return false;

    if (bot->InArena())
    {
        const time_t lastCast = AI_VALUE2(time_t, "last spell cast time", "entangling roots");
        if (lastCast && time(0) - lastCast < ARENA_ROOTS_RECAST_DELAY)
            return false;
    }

    return true;
}

bool EntanglingRootsKiteTrigger::IsActive() 
{ 
	if (!DebuffTrigger::IsActive())
		return false;

    if (bot->InArena())
    {
        const time_t lastCast = AI_VALUE2(time_t, "last spell cast time", "entangling roots");
        if (lastCast && time(0) - lastCast < ARENA_ROOTS_RECAST_DELAY)
            return false;
    }

    if (AI_VALUE(uint8, "attackers count") > 3)
        return false;

	if (!GetTarget()->HasMana())
		return false;

    std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("attackers")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || !sServerFacade.IsAlive(unit))
            continue;

        if (ai->HasMyAura("entangling roots", unit))
            return false;
    }

    return !HasMaxDebuffs();
}

bool HibernateTrigger::IsActive()
{
    if (!HasCcTargetTrigger::IsActive())
        return false;

    if (bot->InArena())
    {
        const time_t lastCast = AI_VALUE2(time_t, "last spell cast time", "hibernate");
        if (lastCast && time(0) - lastCast < ARENA_HIBERNATE_RECAST_DELAY)
            return false;
    }

    return true;
}

bool ActiveHotTrigger::IsActive()
{
    Unit* target = GetTarget();
    return ai->HasAura("regrowth", target) || ai->HasAura("rejuvenation", target) || ai->HasAura("life bloom", target) || ai->HasAura("wild growth", target);
}

Value<Unit*>* LifebloomTankTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party tank without lifebloom", "lifebloom");
}

bool CyclonePvpTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsPlayer() || target->GetDiminishing(DIMINISHING_CYCLONE) >= DIMINISHING_LEVEL_IMMUNE)
        return false;

    // Check if bot is in cat form and has not Predatory strikes
    if (ai->HasAura("cat form", bot) && !ai->HasAura("predator's swiftness", bot))
        return false;

    // Check bot's health for defensive use
    const uint8 health = AI_VALUE2(uint8, "health", "self target");
    const bool defensiveWindow = health <= sPlayerbotAIConfig.lowHealth && target->GetSelectionGuid() == bot->GetObjectGuid();

    if (bot->InArena() && !defensiveWindow)
    {
        const time_t lastCast = AI_VALUE2(time_t, "last spell cast time", "cyclone");
        if (lastCast && time(0) - lastCast < ARENA_CYCLONE_RECAST_DELAY)
            return false;
    }

    // Keep battleground behavior conservative to avoid Cyclone spam.
    if (bot->InBattleGround() && !bot->InArena() && !defensiveWindow)
        return false;

    // Defensive CC: peel when low.
    if (health <= sPlayerbotAIConfig.lowHealth && target->GetSelectionGuid() == bot->GetObjectGuid())
    {
        if (target->GetDistance(bot) <= 20.0f)
        {
            return true;
        }
    }

    // Avoid targets already crowd-controlled
    if (!PossibleAttackTargetsValue::HasUnBreakableCC(target, bot) &&
        target->GetDistance(bot) <= 20.0f)
    {
        std::list<ObjectGuid> attackers = AI_VALUE(std::list<ObjectGuid>, "attackers");

        if (attackers.size() <= 1)
            return false;

        // Check if Cyclone is already active
        for (const auto& guid : attackers)
        {
            Unit* attacker = ai->GetUnit(guid);
            if (ai->HasAura("cyclone", attacker, false, true))
                return false; // Cyclone already active on another target
        }

        // In arena, use Cyclone offensively mostly for healer casts or when we are under pressure.
        if (bot->InArena())
        {
            Unit* enemyHealer = AI_VALUE(Unit*, "enemy healer target");
            const bool targetIsEnemyHealer = enemyHealer && enemyHealer->GetObjectGuid() == target->GetObjectGuid();
            const bool interruptingHealerCast = targetIsEnemyHealer && target->IsNonMeleeSpellCasted(false);

            Unit* victim = target->GetVictim();
            const bool underPressure = health <= sPlayerbotAIConfig.mediumHealth &&
                                       victim && victim->GetObjectGuid() == bot->GetObjectGuid();

            if (!interruptingHealerCast && !underPressure)
                return false;
        }

        return true;
    }

    return false;
}

bool PredatoryStrikesRegrowthTrigger::IsActive()
{
    return AI_VALUE2(uint8, "health", "self target") <= sPlayerbotAIConfig.mediumHealth 
        && ai->HasAura("predator's swiftness", bot)
        && BuffTrigger::IsActive();
}

bool PredatoryStrikesHealTrigger::IsActive()
{
    return AI_VALUE2(uint8, "health", "self target") <= sPlayerbotAIConfig.mediumHealth
        && ai->HasAura("regrowth", bot)
        && HasAuraTrigger::IsActive();
}
