#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"
#include "playerbot/LootObjectStack.h"

namespace ai
{
    class HasTotemValue : public BoolCalculatedValue, public Qualified
	{
	public:
        HasTotemValue(PlayerbotAI* ai, std::string name = "has totem") : BoolCalculatedValue(ai, name), Qualified() {}

        bool Calculate() override
        {
            const float spellRange = ai->GetRange("spell");
            std::list<ObjectGuid> units = *context->GetValue<std::list<ObjectGuid> >("nearest npcs");
            for (const ObjectGuid& guid : units)
            {
                Unit* unit = ai->GetUnit(guid);
                if (!unit || unit->GetTypeId() != TYPEID_UNIT)
                    continue;

                Creature* totem = static_cast<Creature*>(unit);
                if (!totem->IsTotem())
                    continue;
                
                const bool totemIsInRange = strstri(totem->GetName(), qualifier.c_str()) &&
                    sServerFacade.GetDistance2d(bot, totem) <= spellRange;
                
                if (!totemIsInRange) continue;

                Unit* totemOwner = totem->GetCreator(totem);
                if (!totemOwner)
                    continue;

                if (totemOwner == bot) return true; // Found our own totem in range

                const Group* botGroup = bot->GetGroup();
                if (!botGroup) continue;

                if (totemOwner->IsPlayer())
                {
                    const Player* totemPlayerOwner = static_cast<Player*>(totemOwner);
#ifdef MANGOSBOT_TWO
                    // Most totems are raid-wide in Wrath - Special handling should be added for the handful that aren't
                    if (totemPlayerOwner->GetGroup() == botGroup) return true;
#else
                    // Totem from a player in our group found, and we are either not in a raid or in the same raid subgroup
                    if (totemPlayerOwner->GetGroup() == botGroup)
                    {
                        if (!botGroup->IsRaidGroup() || botGroup->SameSubGroup(totemPlayerOwner, bot)) return true;
                    }
#endif
                }
            }

            return false;
        }
    };
}
