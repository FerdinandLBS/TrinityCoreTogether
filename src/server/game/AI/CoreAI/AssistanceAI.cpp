/*
 * Copyright (C) 2008-2019 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AssistanceAI.h"
#include "CombatAI.h"
#include "ConditionMgr.h"
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "FollowMovementGenerator.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <TCTogetherHandler.h>
#include "bot_ai.h"

#define AAI_CASTER_GCD 1500
#define AAI_MELEE_GCD 1000

std::map<unsigned, AssistsAddon> AssistanceAI::assist_addons;

static std::string getCustomGreeting(int entry) {
    std::map<unsigned, AssistsAddon>::iterator it = AssistanceAI::assist_addons.find(entry);
    if (it == AssistanceAI::assist_addons.end())
        return "";
    else {
        unsigned i = rand32() % 3;
        return it->second.getText(i);
    }
}

uint32 GetPorperSpellIdForLevel(uint32 basespell, uint8 lvl)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(basespell);
    if (!info) {
        return 0; //invalid spell id
    }

    uint32 spellId = basespell;

    while (info != nullptr && lvl >= info->BaseLevel)
    {
        spellId = info->Id; //can use this spell
        info = info->GetNextRankSpell(); //check next rank
    }
    return spellId;
}


class NearyByGroupAliveCheck
{
public:
    explicit NearyByGroupAliveCheck(Unit const* unit, float maxdist) : _me(unit), _range(maxdist) { }
    bool operator()(WorldObject const* u) const
    {
        Unit const* unit = u->ToUnit();
        if (!unit || !unit->IsAlive())
            return false;

        if (!_me->IsFriendlyTo(unit))
            return false;

        if (unit->IsHostileTo(_me))
            return false;
        if (unit->IsNeutralToAll())
            return false;
        if (!unit->InSamePhase(_me))
            return false;
        if (!unit->IsWithinDistInMap(_me, _range))
            return false;

        return true;
    }
private:
    Unit const* _me;
    float _range;
    NearyByGroupAliveCheck(NearyByGroupAliveCheck const&);

};

//AoE caster dynobject
class AoePointCheck
{
public:
    explicit AoePointCheck(Unit const* unit, Position l, float maxdist) : _me(unit), _range(maxdist), _l(l) { }
    bool operator()(Unit const* u) const
    {
        if (!u || u == _me)
            return false;
        if (!u->IsAlive())
            return false;
        if (!u->InSamePhase(_me))
            return false;
        if (!u->IsHostileTo(_me) && !u->IsInCombatWith(_me))
            return false;
        if (!u->IsWithinDistInMap(_me, _range))
            return false;
        if (!u->IsInDist(_l, _range))
            return false;

        return true;
    }
private:
    Unit const* _me;
    float _range;
    Position _l;
    AoePointCheck(AoePointCheck const&);
};

Unit* SelectMostHpPctFriedly(Unit* who, float range, bool isCombat) {
    Unit* unit = nullptr;
    Trinity::MostHPPctInRange u_check(who, range, isCombat);
    Trinity::UnitLastSearcher<Trinity::MostHPPctInRange> searcher(who, unit, u_check);
    Cell::VisitAllObjects(who, searcher, range);

    return unit;
}

Unit* SelectLeastHpPctFriendly(Unit* who, float range, bool isCombat) {
    Unit* unit = nullptr;
    Trinity::MostHPMissingPctInRange u_check(who, range, isCombat);
    Trinity::UnitLastSearcher<Trinity::MostHPMissingPctInRange> searcher(who, unit, u_check);
    Cell::VisitAllObjects(who, searcher, range);

    return unit;
}

void UnitAddCriticalRate(Unit* who, int rate) {
    if (!who)
        return;

    CastSpellExtraArgs args;
    args.AddSpellMod(SpellValueMod::SPELLVALUE_BASE_POINT0, rate - 1);
    args.AddSpellMod(SpellValueMod::SPELLVALUE_BASE_POINT1, rate - 1);
    //who->CastSpell(who, 88000, args);
}

void UnitAddHealthPct(Unit* who, int pct) {
    if (!who)
        return;

    CastSpellExtraArgs args;
    args.AddSpellMod(SpellValueMod::SPELLVALUE_BASE_POINT1, pct - 1);
    who->CastSpell(who, 19583, args);
}

//////////////////
// Assistance AI
//////////////////
int32 AssistanceAI::Permissible(Creature const* creature)
{
    // have some hostile factions, it will be selected by IsHostileTo check at MoveInLineOfSight
    if (!creature->IsCivilian() && !creature->IsNeutralToAll())
        return PERMIT_BASE_REACTIVE;
    
    return PERMIT_BASE_NO;
}

void AssistanceAI::stopCombatWith(Unit* victim) {
    if (!victim)
        return;
    if (!victim->IsInCombatWith(me))
        return;

    auto ref = victim->GetCombatManager().GetPvECombatRefs().find(me->GetGUID());
    if (ref != victim->GetCombatManager().GetPvECombatRefs().end()) {
        ref->second->EndCombat();
    }

    Unit* owner = me->GetCharmerOrOwner();
    if (owner && owner->IsInCombatWith(victim) && owner->GetMap()->IsRaid() == false) {
        auto ref = owner->GetCombatManager().GetPvECombatRefs().find(victim->GetGUID());
        if (ref != owner->GetCombatManager().GetPvECombatRefs().end()) {
            ref->second->EndCombat();
        }
    }
}

void BuildDwarfRaceTalentGossip(Player* player, Creature* creature) {
    if (!player || !player->IsAlive() || !creature->IsAlive())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_TALK, "??", MW_GOSSIP_DWARF_TALENT_MAIN, MW_GOSSIP_ACTION_DO + 1);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, creature->GetGUID());
}

// Called when a player opens a gossip dialog with the creature.
bool AssistanceAI::OnGossipHello(Player* player) {
    switch (me->GetEntry()) {
    case 45006:
        BuildHumanRaceTalentGossip(player, me, 0);
        return true;
    case 45008:
    case 45009:
    case 45010:
    case 45011:
    case 45012:
        BuildDwarfRaceTalentGossip(player, me);
        return true;
    }
    return false;
}

// Called when a player selects a gossip item in the creature's gossip menu.
bool AssistanceAI::OnGossipSelect(Player* player, uint32, uint32 gossipListId) {
    uint32 sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
    uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
    switch (sender) {
    case MW_GOSSIP_HUM_TALENT_MAIN:
        UpgradeHumanRaceTalentMinion(player, me, action);
        break;
    case MW_GOSSIP_DWARF_TALENT_MAIN:
        if (action == MW_GOSSIP_ACTION_DO + 1) {
            HireDwarfRaceTalentMinion(player, me, action);
        }
        else {
            BuildDwarfRaceTalentGossip(player, me);
        }
        break;
    }
    return false;
}

// Called when a player selects a gossip with a code in the creature's gossip menu.
bool AssistanceAI::OnGossipSelectCode(Player* /*player*/, uint32 /*menuId*/, uint32 /*gossipListId*/, char const* /*code*/) {
    return false;
}

