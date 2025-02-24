
#include "playerbot/playerbot.h"
#include "PaladinTriggers.h"
#include "PaladinActions.h"

using namespace ai;

bool SealTrigger::IsActive()
{
	Unit* target = GetTarget();
	return !ai->HasAura("seal of justice", target) &&
        !ai->HasAura("seal of command", target) &&
        !ai->HasAura("seal of vengeance", target) &&
		!ai->HasAura("seal of righteousness", target) &&
		(!ai->HasAura("seal of light", target) || AI_VALUE2(uint8, "health", "self target") > 85) &&
        (!ai->HasAura("seal of wisdom", target) || AI_VALUE2(uint8, "mana", "self target") > 85) &&
        AI_VALUE2(bool, "combat", "self target");
}

bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::vector<std::string> altBlessings;
        std::vector<std::string> haveBlessings;
        altBlessings.push_back("blessing of might");
        altBlessings.push_back("blessing of wisdom");
        altBlessings.push_back("blessing of kings");
        altBlessings.push_back("blessing of sanctuary");
        altBlessings.push_back("blessing of salvation");
#ifndef MANGOSBOT_TWO
        altBlessings.push_back("blessing of light");
#endif

        for (auto blessing : altBlessings)
        {
            if (AI_VALUE2(uint32, "spell id", blessing))
            {
                haveBlessings.push_back(blessing);

                const std::string greaterBlessing = "greater " + blessing;
                if (AI_VALUE2(uint32, "spell id", greaterBlessing))
                {
                    haveBlessings.push_back(greaterBlessing);
                }
            }
        }

        if (haveBlessings.empty())
        {
            return false;
        }

        bool noBlessings = true;
        for (auto blessing : haveBlessings)
        {
            if (ai->HasMyAura(blessing, bot))
            {
                noBlessings = false;
            }
        }

        return noBlessings;
    }

    return false;
}

bool GreaterBlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::vector<std::string> altBlessings;
        std::vector<std::string> haveBlessings;
        altBlessings.push_back("blessing of might");
        altBlessings.push_back("blessing of wisdom");
        altBlessings.push_back("blessing of kings");
        altBlessings.push_back("blessing of sanctuary");
        altBlessings.push_back("blessing of salvation");
#ifndef MANGOSBOT_TWO
        altBlessings.push_back("blessing of light");
#endif

        for (auto blessing : altBlessings)
        {
            const std::string greaterBlessing = "greater " + blessing;
            if (AI_VALUE2(uint32, "spell id", greaterBlessing))
            {
                haveBlessings.push_back(greaterBlessing);
            }
        }

        if (haveBlessings.empty())
        {
            return false;
        }

        bool noBlessings = true;
        for (auto blessing : haveBlessings)
        {
            if (ai->HasMyAura(blessing, bot))
            {
                noBlessings = false;
            }
        }

        return noBlessings;
    }

    return false;
}

bool BlessingOnPartyTrigger::IsActive()
{
    std::vector<std::string> altBlessings;
    std::vector<std::string> haveBlessings;
    altBlessings.push_back("blessing of might");
    altBlessings.push_back("blessing of wisdom");
    altBlessings.push_back("blessing of kings");
    altBlessings.push_back("blessing of sanctuary");
    altBlessings.push_back("blessing of salvation");
#ifndef MANGOSBOT_TWO
    altBlessings.push_back("blessing of light");
#endif

    for (auto blessing : altBlessings)
    {
        if (AI_VALUE2(uint32, "spell id", blessing))
        {
            haveBlessings.push_back(blessing);

            const std::string greaterBlessing = "greater " + blessing;
            if (AI_VALUE2(uint32, "spell id", greaterBlessing))
            {
                haveBlessings.push_back(greaterBlessing);
            }
        }
    }

    if (haveBlessings.empty())
    {
        return false;
    }

    std::string blessings = "";
    for (auto blessing : haveBlessings)
    {
        blessings += blessing;
        if (blessing != haveBlessings[haveBlessings.size() - 1])
        {
            blessings += ",";
        }
    }

    // Doesn't have any of my blessings
    return AI_VALUE2(Unit*, "party member without my aura", blessings);
}

bool GreaterBlessingOnPartyTrigger::IsActive()
{
    std::vector<std::string> altBlessings;
    std::vector<std::string> haveBlessings;
    altBlessings.push_back("blessing of might");
    altBlessings.push_back("blessing of wisdom");
    altBlessings.push_back("blessing of kings");
    altBlessings.push_back("blessing of sanctuary");
    altBlessings.push_back("blessing of salvation");
#ifndef MANGOSBOT_TWO
    altBlessings.push_back("blessing of light");
#endif

    for (auto blessing : altBlessings)
    {
        const std::string greaterBlessing = "greater " + blessing;
        if (AI_VALUE2(uint32, "spell id", greaterBlessing))
        {
            haveBlessings.push_back(greaterBlessing);
        }
    }

    if (haveBlessings.empty())
    {
        return false;
    }

    std::string blessings = "";
    for (auto blessing : haveBlessings)
    {
        blessings += blessing;
        if (blessing != haveBlessings[haveBlessings.size() - 1])
        {
            blessings += ",";
        }
    }

    // Doesn't have any of my blessings
    Unit* target = AI_VALUE2(Unit*, "party member without my aura", blessings);
    return target && bot->IsInGroup(target);
}

bool NoPaladinAuraTrigger::IsActive()
{
    // List of alternative auras
    const std::vector<std::string> altAuras = {
        "devotion aura",
        "retribution aura",
        "concentration aura",
#ifndef MANGOSBOT_TWO
        "sanctity aura",
#endif
        "shadow resistance aura",
        "fire resistance aura",
        "frost resistance aura",
        "crusader aura"
    };

    bool hasAura = false;
    bool needAura = false;

    for (const auto& aura : altAuras)
    {
        // Check if the aura is available to the bot
        if (AI_VALUE2(uint32, "spell id", aura))
        {
            if (ai->HasMyAura(aura, bot))
            {
                hasAura = true;
                break; // No need to continue if the bot already has an aura
            }

            if (!ai->HasAura(aura, bot))
            {
                needAura = true;
            }
        }
    }

    // If the bot already has one of its own auras, return false
    if (hasAura)
        return false;

    // If no aura is active and an aura is available, return true
    return needAura;
}

bool HammerOfJusticeOnEnemyTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    uint8 targetHP = AI_VALUE2(uint8, "health", GetTargetName());
    uint8 selfHP = AI_VALUE2(uint8, "health", "self target");
    uint8 selfMP = AI_VALUE2(uint8, "mana", "self target");
    bool isMoving = AI_VALUE2(bool, "moving", GetTargetName());

    if (isMoving && target->IsPlayer())
        return true;

    if (targetHP < 10)
        return true;

    if (selfHP < sPlayerbotAIConfig.lowHealth && selfMP > 10)
        return true;

    return false;
}

bool ConsecrationTrigger::IsActive()
{
    if (SpellNoCooldownTrigger::IsActive())
    {
#ifdef MANGOSBOT_TWO
        return true;
#else
        return AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.mediumMana;
#endif
    }

    return false;
}

bool HandOfFreedomTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    if (!spellId)
        return false;

    return BuffOnPartyTrigger::IsActive() && bot->IsSpellReady(spellId) && AI_VALUE(Unit*, "party member to remove roots");
}
