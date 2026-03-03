
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
    for (const ObjectGuid& guid : attackers)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || unit == target)
            continue;

        if (sServerFacade.GetDistance2d(bot, unit) > searchRange)
            continue;

        if (!ai->IsInterruptableSpellCasting(unit, spell, true))
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && IsPositiveSpell(spell->m_spellInfo))
            return unit;

        spell = unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (spell && IsPositiveSpell(spell->m_spellInfo))
            return unit;
    }

    return NULL;
}
