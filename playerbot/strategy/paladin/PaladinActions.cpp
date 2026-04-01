
#include "playerbot/playerbot.h"
#include "PaladinActions.h"

using namespace ai;

namespace
{
    bool HasAnyConfiguredBlessing(Unit* target, PlayerbotAI* ai, const std::vector<std::string>& blessings, bool greaterOnly)
    {
        if (!target)
            return false;

        for (const std::string& blessing : blessings)
        {
            const std::string greaterBlessing = "greater " + blessing;
            if (greaterOnly)
            {
                if (ai->HasAura(greaterBlessing, target))
                    return true;
            }
            else if (ai->HasAura(blessing, target) || ai->HasAura(greaterBlessing, target))
            {
                return true;
            }
        }

        return false;
    }
}

bool CastPaladinAuraAction::Execute(Event& event)
{
    // List of all paladin auras
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

    for (const auto& aura : altAuras)
    {
        // Check if the aura is available
        if (AI_VALUE2(uint32, "spell id", aura))
        {
            // If the aura is not already active, cast it
            if (!ai->HasAura(aura, bot))
            {
                if (ai->CastSpell(aura, bot, nullptr, false))
                {
                    SetDuration(1.0f); // Prevent immediate re-casting
                    return true;       // Return after successfully casting an aura
                }
            }
        }
    }

    return false; // No aura was cast
}

Unit* CastBlessingAction::GetTarget()
{
    return bot;
}

bool CastBlessingAction::isPossible()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::string blessing = GetBlessingForTarget(target);
        if (!blessing.empty())
        {
            SetSpellName(blessing);
            return CastSpellAction::isPossible();
        }
    }

    return false;
}

std::string CastBlessingAction::GetBlessingForTarget(Unit* target)
{
    std::string chosenBlessing = "";
    if (target)
    {
        std::vector<std::string> possibleBlessings = GetPossibleBlessingsForTarget(target);
        if (!greater)
        {
            // Any already active valid blessing is acceptable: don't swap between alternatives.
            if (HasAnyConfiguredBlessing(target, ai, possibleBlessings, false))
                return "";
        }
        else
        {
            // If a valid greater blessing is already up, do nothing.
            if (HasAnyConfiguredBlessing(target, ai, possibleBlessings, true))
                return "";

            // If a normal blessing is up, only upgrade that same blessing to greater.
            for (const std::string& blessing : possibleBlessings)
            {
                if (!ai->HasAura(blessing, target))
                    continue;

                const std::string greaterBlessing = "greater " + blessing;
                if (ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true))
                    return greaterBlessing;

                return "";
            }
        }

        for (const std::string& blessing : possibleBlessings)
        {
            const std::string greaterBlessing = "greater " + blessing;
            if (!ai->HasAura(blessing, target) && !ai->HasAura(greaterBlessing, target))
            {
                if ((greater && ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true)) ||
                    (!greater && ai->CanCastSpell(blessing, target, 0, nullptr, true)))
                {
                    chosenBlessing = greater ? greaterBlessing : blessing;
                    break;
                }
            }
        }
    }

    return chosenBlessing;
}


Unit* CastBlessingOnPartyAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* player = ref->getSource();
        if (!player || player == bot || player->GetMapId() != bot->GetMapId())
            continue;

        if (HasPossibleBlessingForTarget(player, ai, greater))
            return player;

        Pet* pet = player->GetPet();
        if (pet && pet->GetMapId() == bot->GetMapId() && HasPossibleBlessingForTarget(pet, ai, greater))
            return pet;
    }

    return nullptr;
}

bool CastBlessingOnPartyAction::isPossible()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::string blessing = GetBlessingForTarget(target);
        if (!blessing.empty())
        {
            SetSpellName(blessing);
            return CastSpellAction::isPossible();
        }
    }

    return false;
}

std::string CastBlessingOnPartyAction::GetBlessingForTarget(Unit* target)
{
    std::string chosenBlessing = "";
    if (target)
    {
        std::vector<std::string> possibleBlessings = GetPossibleBlessingsForTarget(target);
        if (!greater)
        {
            // Any already active valid blessing is acceptable: don't swap between alternatives.
            if (HasAnyConfiguredBlessing(target, ai, possibleBlessings, false))
                return "";
        }
        else
        {
            // If a valid greater blessing is already up, do nothing.
            if (HasAnyConfiguredBlessing(target, ai, possibleBlessings, true))
                return "";

            // If a normal blessing is up, only upgrade that same blessing to greater.
            for (const std::string& blessing : possibleBlessings)
            {
                if (!ai->HasAura(blessing, target))
                    continue;

                const std::string greaterBlessing = "greater " + blessing;
                if (ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true))
                    return greaterBlessing;

                return "";
            }
        }

        for (const std::string& blessing : possibleBlessings)
        {
            // Don't cast greater salvation on possible tank classes
            if (greater && blessing == "blessing of salvation" && target->IsPlayer())
            {
                const uint8 playerClass = ((Player*)target)->getClass();
#ifdef MANGOSBOT_TWO
                if (playerClass == CLASS_PALADIN || playerClass == CLASS_WARRIOR || playerClass == CLASS_DRUID || playerClass == CLASS_DEATH_KNIGHT)
#else
                if (playerClass == CLASS_PALADIN || playerClass == CLASS_WARRIOR || playerClass == CLASS_DRUID)
#endif
                {
                    break;
                }
            }

            const std::string greaterBlessing = "greater " + blessing;
            if (!ai->HasAura(blessing, target) && !ai->HasAura(greaterBlessing, target))
            {
                if ((greater && ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true)) ||
                    (!greater && ai->CanCastSpell(blessing, target, 0, nullptr, true)))
                {
                    chosenBlessing = greater ? greaterBlessing : blessing;
                    break;
                }
            }
        }
    }

    return chosenBlessing;
}