bool AssistanceAI::canAttackTarget(Unit const* target)
{
    if (!target)
        return false;

    Unit* master = _realowner;
    uint8 followdist = 50;
    float foldist = 36;

    return
        (target->IsAlive() && target->IsVisible() && me->IsValidAttackTarget(target) && target->isTargetableForAttack() &&
            (me->CanSeeOrDetect(target) && target->InSamePhase(me)) &&
            (!master->IsAlive() || target->IsControlledByPlayer() ||
                (target->GetDistance(master) < foldist && me->GetDistance(master) < followdist)) &&//if master is killed pursue to the end
            (target->IsHostileTo(master) || target->IsHostileTo(me) ||//if master is controlled
                (target->GetReactionTo(me) < REP_FRIENDLY && (master->IsInCombat() || target->IsInCombat()))));
}

bool AssistanceAI::IsTargetValid(Unit* target, bool& endCombat) {
    endCombat = false;
    switch (target->GetEntry()) {
    case 36954:
        return false;
    default:
        if (!canAttackTarget(target)) {
            endCombat = true;
            return false;
        }
        return true;
    }
}

Unit* AssistanceAI::getAttackerForHelper(Unit* unit)                 // If someone wants to help, who to give them
{
    bool endCombat;
    Unit* victim;
    if (!unit->IsEngaged())
        return nullptr;

    CombatManager const& mgr = unit->GetCombatManager();
    // pick arbitrary targets; our pvp combat > owner's pvp combat > our pve combat > owner's pve combat
    Unit* owner = unit->GetCharmerOrOwner();
    if (mgr.HasPvECombat()) {
        for (auto const& pair : mgr.GetPvECombatRefs()) {
            if (!pair.second) continue;
            victim = pair.second->GetOther(unit);
            if (IsTargetValid(victim, endCombat)) {
                return victim;
            }
        }
    }
    if (owner && (owner->GetCombatManager().HasPvECombat())) {
        for (auto const& pair : owner->GetCombatManager().GetPvECombatRefs()) {
            if (!pair.second) continue;
            victim = pair.second->GetOther(owner);
            if (IsTargetValid(victim, endCombat)) {
                return victim;
            }
        }
    }
    if (mgr.HasPvPCombat()) {
        return mgr.GetPvPCombatRefs().begin()->second->GetOther(unit);
    }
    if (owner && (owner->GetCombatManager().HasPvPCombat()))
        return owner->GetCombatManager().GetPvPCombatRefs().begin()->second->GetOther(owner);
    return nullptr;
}

Unit* AssistanceAI::SelectNextTarget(bool allowAutoSelect)
{
    (void)allowAutoSelect;

    // Check pet attackers first so we don't drag a bunch of targets to the owner
    if (Unit* myAttacker = getAttackerForHelper(me))
        if (!myAttacker->HasBreakableByDamageCrowdControlAura() && myAttacker != _realowner)
            return myAttacker;

    // Not sure why we wouldn't have an owner but just in case...
    if (!me->GetCharmerOrOwner())
        return nullptr;

    if (Unit* ownerVictim = me->GetCharmerOrOwner()->GetVictim())
        return ownerVictim;

    // Default - no valid targets
    return nullptr;
}

bool AssistanceAI::spellCasted(SpellCastResult result) {
    if (result == SpellCastResult::SPELL_CAST_OK) {
        // Set GCD
        _gcd = (_type == AssistanceAI::ATTACK_TYPE_CASTER ? AAI_CASTER_GCD : AAI_MELEE_GCD);
        return true;
    }
    return false;
}

bool AssistanceAI::isSpellReady(int32 index) {
    if (_gcd > 0)
        return false;

    if (index < 0)
        return true;

    SpellHistory* spellHistory = me->GetSpellHistory();
    if (spellHistory) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(me->m_spells[index]);
        return spellHistory->IsReady(si);
    }
    return true;
};

float AssistanceAI::GetManaPct() {
    return (float)me->GetPower(Powers::POWER_MANA) / (float)me->GetMaxPower(Powers::POWER_MANA);
}

Unit* AssistanceAI::GetVictim() {
    Unit* victim = nullptr;

    if (!me->IsAlive())
    {
        EngagementOver();
        me->SetTarget(ObjectGuid::Empty);
        return nullptr;
    }

    isTargetChanged = false;
    victim = me->GetVictim();
    if (nullptr == victim) {
        victim = SelectNextTarget(false);

        if (victim && canAttackTarget(victim)) {
            isTargetChanged = true;
        }
        else {
            victim = nullptr;
        }
    }

    if (_realowner->IsInCombat()) {
        Unit* ownerTarget = ObjectAccessor::GetUnit(*_realowner, _realowner->GetTarget());
        if (ownerTarget && canAttackTarget(ownerTarget)) {
            // If owner selected one enemy
            if (victim != ownerTarget) {
                victim = ownerTarget;
                isTargetChanged = true;
            }
        }
    }

    if (!victim) {
        me->SetTarget(ObjectGuid::Empty);
    }

    return victim;
}


bool AssistanceAI::checkOneTimeSpells() {
    return false;

}

bool AssistanceAI::checkNoneCombatSpells() {
    std::vector<int32> list = _spells.find(AAI_SPELL_NONE_COMBAT)->second;
    std::vector<int32> remove_list;
    Unit* target = nullptr;

    if (list.empty() || me->HasUnitState(UNIT_STATE_CASTING) || _gcd > 0)
        return false;

    /* Score every spell */
    for (std::vector<int32>::reverse_iterator it = list.rbegin(); it != list.rend(); it++) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(*it);
        _lastSpellResult = SpellCastResult::SPELL_CAST_OK;

        for (auto eff : si->GetEffects()) {
            switch (eff.Effect) {
            case SPELL_EFFECT_SUMMON:
                _lastSpellResult = me->CastSpell(me, si->Id);
                goto exists;
            case SPELL_EFFECT_APPLY_AURA:
            case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
            case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
                
                if (si->IsSelfCast() && si->GetDuration() < 0) {
                    me->CastSpell(me, si->Id, true);
                    remove_list.push_back(si->Id);
                    goto next_loop;
                }
                else {
                    Trinity::UnitAuraCheck check(false, si->Id, me->GetGUID());
                    Trinity::UnitSearcher<Trinity::UnitAuraCheck> searcher(me, target, check);
                    Cell::VisitAllObjects(me, searcher, 40.f);
                    if (target != nullptr) {
                        _lastSpellResult = me->CastSpell(me, si->Id);
                        goto exists;
                    }
                }
                break;
            }
        }
    next_loop:
        continue;
    }
exists:
    if (remove_list.size() > 0) {
        list.erase(std::remove_if(list.begin(), list.end(), [remove_list](int x) {
            for (auto e : remove_list) {
                if (e == x)
                    return true;
            }
            return false;
        }), list.end());
    }

    switch (_lastSpellResult) {
    case SpellCastResult::SPELL_FAILED_LINE_OF_SIGHT:
    case SpellCastResult::SPELL_FAILED_OUT_OF_RANGE:
        if (!me->HasUnitState(UNIT_STATE_CHASE) && isMovable && target && isCaster()) {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(target, 2.0f);
        }
        break;
    default:
        ;
    }
    return spellCasted(_lastSpellResult);
}

