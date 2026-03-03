
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

    const std::list<ObjectGuid>& attackers = *ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets");
    Unit* target = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    for (const ObjectGuid& guid : attackers)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || unit == target)
            continue;

        if (bot->GetDistance(unit) > searchRange)
            continue;

        if (!ai->HasAura(qualifier, unit))
            return unit;
    }

    return NULL;
}
