
#include "playerbot/playerbot.h"
#include "DKMultipliers.h"
#include "FrostDKStrategy.h"

using namespace ai;

class FrostDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
	FrostDKStrategyActionNodeFactory()
	{
		//creators["icy touch"] = &icy_touch;
		creators["howling blast"] = &howling_blast;
		creators["frost strike"] = &frost_strike;
		//creators["chains of ice"] = &chains_of_ice;
		creators["rune strike"] = &rune_strike;
		//creators["icy clutch"] = &icy_clutch;
		//creators["horn of winter"] = &horn_of_winter;
		//creators["killing machine"] = &killing_machine;
		//creators["frost presence"] = &frost_presence;
		//creators["deathchill"] = &deathchill;
		//creators["icebound fortitude"] = &icebound_fortitude;
		//creators["mind freeze"] = &mind_freeze;
		//creators["empower weapon"] = &empower_weapon;
		//creators["hungering cold"] = &hungering_cold;
		//creators["unbreakable armor"] = &unbreakable_armor;
		//creators["improved icy talons"] = &improved_icy_talons;
	}

	private:
		static ActionNode* rune_strike(PlayerbotAI* ai)
		{
		return new ActionNode("rune strike",
			/*P*/ NextAction::array(0, new NextAction("blood presence"), NULL),
			/*A*/ NextAction::array(0, new NextAction("melee"), NULL),
			/*C*/ NULL);
		}
		static ActionNode* frost_strike(PlayerbotAI* ai)
		{
		return new ActionNode("frost strike",
			/*P*/ NextAction::array(0, new NextAction("blood presence"), NULL),
			/*A*/ NULL,
			/*C*/ NULL);
		}
		static ActionNode* howling_blast(PlayerbotAI* ai)
		{
		return new ActionNode("howling blast",
			/*P*/ NextAction::array(0, new NextAction("blood presence"), NULL),
			/*A*/ NULL,
			/*C*/ NULL);	
		}
};

FrostDKStrategy::FrostDKStrategy(PlayerbotAI* ai) : GenericDKStrategy(ai)
{
	actionNodeFactories.Add(new FrostDKStrategyActionNodeFactory());
}

NextAction** FrostDKStrategy::GetDefaultCombatActions()
{
	return NextAction::array(0, 
			new NextAction("melee", ACTION_IDLE), NULL);
}

void FrostDKStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericDKStrategy::InitCombatTriggers(triggers);

	triggers.push_back(new TriggerNode(
		"will of the forsaken",
		NextAction::array(0, new NextAction("lichborne", ACTION_DISPEL), NULL)));

	triggers.push_back(new TriggerNode(
		"often",
		NextAction::array(0, new NextAction("unbreakable armor", ACTION_HIGH), NULL)));

	triggers.push_back(new TriggerNode(
		"obliterate",
		NextAction::array(0, new NextAction("obliterate", ACTION_NORMAL + 5), NULL)));

	triggers.push_back(new TriggerNode(
		"killing machine",
		NextAction::array(0, new NextAction("frost strike", ACTION_NORMAL + 4), NULL)));

	triggers.push_back(new TriggerNode(
		"rime",
		NextAction::array(0, new NextAction("howling blast", ACTION_NORMAL + 3), NULL)));

	triggers.push_back(new TriggerNode(
		"frost strike",
		NextAction::array(0, new NextAction("frost strike", ACTION_NORMAL + 2), NULL)));

	triggers.push_back(new TriggerNode(
		"blood strike",
		NextAction::array(0, new NextAction("blood strike", ACTION_NORMAL), NULL)));

	triggers.push_back(new TriggerNode(
		"rune strike",
		NextAction::array(0, new NextAction("rune strike", ACTION_NORMAL), NULL)));
}

void FrostDKAoeStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
	triggers.push_back(new TriggerNode(
		"melee light aoe",
		NextAction::array(0, new NextAction("hungering cold", ACTION_HIGH), NULL)));
	
	triggers.push_back(new TriggerNode(
		"melee light aoe",
		NextAction::array(0, new NextAction("howling blast", ACTION_NORMAL + 4), NULL)));
}
