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
            const Group* botGroup = bot->GetGroup();
            const std::list<ObjectGuid>& units = *context->GetValue<std::list<ObjectGuid> >("nearest npcs");
            for (const ObjectGuid& guid : units)
            {
                Unit* unit = ai->GetUnit(guid);
                if (!unit || unit->GetTypeId() != TYPEID_UNIT)
                    continue;

                Creature* totem = static_cast<Creature*>(unit);
                if (!totem->IsTotem())
                    continue;

                Unit* totemOwner = totem->GetCreator(totem);
                if (!totemOwner)
                    continue;

                if (totemOwner == bot)
                    return true;

                if (!botGroup || !totemOwner->IsPlayer())
                    continue;

                const Player* totemPlayerOwner = static_cast<Player*>(totemOwner);
#ifdef MANGOSBOT_TWO
                if (totemPlayerOwner->GetGroup() == botGroup)
                    return true;
#else
                if (totemPlayerOwner->GetGroup() == botGroup)
                {
                    if (!botGroup->IsRaidGroup() || botGroup->SameSubGroup(totemPlayerOwner, bot))
                        return true;
                }
#endif
            }

            return false;
        }
    };
}
