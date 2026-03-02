#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"
#include "playerbot/LootObjectStack.h"

namespace ai
{
    class HaveAnyTotemValue : public BoolCalculatedValue, public Qualified
	{
	public:
        HaveAnyTotemValue(PlayerbotAI* ai, std::string name = "have any totem") : BoolCalculatedValue(ai, name), Qualified() {}

        bool Calculate() override
        {
            std::list<ObjectGuid> units = *context->GetValue<std::list<ObjectGuid> >("nearest npcs");
            for (const ObjectGuid& guid : units)
            {
                Unit* unit = ai->GetUnit(guid);
                if (!unit || unit->GetTypeId() != TYPEID_UNIT)
                    continue;

                if (static_cast<Creature*>(unit)->IsTotem()) 
                {
                    return true;
                }
            }

            return false;
        }
    };
}
