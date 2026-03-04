
#include "playerbot/playerbot.h"
#include "EnemyHealerTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

Unit* EnemyHealerTargetValue::Calculate()
{
    std::string spell = qualifier;
    float searchRange = ai->GetRange("spell");
    float spellRange = 0.0f;
    if (ai->GetSpellRange(spell, &spellRange) && spellRange > searchRange)
        searchRange = spellRange;

    const std::list<ObjectGuid>& attackers = *ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets");
    Unit* target = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    Unit* bestTarget = NULL;
    float bestDistance = std::numeric_limits<float>::max();
    for (const ObjectGuid& guid : attackers)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || unit == target)
            continue;

        float distance = sServerFacade.GetDistance2d(bot, unit);
        if (distance > searchRange)
            continue;

        if (!ai->IsInterruptableSpellCasting(unit, spell, true))
            continue;

        bool positiveCast = false;
        Spell* currentSpell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (currentSpell && IsPositiveSpell(currentSpell->m_spellInfo))
            positiveCast = true;

        currentSpell = unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (currentSpell && IsPositiveSpell(currentSpell->m_spellInfo))
            positiveCast = true;

        if (positiveCast && distance < bestDistance)
        {
            bestDistance = distance;
            bestTarget = unit;
        }
    }

    return bestTarget;
}
