
#include "playerbot/playerbot.h"
#include "CurrentCcTargetValue.h"

using namespace ai;

class FindCurrentCcTargetStrategy : public FindTargetStrategy
{
public:
    FindCurrentCcTargetStrategy(PlayerbotAI* ai, std::string spell) : FindTargetStrategy(ai)
    {
        this->spell = spell;
    }

public:
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) override
    {
        if (ai->HasMyAura(spell, attacker))
        {
            result = attacker;
            return;
        }

        if (spell == "polymorph" &&
            (ai->HasMyAura("polymorph: pig", attacker) || ai->HasMyAura("polymorph: turtle", attacker)))
            result = attacker;
    }

private:
    std::string spell;
};


Unit* CurrentCcTargetValue::Calculate()
{
    FindCurrentCcTargetStrategy strategy(ai, qualifier);
    return FindTarget(&strategy);
}