bool AssistanceAI::checkHealingSpells() {
    std::vector<int32> list = _spells.find(AAI_SPELL_HEALING)->second;
    int32 current_score = 0;
    SpellInfo const* sel_sp = nullptr;
    Unit* most_badly_hurt;
    float highest_hp_lost = 0;
    float everage_hp_lost = 0;

    if (list.empty() || me->HasUnitState(UNIT_STATE_CASTING) || _gcd > 0)
        return false;

    std::list<Unit*> group_list;
    NearyByGroupAliveCheck check(me, 40.f);
    Trinity::UnitListSearcher<NearyByGroupAliveCheck> searcher(me, group_list, check);
    Cell::VisitAllObjects(me, searcher, 40.f);

    // No one to heal
    if (group_list.size() == 0)
        return false;

    for (auto unit : group_list) {
        float hp_lost = 100.f - unit->GetHealthPct();
        if (hp_lost >= highest_hp_lost) {
            most_badly_hurt = unit;
            highest_hp_lost = hp_lost;
        }
        everage_hp_lost += hp_lost;
    }
    everage_hp_lost /= group_list.size();

    // No one is injured.
    if (everage_hp_lost < 3 && highest_hp_lost <= 5)
        return false;

    /* Score every spell */
    for (auto spellId : list) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(spellId);
        int32 score = 0;
        _lastSpellResult = SpellCastResult::SPELL_CAST_OK;
        int32 power_cost = si->CalcPowerCost(me, SpellSchoolMask::SPELL_SCHOOL_MASK_ALL);
        int32 heal_mount = 0;

        // One spell that we don't have enough power should never be considered.
        if (me->GetPower(si->PowerType) < (uint32)power_cost)
            continue;

        // One spell that is not recovered should never be considered.
        if (me->GetSpellHistory()->HasCooldown(si))
            continue;

        // Try to save ourself firstly
        if (si->IsSelfCast() && me == most_badly_hurt) {
            score = 1;
            sel_sp = si;
            break;
        }

        // We must do something
        if (highest_hp_lost > 25.f)
            score += 5;

        score -= si->CalcCastTime() / 2000;
        score -= power_cost * 5 / me->GetMaxPower(si->PowerType);
        for (auto eff : si->GetEffects()) {
            // AOE of heal
            if (eff.HasRadius() || eff.TargetA.GetTarget() == TARGET_UNIT_TARGET_CHAINHEAL_ALLY) {
                if (highest_hp_lost > 30.f) {
                    if (everage_hp_lost > 10.f)
                        score += 10;
                    else
                        score += 4;
                }
                else {
                    if (everage_hp_lost > 10)
                        score += 5;
                    else
                        score += 1;
                }

                // Balance the radius score
                if (si->RangeEntry->ID == 1) {
                    score += (-4 + (int32)(eff.CalcRadius() / 5.f));
                }
            }

            if (eff.Effect == SPELL_EFFECT_HEAL || eff.Effect == SPELL_EFFECT_HEAL_MAX_HEALTH) {
                heal_mount += eff.CalcValue(me);
            }

            if (eff.IsAura() && eff.ApplyAuraName == AuraType::SPELL_AURA_PERIODIC_HEAL || eff.ApplyAuraName == AuraType::SPELL_AURA_SCHOOL_ABSORB) {
                heal_mount += eff.CalcValue(me);
                if (most_badly_hurt->HasAura(si->Id, me->GetGUID()))
                    score -= 5;
                else
                    score += 5 - (int32)(highest_hp_lost/20);
            }
        }
        int32 heal_score = heal_mount * 15 / me->GetMaxHealth();
        if (heal_score == 0 && heal_mount > 0) heal_score = 1;
        score += heal_score;

        if (score > current_score) {
            sel_sp = si;
            current_score = score;
        }
    }

    /* Nothing to do */
    if (current_score <= 0)
        return false;

    _lastSpellResult = me->CastSpell(most_badly_hurt, sel_sp->Id);
    switch (_lastSpellResult) {
    case SpellCastResult::SPELL_FAILED_LINE_OF_SIGHT:
    case SpellCastResult::SPELL_FAILED_OUT_OF_RANGE:
        if (!me->HasUnitState(UNIT_STATE_CHASE) && isMovable && isCaster()) {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(most_badly_hurt, 2.0f);
        }
        break;
    default:
        ;
    }
    return spellCasted(_lastSpellResult);
}

bool AssistanceAI::checkDamagingSpells() {
    std::vector<int32> list = _spells.find(AAI_SPELL_DAMAGING)->second;
    Unit* victim = GetVictim();
    int32 current_score = 0;
    SpellInfo const* sel_sp = nullptr;
    bool isSummon = false;

    if (victim == nullptr)
        return false;

    if (list.size() == 0 || me->HasUnitState(UNIT_STATE_CASTING) || _gcd > 0)
        return false;

    // We are healer and should focus on heal
    if (_class == HEALER && me->GetPowerPct(me->GetPowerType()) < 50.f)
        return false;

    std::list<Unit*> aoe_list;
    AoePointCheck check(me, victim->GetPosition(), 15.f);
    Trinity::UnitListSearcher<AoePointCheck> searcher(me, aoe_list, check);
    Cell::VisitAllObjects(victim, searcher, 15.f);

    //me->SetFacingToObject(victim);

    /* Score every spell */
    for (auto spellId : list) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(spellId);
        int32 score = 5;
        bool is_summon = false;
        bool aura_checked = false;
        bool aoe_checked = false;
        bool dmg_checked = false;
        _lastSpellResult = SpellCastResult::SPELL_CAST_OK;

        // One spell that we don't have enough power should never be considered.
        if (me->GetPower(si->PowerType) < (uint32)si->CalcPowerCost(me, SpellSchoolMask::SPELL_SCHOOL_MASK_ALL))
            continue;

        // One spell that is not recovered should never be considered.
        if (me->GetSpellHistory()->HasCooldown(si))
            continue;

        score += si->GetRecoveryTime() / 1000;
        score -= si->CalcCastTime() / 1000;
        score -= (si->IsChanneled()?2 :0);
        
        for (auto eff : si->GetEffects()) {
            if (eff.HasRadius() && eff.Effect != SPELL_EFFECT_SUMMON) {
                if (aoe_list.size() > 1) {
                    score += aoe_list.size() * aoe_list.size();

                    // A self cast AOE
                    if (si->RangeEntry->ID == 1) {
                        score -= 3;
                    }
                }
                else {
                    score -= 15;
                }
                aoe_checked = true;
            } else if (eff.IsAura()) {
                if (victim->HasAura(si->Id, me->GetGUID()))
                    score -= 10;
                else
                    score += 8;
                aura_checked = true;
            }
            else if (eff.Effect == SPELL_EFFECT_SCHOOL_DAMAGE || eff.Effect == SPELL_EFFECT_WEAPON_DAMAGE) {
                score += (eff.CalcValue(me)*10 / _realowner->GetMaxHealth());
                dmg_checked = true;
            }

            if (eff.Effect == SPELL_EFFECT_SUMMON) {
                is_summon = true;
            }

            if (eff.Effect == SPELL_EFFECT_POWER_BURN)
                score += 1;
            else if (eff.Effect == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
                score += 3;
            else
                score += 2;

            if (aoe_checked || aura_checked || dmg_checked)
                break;
        }

        if (score > current_score) {
            sel_sp = si;
            current_score = score;
            isSummon = is_summon;
        }
    }

    /* Nothing to do */
    if (current_score <= 0)
        return false;

    CastSpellExtraArgs args;

    if (isSummon) {
        args.SetOriginalCaster(_realowner->GetGUID());
    }

    if (sel_sp->IsSelfCast()) {
        _lastSpellResult = me->CastSpell(me, sel_sp->Id, args);
    }
    else if (sel_sp->Targets == 64) {
        if (aoe_list.size() == 0)
            _lastSpellResult = me->CastSpell(victim->GetPosition(), sel_sp->Id);
        else
            _lastSpellResult = me->CastSpell(aoe_list.front(), sel_sp->Id);
    }
    else
        _lastSpellResult = me->CastSpell(victim, sel_sp->Id, args);

    switch (_lastSpellResult) {
    case SpellCastResult::SPELL_FAILED_LINE_OF_SIGHT:
    case SpellCastResult::SPELL_FAILED_OUT_OF_RANGE:
        if (!me->HasUnitState(UNIT_STATE_CHASE) && isMovable && isCaster()) {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(victim, 2.0f);
        }
        break;
    default:
        ;
    }
    return spellCasted(_lastSpellResult);
}

