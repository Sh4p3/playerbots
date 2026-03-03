
#include "playerbot/playerbot.h"
#include "AttackerWithoutAuraTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"

using namespace ai;

Unit* AttackerWithoutAuraTargetValue::Calculate()
{
    float searchRange = ai->GetRange("spell");
    float spellRange = 0.0f;
    if (ai->GetSpellRange(qualifier, &spellRange) && spellRange > searchRange)
        searchRange = spellRange;

    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    Unit* target = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || unit == target)
            continue;

        if (bot->GetDistance(unit) > searchRange)
            continue;

        if (!ai->HasAura(qualifier, unit))
            return unit;
    }

    return NULL;
}