std::vector<std::string> GetBlessingsForPlayerRole(Player* player, PlayerbotAI* ai)
{
    if (ai->IsTank(player))
    {
#ifndef MANGOSBOT_TWO
        if (player->getClass() == CLASS_PALADIN)
        {
            return { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light" };
        }
        return { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light", "blessing of wisdom" };
#else
        return { "blessing of sanctuary", "blessing of kings", "blessing of might", "blessing of wisdom" };
#endif 
    }
    else if (ai->IsHeal(player))
    {
#ifndef MANGOSBOT_TWO
        return { "blessing of wisdom", "blessing of kings", "blessing of light", "blessing of sanctuary", "blessing of might" };
#else
        return { "blessing of wisdom", "blessing of kings", "blessing of sanctuary", "blessing of might" };
#endif 
    }
    else if (ai->IsRanged(player))
    {
        if (player->getClass() == CLASS_HUNTER)
        {
#ifndef MANGOSBOT_TWO
            return { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of light", "blessing of sanctuary" };
#else
            return { "blessing of might", "blessing of kings", "blessing of wisdom", "blessing of sanctuary" };
#endif 
        }
#ifndef MANGOSBOT_TWO
        return { "blessing of kings", "blessing of wisdom", "blessing of light", "blessing of sanctuary", "blessing of might" };
#else
        return { "blessing of kings", "blessing of wisdom", "blessing of sanctuary", "blessing of might" };
#endif 
    }
    else
    {
#ifndef MANGOSBOT_TWO
        if (player->getClass() == CLASS_PALADIN)
        {
            return { "blessing of wisdom", "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
        }
#else
        if (player->getClass() == CLASS_WARRIOR)
        {
            return { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of wisdom" };
        }
#endif
#ifndef MANGOSBOT_TWO
        return { "blessing of might", "blessing of kings", "blessing of light", "blessing of wisdom", "blessing of sanctuary" };
#else
        return { "blessing of might", "blessing of kings", "blessing of wisdom", "blessing of sanctuary" };
#endif 
    }
}

std::vector<std::string> GetBlessingsForTarget(Unit* target, PlayerbotAI* ai)
{
    if (target && target->IsPlayer())
    {
        Player* player = static_cast<Player*>(target);
        return GetBlessingsForPlayerRole(player, ai);
    }

    // Blessings for pets or non-players
#ifndef MANGOSBOT_TWO
    return { "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
#else
    return { "blessing of might", "blessing of kings", "blessing of sanctuary" };
#endif 
}

bool ai::HasPossibleBlessingForTarget(Unit* target, PlayerbotAI* ai, bool greater)
{
    if (!target)
        return false;

    const std::vector<std::string> possibleBlessings = GetBlessingsForTarget(target, ai);
    if (!greater)
    {
        return !HasAnyConfiguredBlessing(target, ai, possibleBlessings, false);
    }

    if (HasAnyConfiguredBlessing(target, ai, possibleBlessings, true))
        return false;

    for (const std::string& blessing : possibleBlessings)
    {
        if (!ai->HasAura(blessing, target))
            continue;

        const std::string greaterBlessing = "greater " + blessing;
        return ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true);
    }

    for (const std::string& blessing : possibleBlessings)
    {
        const std::string greaterBlessing = "greater " + blessing;
        if (ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true))
            return true;
    }

    return false;
}

std::vector<std::string> CastPveBlessingAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    return GetBlessingsForTarget(target, ai);
}

std::vector<std::string> CastPvpBlessingAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    return GetBlessingsForTarget(target, ai);
}

std::vector<std::string> CastRaidBlessingAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    return GetBlessingsForTarget(target, ai);
}

std::vector<std::string> CastPveBlessingOnPartyAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    return GetBlessingsForTarget(target, ai);
}

std::vector<std::string> CastPvpBlessingOnPartyAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    return GetBlessingsForTarget(target, ai);
}

std::vector<std::string> CastRaidBlessingOnPartyAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    return GetBlessingsForTarget(target, ai);
}