bool AssistanceAI::checkProtectingSpells() {
    std::vector<int32> list = _spells.find(AAI_SPELL_PROTECTING)->second;

    if (list.size() == 0)
        return false;

    for (auto spellId : list) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(spellId);

        if (me->GetSpellHistory()->HasCooldown(spellId))
            continue;

        for (auto eff : si->GetEffects()) {
            switch (eff.Effect) {
            case SPELL_EFFECT_APPLY_AURA:
                
                if (eff.ApplyAuraName == SPELL_AURA_MANA_SHIELD)
                    if (!me->HasAura(spellId)) {
                        me->CastStop();
                        me->CastSpell(me, spellId);
                        return true;
                    }
                break;
            case SPELL_EFFECT_LEAP:
                me->CastStop();
                me->CastSpell(me, spellId);
                break;
            default:
                ;
            }
        }
    }

    return false;
}

bool AssistanceAI::checkGenPowerSpells() {
    std::vector<int32> list = _spells.find(AAI_SPELL_GEN_POWER)->second;
    Unit* victim = GetVictim();
    SpellInfo const* sel_sp = nullptr;

    if (list.size() == 0 || me->HasUnitState(UNIT_STATE_CASTING) || _gcd > 0)
        return false;

    /* Score every spell */
    for (auto spellId : list) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(spellId);
        _lastSpellResult = SpellCastResult::SPELL_CAST_OK;

        // One spell that we don't have enough power should never be considered.
        if (me->GetPower(si->PowerType) < (uint32)si->CalcPowerCost(me, SpellSchoolMask::SPELL_SCHOOL_MASK_ALL))
            continue;



        // One spell that is not recovered should never be considered.
        if (me->GetSpellHistory()->HasCooldown(si))
            continue;

        if (victim->GetTarget() == me->GetOwner()->GetGUID() || victim->GetTarget() == me->GetGUID() || victim->GetTarget() == _realowner->GetGUID()) {
            sel_sp = si;
            break;
        }
        else if (rand() % 100 < 15) {
            sel_sp = si;
            break;
        }
    }
    if (sel_sp == nullptr) {
        return false;
    }

    CastSpellExtraArgs args;

    if (sel_sp->IsSelfCast()) {
        _lastSpellResult = me->CastSpell(me, sel_sp->Id, args);
    }
    else if (sel_sp->Targets == 64) {
        _lastSpellResult = me->CastSpell(victim->GetPosition(), sel_sp->Id);
    }
    else
        _lastSpellResult = me->CastSpell(victim, sel_sp->Id, args);

    switch (_lastSpellResult) {
    case SpellCastResult::SPELL_FAILED_LINE_OF_SIGHT:
    case SpellCastResult::SPELL_FAILED_OUT_OF_RANGE:
        if (!me->HasUnitState(UNIT_STATE_CHASE) && isMovable && isCaster()) {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(victim, 2.0f);
        }
        break;
    default:
        ;
    }
    return spellCasted(_lastSpellResult);
}

bool AssistanceAI::checkControlSpells() {
    std::vector<int32> list = _spells.find(AAI_SPELL_CONTROL)->second;
    Unit* victim = GetVictim();
    SpellInfo const* sel_sp = nullptr;

    if (victim == nullptr)
        return false;

    if (list.size() == 0 || me->HasUnitState(UNIT_STATE_CASTING) || _gcd > 0)
        return false;

    // We are healer and should focus on heal
    if (_class == HEALER && me->GetPowerPct(me->GetPowerType()) < 50.f)
        return false;

    /* Score every spell */
    for (auto spellId : list) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(spellId);
        _lastSpellResult = SpellCastResult::SPELL_CAST_OK;

        // One spell that we don't have enough power should never be considered.
        if (me->GetPower(si->PowerType) < (uint32)si->CalcPowerCost(me, SpellSchoolMask::SPELL_SCHOOL_MASK_ALL))
            continue;

        // One spell that is not recovered should never be considered.
        if (me->GetSpellHistory()->HasCooldown(si))
            continue;

        if (victim->GetTarget() == me->GetOwner()->GetGUID() || victim->GetTarget() == me->GetGUID() || victim->GetTarget() == _realowner->GetGUID()) {
            sel_sp = si;
            break;
        }
        else if (rand()%100 < 15) {
            sel_sp = si;
            break;
        }
    }
    if (sel_sp == nullptr) {
        return false;
    }

    CastSpellExtraArgs args;

    if (sel_sp->IsSelfCast()) {
        _lastSpellResult = me->CastSpell(me, sel_sp->Id, args);
    }
    else if (sel_sp->Targets == 64) {
        _lastSpellResult = me->CastSpell(victim->GetPosition(), sel_sp->Id);
    }
    else
        _lastSpellResult = me->CastSpell(victim, sel_sp->Id, args);

    switch (_lastSpellResult) {
    case SpellCastResult::SPELL_FAILED_LINE_OF_SIGHT:
    case SpellCastResult::SPELL_FAILED_OUT_OF_RANGE:
        if (!me->HasUnitState(UNIT_STATE_CHASE) && isMovable && isCaster()) {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(victim, 2.0f);
        }
        break;
    default:
        ;
    }
    return spellCasted(_lastSpellResult);
}

