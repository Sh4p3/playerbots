#pragma once

#include "PlayerbotAI.h"

class Player;

namespace ai
{
    class AiObjectContext;
    class ReactionEngine;
    class Engine;
}

using namespace ai;

class AiFactory
{
public:
    static AiObjectContext* createAiObjectContext(Player* player, PlayerbotAI* ai);
	static Engine* createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext);
	static Engine* createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext);
    static Engine* createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext);
    static ReactionEngine* createReactionEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext);
    static void AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine, int specTab = -1);
    static void AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine, int specTab = -1);
    static void AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* engine, int specTab = -1);
    static void AddDefaultReactionStrategies(Player* player, PlayerbotAI* const facade, ReactionEngine* reactionEngine, int specTab = -1);

public:
    static int GetPlayerSpecTab(const Player* player);
    static std::map<uint32, int32> GetPlayerSpecTabs(const Player* player);
    static BotRoles GetPlayerRoles(const Player* player);
};
