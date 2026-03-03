
#include "playerbot/playerbot.h"
#include "SnareTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "MotionGenerators/TargetedMovementGenerator.h"

using namespace ai;

Unit* SnareTargetValue::Calculate()
{
    std::string spell = qualifier;
    float searchRange = ai->GetRange("spell");
    float spellRange = 0.0f;
    if (ai->GetSpellRange(spell, &spellRange) && spellRange > searchRange)
        searchRange = spellRange;

    Unit* enemy = AI_VALUE(Unit*, "enemy player target");
    if (enemy && sServerFacade.GetDistance2d(bot, enemy) <= searchRange)
    {
        Player* plr = dynamic_cast<Player*>(enemy);
        if (plr && !(plr->HasAuraType(SPELL_AURA_MOD_ROOT) || plr->HasAuraType(SPELL_AURA_MOD_STUN)))
        {
            Unit* victim = plr->GetVictim();
            const bool isHittingBot = victim && victim->GetObjectGuid() == bot->GetObjectGuid();
            bool shouldSnare = !plr->IsStopped() || plr->IsNonMeleeSpellCasted(false) || isHittingBot;

            // Melee bots do not need to waste a snare on a target already trading blows in place.
            if (isHittingBot && !ai->IsRanged(bot) && !plr->IsNonMeleeSpellCasted(false))
                shouldSnare = false;

            if (shouldSnare)
                return enemy;
        }
    }

    const std::list<ObjectGuid>& attackers = *ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets");
    for (const ObjectGuid& guid : attackers)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit)
            continue;

        if (sServerFacade.GetDistance2d(bot, unit) > searchRange)
            continue;

        // case real player or bot not moving
        if (unit->IsPlayer() && unit->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
        {
            if (Unit* victim = unit->GetVictim())
            {
                // check if need to snare
                bool shouldSnare = true;

                // do not slow down if bot is melee and mob/bot attack each other
                if (!ai->IsRanged(bot) && victim == bot && !unit->IsNonMeleeSpellCasted(false))
                    shouldSnare = false;

                if (unit->HasAuraType(SPELL_AURA_MOD_ROOT) || unit->HasAuraType(SPELL_AURA_MOD_STUN))
                    shouldSnare = false;

                if (victim && shouldSnare)
                    return unit;
            }
        }

        Unit* chaseTarget;
        switch (unit->GetMotionMaster()->GetCurrentMovementGeneratorType())
        {
        case FLEEING_MOTION_TYPE:
            return unit;
        case CHASE_MOTION_TYPE:
            chaseTarget = sServerFacade.GetChaseTarget(unit);
            if (!chaseTarget) continue;
            Player* chaseTargetPlayer = sObjectMgr.GetPlayer(chaseTarget->GetObjectGuid());
            
            // check if need to snare
            bool shouldSnare = true;

            // do not slow down if bot is melee and mob/bot attack each other
            if (chaseTargetPlayer && !ai->IsRanged(bot) && chaseTargetPlayer == bot)
                shouldSnare = false;

            if (!sServerFacade.isMoving(unit))
                shouldSnare = false;

            if (unit->HasAuraType(SPELL_AURA_MOD_ROOT) || unit->HasAuraType(SPELL_AURA_MOD_STUN))
                shouldSnare = false;

            if (chaseTargetPlayer && shouldSnare && !ai->IsTank(chaseTargetPlayer))
                return unit;
        }
    }

    return NULL;
}