void UpdateHumanRaceTalentBuffXP(Creature* me, int changeCount) {
    Aura* aura = me->GetAura(81192);
    AssistanceAI* ai = (AssistanceAI*)me->GetAI();
    int count = aura ? aura->GetStackAmount() : 0;

    if (count >= 100 || ai == nullptr) return;

    count += changeCount;
    if (count >= 100) {
        // Upgrade
        if (me->GetVirtualItemId(0) == 1485) {
            me->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);
            count = 100;
        }
        else {
            Gender gender = me->GetGender();
            if (me->HasNpcFlag(UNIT_NPC_FLAG_GOSSIP))
                me->RemoveNpcFlag(UNIT_NPC_FLAG_GOSSIP);

            switch (((AssistanceAI*)me->GetAI())->GetData(0)) {
            case 0:
                me->SetDisplayId(gender == GENDER_FEMALE ? 31953 : 28560);
                me->SetVirtualItem(0, 29362);
                me->SetVirtualItem(1, 34676);
                ai->AttemptAddProperSpellForLevel(81197);
                ai->AttemptAddProperSpellForLevel(81198);
                ai->AttemptAddProperSpellForLevel(gender == GENDER_FEMALE ? 81195 : 81196);

                UnitAddHealthPct(me, 260);
                break;
            case 1:
                me->SetDisplayId(gender == GENDER_FEMALE ? 26397 : 28149);

                ai->AttemptAddProperSpellForLevel(gender == GENDER_FEMALE ? 81201 : 81203);
                me->SetVirtualItem(0, 28188);
                break;
            case 2:
                me->SetDisplayId(gender == GENDER_FEMALE ? 1495 : 5072);
                ai->AttemptAddProperSpellForLevel(gender == GENDER_FEMALE ? 81207 : 81206);
                me->SetVirtualItem(0, 22394);
                break;
            }

            me->UpdateAllStats();
            me->CastSpell(me, 24312, true);
            me->SetFullHealth();
            me->SetObjectScale(1);
            count = 100;
        }
    }
    if (count <= 0) {
        me->RemoveAura(81192);
    }

    me->SetAuraStack(81192, me, count);
}

void AssistanceAI::KilledUnit(Unit* /*victim*/) {
    switch (me->GetEntry()) {
    case 45006:
        UpdateHumanRaceTalentBuffXP(me, 10);
        break;
    }
}

void AssistanceAI::DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) {
    switch (me->GetEntry()) {
    case 45006:
        UpdateHumanRaceTalentBuffXP(me, 3);
        break;
    }
}

// Called at any Damage from any attacker (before damage apply)
// Note: it for recalculation damage or special reaction at damage
void AssistanceAI::DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) {
    (void)attacker;

    checkProtectingSpells();

    switch (me->GetEntry()) {
    case 45006:
        UpdateHumanRaceTalentBuffXP(me, damage*200/me->GetMaxHealth() + 1);
        break;
    }
}

void AssistanceAI::HealDone(Unit* /*done_to*/, uint32& /*addhealth*/) {
    switch (me->GetEntry()) {
    case 45006:
        UpdateHumanRaceTalentBuffXP(me, 5);
        break;
    }
}

void AssistanceAI::ReadyToDie() {
    switch (me->GetEntry()) {
    case 46002:
        me->CastSpell(_realowner, 87278);
        break;
    }
}

void AssistanceAI::resetLifeTimer(void) {

}

void AssistanceAI::Reborn(uint32 pct) {
    if (me->getDeathState() == ALIVE) {
        return;
    }
    me->setDeathState(ALIVE);
    me->SetHealth(uint32(me->GetMaxHealth() * pct / 100));
    me->SetPower(POWER_MANA, uint32(me->GetMaxPower(POWER_MANA)));
    me->CastSpell(me, 85948);
    //me->m_Events.

    if (me->GetEntry() >= 45000 && me->GetEntry() <= 46000) {
        me->CastSpell(me, 86008);
    }
}

void AssistanceAI::JustDied(Unit* killer) {
    Aura* aura = nullptr;
    (void)killer;
    switch (me->GetEntry()) {
    case 45006:
        aura = me->GetOwner()->GetAura(81400, me->GetGUID());
        if (aura) {
            me->GetOwner()->RemoveOwnedAura(aura);
        }
        me->DespawnOrUnsummon(60s);
        break;
    case 45004:
        me->DespawnOrUnsummon(20s);
        break;
    case 46000:
    case 46030:
        me->DespawnOrUnsummon(5s);
        break;
    }
}

void AssistanceAI::SpellInfoAnalyzeAndInsert(const SpellInfo* si) {
    bool is_damaging = false;
    bool is_healing = false;
    bool is_protecting = false;
    bool is_nonecombat = false;
    bool is_control = false;
    bool is_powergen = false;

    if (!si)
        return;

    std::vector<int32> damaging = _spells.find(AAI_SPELL_DAMAGING)->second;

    for (auto eff : si->_effects) {
        switch (eff.Effect) {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_POWER_BURN:
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
        case SPELL_EFFECT_ADD_COMBO_POINTS:
            is_damaging = true;
            break;
        case SPELL_EFFECT_ATTACK_ME:
            is_control = true;
            break;
        case SPELL_EFFECT_PORTAL_TELEPORT:
        case SPELL_EFFECT_TELEPORT_UNITS:
        case SPELL_EFFECT_JUMP:
        case SPELL_EFFECT_JUMP_DEST:
        case SPELL_EFFECT_LEAP:
        case SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER:
        case SPELL_EFFECT_THREAT:
        case SPELL_AURA_MANA_SHIELD:
            is_protecting = true;
            break;
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
        case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
        case SPELL_EFFECT_PERSISTENT_AREA_AURA:
            switch (eff.ApplyAuraName) {
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_DAMAGE_SHIELD:
                is_healing = true;
                break;
            case SPELL_AURA_SCHOOL_ABSORB:
                if (si->IsSelfCast() && eff.TargetA.GetTarget() != TARGET_UNIT_TARGET_ALLY)
                    is_protecting = true;
                else
                    is_healing = true;
                break;
            case SPELL_AURA_MOD_CONFUSE:
            case SPELL_AURA_MOD_FEAR:
            case SPELL_AURA_MOD_STUN:
            case SPELL_AURA_MOD_ROOT:
            case SPELL_AURA_MOD_SILENCE:
                is_control = true;
                break;
            case SPELL_AURA_PERIODIC_ENERGIZE:
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            case SPELL_AURA_MOD_POWER_REGEN_PERCENT:
                is_powergen = true;
                break;
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PROC_TRIGGER_DAMAGE:
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                is_damaging = true;
                break;
            case SPELL_AURA_PERIODIC_TRIGGER_SPELL: {
                const SpellInfo* trigger_spell = sSpellMgr->GetSpellInfo(eff.TriggerSpell);
                if (trigger_spell) {
                    for (auto tse : trigger_spell->GetEffects()) {
                        if (tse.Effect == SPELL_EFFECT_SCHOOL_DAMAGE) {
                            is_damaging = true;
                            break;
                        }
                    }
                }
                break;
            }
            default:
                is_nonecombat = true;
            }
            break;
        case SPELL_EFFECT_POWER_DRAIN:
        case SPELL_EFFECT_ENERGIZE:
            is_powergen = true;
            break;
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_HEALTH_LEECH:
        case SPELL_EFFECT_HEAL_MAX_HEALTH:
            is_healing = true;
            break;
        case SPELL_EFFECT_ADD_EXTRA_ATTACKS:
        case SPELL_EFFECT_LEARN_SPELL:
        case SPELL_EFFECT_STEALTH:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
            is_nonecombat = true;
            break;
        case SPELL_EFFECT_SUMMON:
            if (si->GetDuration() < 0 || si->GetDuration() > 120000)
                is_nonecombat = true;
            else
                is_damaging = true;
            break;
        default:
            ;
        }
    }

    if (is_healing)
        InsertSpell(AAI_SPELL_HEALING, si->Id);
    else if (is_damaging)
        InsertSpell(AAI_SPELL_DAMAGING, si->Id);
    else if (is_powergen)
        InsertSpell(AAI_SPELL_GEN_POWER, si->Id);
    else if (is_control)
        InsertSpell(AAI_SPELL_CONTROL, si->Id);
    else if (is_protecting)
        InsertSpell(AAI_SPELL_PROTECTING, si->Id);
    else if (is_nonecombat)
        InsertSpell(AAI_SPELL_NONE_COMBAT, si->Id);
}

