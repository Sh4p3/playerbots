
#include "playerbot/playerbot.h"
#include "DKMultipliers.h"
#include "UnholyDKStrategy.h"

using namespace ai;

class UnholyDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
	UnholyDKStrategyActionNodeFactory()
	{
		// Unholy
		//creators["bone shield"] = &bone_shield;
		//creators["plague strike"] = &plague_strike;
		//creators["death grip"] = &death_grip;
		//creators["death coil"] = &death_coil;
		creators["death strike"] = &death_strike;
		//creators["unholy blight"] = &unholy_blight;
		creators["scourge strike"] = &scourge_strike;
		//creators["death and decay"] = &death_and_decay;
		//creators["unholy presence"] = &unholy_presence;
		//creators["raise dead"] = &raise_dead;
		creators["army of the dead"] = &army_of_the_dead;
		//creators["summon gargoyle"] = &army of the dead;
		//creators["anti magic shell"] = &anti_magic_shell;
		//creators["anti magic zone"] = &anti_magic_zone;
		//creators["ghoul frenzy"] = &ghoul_frenzy;
		creators["corpse explosion"] = &corpse_explosion;
	}

private:
	static ActionNode* death_strike(PlayerbotAI* ai)
	{
		return new ActionNode("death strike",
			/*P*/ NextAction::array(0, new NextAction("unholy presence"), NULL),
			/*A*/ NULL,
			/*C*/ NULL);
	}
	static ActionNode* corpse_explosion(PlayerbotAI* ai)
	{
		return new ActionNode("corpse explosion",
			/*P*/ NextAction::array(0, new NextAction("unholy presence"), NULL),
			/*A*/ NULL,
			/*C*/ NULL);
	}
	static ActionNode* scourge_strike(PlayerbotAI* ai)
	{
		return new ActionNode("scourge strike",
			/*P*/ NextAction::array(0, new NextAction("unholy presence"), NULL),
			/*A*/ NextAction::array(0, new NextAction("death strike"), NULL),
			/*C*/ NULL);
	}
	static ActionNode* army_of_the_dead(PlayerbotAI* ai)
	{
		return new ActionNode("army of the dead",
			/*P*/ NextAction::array(0, new NextAction("unholy presence"), NULL),
			/*A*/ NextAction::array(0, new NextAction("summon gargoyle"), NULL),
			/*C*/ NULL);
	}
};

UnholyDKStrategy::UnholyDKStrategy(PlayerbotAI* ai) : GenericDKStrategy(ai)
{
	actionNodeFactories.Add(new UnholyDKStrategyActionNodeFactory());
}

NextAction** UnholyDKStrategy::GetDefaultCombatActions()
{
	return NextAction::array(0, 
		new NextAction("death and decay", ACTION_HIGH), new NextAction("ghoul frenzy", ACTION_IDLE + 2),
		new NextAction("death coil", ACTION_IDLE + 1), new NextAction("melee" , ACTION_IDLE), NULL);
}

void UnholyDKStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericDKStrategy::InitCombatTriggers(triggers);

	triggers.push_back(new TriggerNode(
		"summon gargoyle",
		NextAction::array(0, new NextAction("summon gargoyle", ACTION_HIGH + 1), NULL)));
	
	triggers.push_back(new TriggerNode(
		"often",
		NextAction::array(0, new NextAction("army of the dead", ACTION_HIGH), NULL)));
	
	triggers.push_back(new TriggerNode(
		"scourge strike",
		NextAction::array(0, new NextAction("scourge strike", ACTION_NORMAL + 5), NULL)));

	triggers.push_back(new TriggerNode(
		"blood strike",
		NextAction::array(0, new NextAction("blood strike", ACTION_NORMAL + 4), NULL)));

	triggers.push_back(new TriggerNode(
		"critical health",
		NextAction::array(0, new NextAction("death pact", ACTION_EMERGENCY + 1), NULL)));

	triggers.push_back(new TriggerNode(
		"low health",
		NextAction::array(0, new NextAction("death pact", ACTION_CRITICAL_HEAL + 1), NULL)));

	triggers.push_back(new TriggerNode(
		"bone shield",
		NextAction::array(0, new NextAction("bone shield", ACTION_NORMAL + 1), NULL)));
}

void UnholyDKAoeStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"loot available",
		NextAction::array(0, new NextAction("corpse explosion", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "melee medium aoe",
        NextAction::array(0, new NextAction("death and decay", ACTION_NORMAL + 3),
			new NextAction("corpse explosion", ACTION_NORMAL + 3), NULL)));
}