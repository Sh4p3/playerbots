
#include "playerbot/playerbot.h"
#include "CurrentCcTargetValue.h"

using namespace ai;

Unit* CurrentCcTargetValue::Calculate()
{
    const std::list<ObjectGuid>& possible = *ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible targets no los");
    for (std::list<ObjectGuid>::const_iterator i = possible.begin(); i != possible.end(); ++i)
    {
        Unit* target = ai->GetUnit(*i);
        if (!target)
            continue;

        if (ai->HasMyAura(qualifier, target))
            return target;

        if (qualifier == "polymorph" &&
            (ai->HasMyAura("polymorph: pig", target) || ai->HasMyAura("polymorph: turtle", target)))
        {
            return target;
        }
    }

    return NULL;
}