void AssistanceAI::updateTimer(uint32 diff)
{
    if (!me->IsAssistUnit() || !_realowner || _realowner->IsGameObject())
        return;

    if (me->GetOwner() && me->GetDistance(me->GetOwner()) > 50.0f) {
        me->NearTeleportTo(me->GetOwner()->GetPosition(), true);
        return;
    }

    // Update my level to owner's level
    if (me->GetLevel() < _realowner->GetLevel()) {
        me->SetLevel(_realowner->GetLevel());
        ((Guardian*)me)->InitStatsForLevel(me->GetLevel());
        OnLevelUp();
        return;
    }

    if (isInitailized) {
        _gcd = (_gcd > diff ? _gcd - diff : 0);
        if (_lifeTimer > 0) {
            _lifeTimer -= diff;
            if (_lifeTimer <= 0) {
                ReadyToDie();
            }
        }
    }
    else {
        /* This is the first time of this call. Just do something initialization. */
        //resetLifeTimer();

        /* Iterate all spells and categorize them into different sets. */
        for (uint32 i = 0; i < MAX_CREATURE_SPELL; i++) {
            if (me->m_spells[i]) {
                const SpellInfo* si = sSpellMgr->GetSpellInfo(me->m_spells[i]);

                if (si == nullptr)
                    continue;

                SpellInfoAnalyzeAndInsert(si);
            }
        }
        isInitailized = true;
    }
}

// Unit is idle. Only heal spells can cast
void AssistanceAI::ResetPosition(bool force)
{
    (void)force;

    if (AIFlag == AI_ACTION_FLAG::AI_ACTION_HOLD_POSITION)
        return;

    if (!isMovable) {
        me->StopMoving();
        return;
    }

    // Already reseting position
    if (me->HasUnitState(UNIT_STATE_FOLLOW)) {
        return;
    }

    if (!(me->IsCharmed() || me->IsSummon() || me->IsGuardian())) {
        return;
    }

    me->AttackStop();
    me->SetTarget(ObjectGuid::Empty);

    me->StopMoving();
    me->GetMotionMaster()->Clear();
    me->GetMotionMaster()->MoveFollow(me->GetCharmerOrOwner(), _followDistance, _followAngle);
}

bool AssistanceAI::AddOneTimeSpell(int32 spellId) {
    (void)spellId;
    return false;
}

Creature* AssistanceAI::getOwnerPet() {
    Unit* owner = me->GetCharmerOrOwner();

    if (!owner)
        return nullptr;

    if (owner->ToPlayer()) {
        return owner->ToPlayer()->GetPet();
    }

    // we are npc bot
    /*if (owner->GetEntry() >= 70000) {
        return ((bot_ai*)owner->GetAI())->GetBotsPet();
    }*/

    return nullptr;
}

bool checkGnomeWarlockTelantPets(Creature* currentPet, Creature* summoned) {
    int entry;

    if (!currentPet || !summoned)
        return false;

    entry = summoned->GetEntry();

    return (entry == 46005 && (currentPet->GetEntry() == 17252 || currentPet->GetEntry() == 70505)) ||
        (entry == 46004 && (currentPet->GetEntry() == 1863 || currentPet->GetEntry() == 70503)) ||
        (entry == 46003 && (currentPet->GetEntry() == 417 || currentPet->GetEntry() == 70504));
}

void AssistanceAI::InsertSpell(int type, int spellid) {
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellid);
    if (!info) return;

    for (std::vector<int32>::iterator it = _spells.find(type)->second.begin(); it != _spells.find(type)->second.end(); it++) {
        SpellInfo const* saved_info = sSpellMgr->GetSpellInfo(*it);
        if (!saved_info) continue;

        if (saved_info->IsRankOf(info)) {
            if (info->IsHighRankOf(saved_info)) {
                _spells.find(type)->second.erase(it);
                _spells.find(type)->second.push_back(info->Id);
            }
            return;
        }
    }
    _spells.find(type)->second.push_back(info->Id);
}

bool AssistanceAI::AttemptAddProperSpellForLevel(uint32 basespell) {
    uint8 lvl = me->GetLevel();
    SpellInfo const* final_info = nullptr;
    SpellInfo const* info = sSpellMgr->GetSpellInfo(basespell);
    if (!info) {
        return false; //invalid spell id
    }
    final_info = info;

    uint32 spellId = 0;
    while (info != nullptr && lvl >= info->BaseLevel)
    {
        spellId = info->Id; //can use this spell
        info = info->GetNextRankSpell(); //check next rank
    }
    if (info != final_info && info != nullptr) {
        final_info = info;
    }

    SpellInfoAnalyzeAndInsert(final_info);

    return true;
}

void AssistanceAI::AddSpellWithLevelLimit(int32 spellid, int32 level) {
    if (me->GetLevel() < level)
        return;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellid);
    SpellInfoAnalyzeAndInsert(info);
}

void AssistanceAI::OnLevelUp() {
    switch (me->GetEntry()) {
    case 45004:
        handleUndeadRaceTalent();
        break;
    }
}

void AssistanceAI::handleOrcRaceTalent() {
    switch (rand() % 3) {
    case 0:
        AttemptAddProperSpellForLevel(81182);
        AddOneTimeSpell(56222);
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        AIFlag = AI_ACTION_HIDE;
        break;
    case 1:
        AttemptAddProperSpellForLevel(85984);
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        break;
    case 2:
    default:
        AttemptAddProperSpellForLevel(31643);
        //AddOneTimeSpell(31643);
        break;
    }
}

void AssistanceAI::handleUndeadRaceTalent() {
    int i = 0;
    Player* owner = _realowner->ToPlayer();

    /* Update visual item of skull */
    Item* mainWeap = owner->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    Item* offWeap = owner->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

    if (mainWeap) {
        me->SetVirtualItem(0, mainWeap->GetTemplate()->ItemId);
        Guardian* g = (Guardian*)me;

        g->SetAttackPower(me->GetLevel() * 5 + mainWeap->GetTemplate()->ItemLevel * 4);
        g->SetBonusDamage(g->GetBonusDamage() + mainWeap->GetTemplate()->ItemLevel * 1.5);
    }
    if (offWeap)
        me->SetVirtualItem(1, offWeap->GetTemplate()->ItemId);

    /* Apply all talents that player is using */
    for (auto spell : owner->GetSpellMap()) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(spell.first);
        if (spell.second.active == true && si && si->IsPassive()) {
            me->CastSpell(me, spell.first, true);
        }
    }

    switch (owner->GetClass()) {
    case CLASS_WARRIOR:
        if (!offWeap && !mainWeap) {
            me->SetVirtualItem(0, 1566);
        }
        if (offWeap && offWeap->GetTemplate()->InventoryType == INVTYPE_SHIELD) {
            _class = ASSISTANCE_CLASS::TANK;
            me->CastSpell(me, 71, true);
        }
        else {
            _class = ASSISTANCE_CLASS::DPS;
            me->CastSpell(me, 2457, true);
        }
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_MELEE;
        me->SetPowerType(Powers::POWER_RAGE);

        AddSpellWithLevelLimit(GetPorperSpellIdForLevel(355, me->GetLevel()), 10);
        AddSpellWithLevelLimit(GetPorperSpellIdForLevel(78, me->GetLevel()), 1);
        AddSpellWithLevelLimit(87260, 15);
        if (_class == ASSISTANCE_CLASS::TANK) {
            me->SetAttackTime(WeaponAttackType::BASE_ATTACK, 1500);
            AddSpellWithLevelLimit(46968, 60);
            AddSpellWithLevelLimit(GetPorperSpellIdForLevel(871, me->GetLevel()), 28);
            AddSpellWithLevelLimit(GetPorperSpellIdForLevel(6343, me->GetLevel()), 6);
        }
        else {
            AddSpellWithLevelLimit(GetPorperSpellIdForLevel(12294, me->GetLevel()), 40);
            AddSpellWithLevelLimit(GetPorperSpellIdForLevel(12970, me->GetLevel()), 0);
            AddSpellWithLevelLimit(GetPorperSpellIdForLevel(46924, me->GetLevel()), 60);
            if (mainWeap && mainWeap->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                me->SetAttackTime(WeaponAttackType::BASE_ATTACK, 3100);
            else
                me->SetAttackTime(WeaponAttackType::BASE_ATTACK, 1500);
        }
        break;
    case CLASS_ROGUE:
        _class = ASSISTANCE_CLASS::DPS;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_MELEE;
        me->SetPowerType(Powers::POWER_ENERGY);

        AttemptAddProperSpellForLevel(1784);
        AttemptAddProperSpellForLevel(1752);
        AttemptAddProperSpellForLevel(6774);
        AttemptAddProperSpellForLevel(13705);
        AttemptAddProperSpellForLevel(48594);
        me->CastSpell(me, 21975, true); // Energy limit

        me->SetCanDualWield(true);
        break;
    case CLASS_PRIEST:
        _class = ASSISTANCE_CLASS::HEALER;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        me->SetPowerType(Powers::POWER_MANA);
        //me->SetVirtualItem(0, 3415);
        if (owner->HasSpell(15470)) {
            _class = ASSISTANCE_CLASS::DPS;
            
            AttemptAddProperSpellForLevel(589); // 
            AttemptAddProperSpellForLevel(32379); // 
            AttemptAddProperSpellForLevel(15286); // 
            AttemptAddProperSpellForLevel(15470); // 
        }
        else {
            AttemptAddProperSpellForLevel(139); // Recover
            if (owner->HasSpell(34861))
                i = AttemptAddProperSpellForLevel(34861); // Ring of heal
            else
                i = AttemptAddProperSpellForLevel(17); // Shield
            i = AttemptAddProperSpellForLevel(2054); // Heal
            i = AttemptAddProperSpellForLevel(596); // Pray
        }
        i = AttemptAddProperSpellForLevel(585); // 
        break;
    case CLASS_DEATH_KNIGHT:
        _class = ASSISTANCE_CLASS::DPS;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_MELEE;
        me->SetPowerType(Powers::POWER_RUNIC_POWER);
        //me->SetVirtualItem(0, 3822);
        me->m_spells[0] = GetPorperSpellIdForLevel(47541, me->GetLevel());
        me->m_spells[1] = 81171;
        me->m_spells[2] = 81174;
        me->m_spells[3] = GetPorperSpellIdForLevel(56222, me->GetLevel());
        me->m_spells[4] = 81172;
        me->m_spells[5] = 81173;
        break;
    case CLASS_MAGE:
        _class = ASSISTANCE_CLASS::DPS;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        //me->SetVirtualItem(0, 20724);
        me->SetPowerType(Powers::POWER_MANA);

        if (owner->HasAura(16766)) {
            AttemptAddProperSpellForLevel(116); // // frost bolt
        }
        else if (owner->HasAura(16770)) {
            AttemptAddProperSpellForLevel(5143); // // Arc Missile
        } else
            AttemptAddProperSpellForLevel(133); // // fire bolt

        if (owner->HasSpell(12472)) {
            AttemptAddProperSpellForLevel(10); // // blizzard
        }

        if (owner->HasSpell(11366)) {
            AttemptAddProperSpellForLevel(2136);  // Fire blast
        }

        if (owner->HasSpell(54656)) {
            AttemptAddProperSpellForLevel(1449);  // Arc
            AttemptAddProperSpellForLevel(1953);  // Flash
        }

        if (owner->HasSpell(11426))
            AttemptAddProperSpellForLevel(11426);  // ice shield
        else
            AttemptAddProperSpellForLevel(10193);  // mana shield
        break;
    case CLASS_WARLOCK:
        _class = ASSISTANCE_CLASS::DPS;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        me->SetPowerType(Powers::POWER_MANA);
        if (owner->HasSpell(19028)) { // Check soul bonding
            // Demon Spec
            me->m_spells[i++] = 81104;
            me->m_spells[i++] = GetPorperSpellIdForLevel(5740, me->GetLevel()); // Rain of fire
        } else if (owner->HasSpell(18288)) { // Check improve curse
            // Pain Spec
            me->m_spells[i++] = 81102;
            me->m_spells[i++] = GetPorperSpellIdForLevel(172, me->GetLevel()); // Corruption
        } else { // Default destruction
            // Destrcution Spec
            me->m_spells[i++] = 81100;
            me->m_spells[i++] = GetPorperSpellIdForLevel(30283, me->GetLevel()); // Rage fo shadow
        }
        
        
        me->m_spells[i++] = GetPorperSpellIdForLevel(1490, me->GetLevel()); // Curse of element
        me->m_spells[i++] = GetPorperSpellIdForLevel(686, me->GetLevel()); // Shadowbolt
        break;
    }

}

static void handleUnitEnhancement(Unit* me, Unit* owner) {
    Aura* aura;
    static float dmgBonus = 1.0f;
    static float spBonus = 1.0f;
    if (!owner || !me)
        return;

    switch (me->GetEntry()) {
    case 46003: // Hellhound
    case 46004: // Succubus
    case 46005: // Felguard
    case 46006: // Doomguard
    case 46015: // Eye of Eternity
    case 46016: // Frenzied Priest
        break;
    case 46025: // Imp
        aura = owner->GetAuraOfRankedSpell(18694);
        if (aura) {
            dmgBonus += 0.1f * (aura->GetId() + 1 - 18694);
        }

        aura = owner->GetAuraOfRankedSpell(18769);
        if (aura) {
            dmgBonus += 0.04f * (aura->GetId() + 1 - 18769);
        }

        

        break;
    case 46026: // Voidwalker
        break;
    default: // Do nothing additional
        break;
    }
}

static void handleUnitCritInherited(Unit* me, Unit* owner) {
    int crit = 0;
    Aura* aura;
    uint32 entry;

    if (!owner || !me)
        return;

    if (owner->IsNPCBot()) {
        bot_ai* ai = (bot_ai*)owner->GetAI();

        crit = ai->GetBotCritChance()/2;
    } else if (owner->IsPlayer()) {
        crit = owner->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + 1) / 2 + owner->GetFloatValue(PLAYER_CRIT_PERCENTAGE) / 2;
    }
    else {
        return;
    }

    entry = me->GetEntry();


    if (entry == 46002)

    switch (me->GetEntry()) {
    case 46002:
    case 46003:
    case 46004:
    case 46005:
    case 46006:
    case 46015:
    case 46016:
    case 46025:
    case 46026:
        aura = owner->GetAuraOfRankedSpell(30242);
        if (aura)
            crit += (aura->GetSpellInfo()->GetEffect(EFFECT_0).BasePoints + 1);
        aura = owner->GetAuraOfRankedSpell(54347);
        if (aura)
            crit += (owner->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + 1) * (aura->GetSpellInfo()->GetEffect(EFFECT_0).BasePoints + 1) / 100);

        break;
    }

    UnitAddCriticalRate(me, crit);
    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    me->SetByteValue(UNIT_FIELD_BYTES_2, 1, owner->GetByteValue(UNIT_FIELD_BYTES_2, 1));
}

void AssistanceAI::InitFromDB() {
    std::map<uint32, AssistsAddon>::iterator it = AssistanceAI::assist_addons.find(me->GetEntry());

    if (it != AssistanceAI::assist_addons.end()) {
        it->second.getFollowInfo(_followDistance, _followAngle);
        _effSpell = it->second.getEffSpell();
        _class = static_cast<AssistanceAI::ASSISTANCE_CLASS>(it->second.getClass());
        _type = static_cast<AssistanceAI::ASSISTANCE_ATTACK_TYPE>(it->second.getAttackType());
        _awakeTimer = it->second.getAwakeTime();
        canAttack = !(it->second.hasFlag(AA_FLAG_NO_ATTACK));
        isMovable = !(it->second.hasFlag(AA_FLAG_ROOT));
    }
}

void AssistanceAI::JustAppeared() {
    std::string greeting = getCustomGreeting(me->GetEntry());
    Unit* owner = me->GetOwner();

    if (owner == nullptr) {
        TC_LOG_FATAL("Assistance AI", "An assist must have an owner");
        me->PopAI();
        return;
    }

    Player* p = me->GetOwner()->ToPlayer();
    if (p) {
        _realowner = p;
    }
    else {
        // Here we don't have a player owner, try to find the master's master.
        if (owner->IsNPCBot()) {
            bot_ai* ai = ((bot_ai*)owner->GetAI());
            _realowner = ai->GetBotOwner();
        }
        else {
            Unit* m = me->GetOwner()->GetOwner();
            if (m != nullptr && m->ToPlayer()) {
                _realowner = m->ToPlayer();
            }
            else {
                TC_LOG_FATAL("Assistance AI", "Cannot find a real owner for assistance");
                me->PopAI();
                return;
            }
        }
    }

    if (!greeting.empty())
        me->Say(greeting, Language::LANG_UNIVERSAL, owner);

    handleUnitCritInherited(me, owner);
    handleUnitEnhancement(me, owner);
    InitFromDB();

    /* Special entry handler */
    switch (me->GetEntry()) {
    case 45004:
        handleUndeadRaceTalent();
        break;
    case 45005:
        handleOrcRaceTalent();
        break;
    case 45006: // Human talent
        me->SetPowerType(POWER_HEALTH);
        me->SetGender(me->GetDisplayId() == 3277 ? Gender::GENDER_MALE : Gender::GENDER_FEMALE);
        me->CastSpell(owner, 81400, true);
        owner->CastSpell(me, 81400, true);
        break;
    case 46029:
        me->CastSpell(me, 81107, true);
        break;
    default:
        ;
    }

    if (_type == ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER)
        _returnDistance = 40.f;

    if (_effSpell > 0) {
        me->CastSpell(me, _effSpell, true);
    }

    if (owner) {
        _realowner->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);
        _realowner->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_POWER_TYPE);
    }

    me->StopMoving();
    me->GetMotionMaster()->Clear();
}

inline bool AssistanceAI::isCaster()
{
    return _type == ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
}

void AssistanceAI::EngagementStart(Unit* who) {
    if (who->GetEntry() == 36954) {
        //me->WithT DisengageWithTarget(who);
    }
}

bool AssistanceAI::updateCombatStatus() {
    // Check real owner combat state
    if (!_realowner->IsInCombat()) {
        ResetPosition();
        return false;
    }

    // Check victim alive state
    if (me->HasUnitState(UNIT_STATE_CASTING)) {
        Creature* target = ObjectAccessor::GetCreature(*me, me->GetTarget());
        if (target && !target->IsAlive()) {
            me->CastStop();
            return false;
        } else
            return false;
    }

    // When we check return distance. We need spawn owner
    float dis = me->GetDistance(me->GetOwner());
    if (dis > _returnDistance) {
        ResetPosition();
        TC_LOG_INFO("AAI", "Reset position: out of follow range");
        return false;
    }
    else if (dis > 100) {
        me->NearTeleportTo(me->GetOwner()->GetPosition());
        me->CastSpell(me, 52096, true);
    }

    Unit* victim = GetVictim();
    if (victim) {

        // Here we need to change target
        if (isTargetChanged == true) {
            me->Attack(victim, !isCaster());
            if (!isCaster()) {
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveChase(victim);
                TC_LOG_INFO("AAI", "Target is changed. chase it");
            }
        }
    }
    else {
        ResetPosition();
        return false;
    }

    return true;
}

void AssistanceAI::UpdateAI(uint32 diff)
{
    bool casted = false;

    _updateTimer += diff;
    if (_updateTimer < AAI_DEFAULT_UPDATE_TIMER)
        return;

    // Never take care of free unit
    if (me->GetOwner() == nullptr || me->GetOwnerGUID().IsGameObject())
        return;

    // update spells cool down
    updateTimer(_updateTimer);

    // We are not awaken. Do nothing
    if (_awakeTimer > 0) {
        me->StopMoving();
        _awakeTimer -= _updateTimer;
        if (_awakeTimer <= 0)
            me->SetVisible(true);
        return;
    }
    _updateTimer = 0;

    if (me->HasUnitState(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_CONFUSED))
        return;

    if (!updateCombatStatus()) {
        checkNoneCombatSpells();
        casted = checkHealingSpells();
        return;
    }
    else {
        casted = checkHealingSpells();
        if (!casted)
            casted = checkDamagingSpells();
        if (!casted)
            casted = checkControlSpells();
     }
    if (isCaster()) {
        if (casted == true) {
            // As a caster once we successfully casted one spell. We should stop if we are moving
            me->StopMoving();
            if (me->HasUnitState(UNIT_STATE_CHASE)) {
                me->GetMotionMaster()->Clear();
                me->ClearUnitState(UNIT_STATE_CHASE);
            }
        }
    }
    else {
        DoMeleeAttackIfReady();
    }
}
