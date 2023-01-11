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
//#include "bot_ai.h"


#pragma execution_character_set("utf-8")

const char* getCustomGreeting(int entry) {
    switch (entry) {
    case 45000:
        return "希望这次你准备了烈酒";
    case 45001:
        return "今天，争议将得到伸张！";
    case 45002:
        return "我，来自虚空";
    case 45003:
        return "我到了，让我们速战速决吧";
    case 45005:
        return "为了巫妖王！";
    case 45006:
        return "为了酋长而战！";
    case 45007:
        return "圣光指引着我";
    case 45008:
        return "你在玩弄危险的力量，凡人！";
    case 45011:
        return "我听到了自然的呼唤...";
    case 45012:
        return "你将感受到传说级别的痛苦";
    case 45013:
        return "什么事？";
    case 45014:
        return "渺小的凡人，何事召唤于我？";
    case 45016:
        return "辛多雷永垂不朽";
    case 46000:
        return "为您效劳";
    case 46030:
        return "我到了，请多多指教";
    }
    return nullptr;
}

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

bool AssistanceAI::canAttackTarget(Unit const* target)
{
    if (!target)
        return false;

    Unit* master = me->GetOwner();

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
    // Check pet attackers first so we don't drag a bunch of targets to the owner
    if (Unit* myAttacker = getAttackerForHelper(me))
        if (!myAttacker->HasBreakableByDamageCrowdControlAura() && myAttacker != me->GetOwner())
            return myAttacker;

    // Not sure why we wouldn't have an owner but just in case...
    if (!me->GetCharmerOrOwner())
        return nullptr;

    if (Unit* ownerVictim = me->GetCharmerOrOwner()->GetVictim())
        return ownerVictim;

    // Default - no valid targets
    return nullptr;
}

bool AssistanceAI::castSpell(WorldObject* target, int32 index) {
    if (index >= 0)
        _lastSpellResult = me->CastSpell(target, me->m_spells[index]);
    else
        _lastSpellResult = me->CastSpell(target, oneTimeSpells[index + MAX_CREATURE_SPELL]);

    if (_lastSpellResult == SpellCastResult::SPELL_CAST_OK) {
        if (index >= 0)
            spellsTimer[index][SPELL_TIMER_CURRENT] = 0;
        else
            oneTimeSpells[index + MAX_CREATURE_SPELL] = 0;
        return true;
    }
    return false;
}

bool AssistanceAI::castSpell(const Position& dest, int32 index) {
    if (index >= 0)
        _lastSpellResult = me->CastSpell(dest, me->m_spells[index]);
    else
        _lastSpellResult = me->CastSpell(dest, oneTimeSpells[index + MAX_CREATURE_SPELL]);

    if (_lastSpellResult == SpellCastResult::SPELL_CAST_OK) {
        if (index >= 0)
            spellsTimer[index][SPELL_TIMER_CURRENT] = 0;
        else
            oneTimeSpells[index + MAX_CREATURE_SPELL] = 0;
        return true;
    }
    return false;
}

bool AssistanceAI::isSpellReady(int32 index) {
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
    Unit* owner = me->GetOwner();
    bool reset = false;

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
            reset = true;
        }
        else {
            victim = nullptr;
        }
    }

    if (owner) {
        if (owner->IsEngaged()) {
            Unit* ownerTarget = ObjectAccessor::GetUnit(*owner, owner->GetTarget());
            if (ownerTarget && canAttackTarget(ownerTarget)) {
                // If owner selected one enemy
                if (victim != ownerTarget) {
                    victim = ownerTarget;
                    reset = true;
                }
            }
        }
    }

    if (!victim) {
        me->SetTarget(ObjectGuid::Empty);
    }
    else if (reset) {
        isTargetChanged = true;
    }

    return victim;
}

bool AssistanceAI::AssistantsSpell(uint32 diff, Unit* victim) {
    uint32 id;
    Unit* owner = me->GetOwner();

    if (me->GetEntry() < 45000) {
        if (me->GetEntry() < 40000)
            return false;
    }

    bool casted = false;

    if (me->HasUnitState(UNIT_STATE_CASTING) || me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return true;

    int32 i = -MAX_CREATURE_SPELL;
redo:
    for (; i < MAX_CREATURE_SPELL && casted == false; i++) {
        const SpellInfo* si = nullptr;

        if (i < 0) {
            if (oneTimeSpells[i + MAX_CREATURE_SPELL] > 0)
                id = oneTimeSpells[i + MAX_CREATURE_SPELL];
            else
                continue;
        }
        else
            id = me->m_spells[i];

        si = sSpellMgr->GetSpellInfo(id);
        _lastSpellResult = SpellCastResult::SPELL_CAST_OK;
        if (si == nullptr)
            return false;

        if (si->IsPassive() || !isSpellReady(i)) {
            continue;
        }

        // Take care of specail AI
        switch (id) {
        case 56222:
        case 355: // Taunt
            if (victim && victim->GetTarget() == me->GetGUID() && _class != ASSISTANCE_CLASS::DPS)
                continue;
            if (owner && owner->GetHealthPct() > 55.f && _class != ASSISTANCE_CLASS::DPS) {
                continue;
            }
            //me->CastSpell(me, 71, true);
            break;
        case 47541: // death coil
            if (me->GetPower(Powers::POWER_RUNIC_POWER) < 40)
                continue;
            
            if (true == castSpell(victim, i))
                goto endloop;

            continue;
        case 78:
            if (me->GetPower(Powers::POWER_RAGE) < 400) {
                continue;
            }
            break;
        case 85923: // Silence
            if (!victim || !victim->HasUnitState(UNIT_STATE_CASTING)) {
                continue;
            }
            break;
        case 85953: // Dark Ceremony
            if (GetManaPct() >= 0.6f) {
                continue;
            }
            {
                Unit* target = SelectMostHpPctFriedly(me, 40, true);
                if (target == nullptr)
                    continue;

                casted = castSpell(target, i);
            }
            goto endloop;
        case 85891: // Spirit of Revenge
        case 87278: // Wild Imp: Master Enhance
            casted = castSpell(me->GetOwner(), i);
            goto endloop;
        default:
            ;
        }

        // heal and buff and no target spell.Effect
        if (si->IsSelfCast() || si->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_SUMMON) {
            if (si->GetEffect(EFFECT_0).IsEffect(SpellEffects::SPELL_EFFECT_APPLY_AURA) ||
                si->GetEffect(EFFECT_0).IsEffect(SPELL_EFFECT_APPLY_AREA_AURA_RAID)) {
                if (me->HasAura(id)) {
                    continue;
                }
            }

            // If we are tank. A self cast spell may be a final cast. We should keep it.
            if (_class == ASSISTANCE_CLASS::TANK && !si->GetEffect(EFFECT_0).IsEffect(SpellEffects::SPELL_EFFECT_SUMMON)) {
                if (!(si->GetRecoveryTime() > 30000 && me->GetHealthPct() <= 30)) {
                    continue;
                }
            }

            if (si->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_TELEPORT_UNITS) {
                casted = castSpell(victim, i);
            }
            else {
                casted = castSpell(me, i);
            }
            break;
        }

        // Range Attack
        if ((si->RangeEntry->ID == 1 && si->GetEffect(EFFECT_0).RadiusEntry) ||
            si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_UNIT_CONE_ENEMY_24) {
            if (victim == nullptr || si->GetEffect(EFFECT_0).RadiusEntry->RadiusMax / 2 <= me->GetDistance(victim))
                continue;

            casted = castSpell(me, i);
            break;
        }

        if (si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_UNIT_TARGET_ENEMY ||
            si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_DEST_TARGET_ENEMY) {
            if (victim == nullptr)
                continue;


            // If we are casting one debuff then check it
            if (si->GetEffect(EFFECT_0).IsEffect(SPELL_EFFECT_APPLY_AURA) && victim->HasAura(id) && !si->GetEffect(EFFECT_1).IsEffect(SPELL_EFFECT_SCHOOL_DAMAGE))
                continue;

            casted = castSpell(victim, i);
            break;
        }

        if ((si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENEMY ||
            si->GetEffect(EFFECT_1).TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENEMY ||
            si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_DEST_DYNOBJ_ENEMY)
            && victim) {
            casted = castSpell(victim->GetPosition(), i);
            break;
        }

        /* We are healers */
        if (si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_UNIT_TARGET_ALLY ||
            si->GetEffect(EFFECT_1).TargetB.GetTarget() == TARGET_UNIT_DEST_AREA_ALLY ||
            si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_UNIT_TARGET_CHAINHEAL_ALLY) {
            //victim->GetThreatManager()

            Unit* owner = me;
            if (me->GetOwner()->ToPlayer() == nullptr)
                owner = me->GetOwner();
            Unit* t = SelectLeastHpPctFriendly(owner, 50.0f, true);
            if (t && t->GetHealthPct() < 95) {
                if (si->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_APPLY_AURA && t->HasAura(si->Id))
                    continue;
                casted = castSpell(t, i);
                break;
            }
            else {
                if (me->GetDistance(me->GetOwner()->GetPosition()) > 15.0f)
                    me->GetMotionMaster()->MoveCloserAndStop(1, me->GetOwner(), 15.0f);
            }
        }
    }
endloop:
    if (casted) {
        if (id == 81171 || id == 81174) {
            me->SetPower(Powers::POWER_RUNIC_POWER, me->GetPower(Powers::POWER_RUNIC_POWER) + 10);
        }
        return true;
    }

    switch (_lastSpellResult) {
    case SpellCastResult::SPELL_FAILED_LINE_OF_SIGHT:
    case SpellCastResult::SPELL_FAILED_OUT_OF_RANGE:
        if (!me->HasUnitState(UNIT_STATE_CHASE) && isMovable && _type == ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER) {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(victim, 2.0f);
        }
        break;
    case SpellCastResult::SPELL_FAILED_NOT_READY: // Additional spell could have this result
    case SpellCastResult::SPELL_FAILED_NO_POWER:
        _lastSpellResult = SpellCastResult::SPELL_CAST_OK;
        i++;
        goto redo;
        break;
    default:
        ;
    }

    return false;
}

float AssistanceAI::getSpecialFollowAngle() {
    uint32 index = 0;
    /*for (uint32 i = 0; i < MAX_CREATURE_SPELL; i++) {
        switch (me->m_spells[i]) {
        case 85990: // sword
            return 0.0f;
        case 86001: // tank
            return static_cast<float>(-M_PI / 4);
        case 85980: // healer
            return static_cast<float>(M_PI / 4);
        case 85970: // mage
            return static_cast<float>(3 * M_PI / 4);
        case 85895: // assistance
            return static_cast<float>(3 * M_PI / 2);
        }
    }*/
    return static_cast<float>((rand() % 32) * M_PI / 16);
}

bool AssistanceAI::hasSpell(uint32 id, uint32& index) {
    for (uint32 i = 0; i < MAX_CREATURE_SPELL; i++) {
        if (id == me->m_spells[i]) {
            index = i;
            return true;
        }
    }
    return false;
}

void AssistanceAI::ReadyToDie() {
    switch (me->GetEntry()) {
    case 46002:
        me->CastSpell(me->GetOwner(), 87278);
        break;
    }
}

void AssistanceAI::resetLifeTimer() {
    switch (me->GetEntry()) {
    case 46002:
        _lifeTimer = 20000;
        break;
    }
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

    AssistantsSpell(0, 0);
}

void AssistanceAI::JustDied(Unit* killer) {
    switch (me->GetEntry()) {
    case 45006:
    {
        uint32 index = 0;
        if (hasSpell(85949, index)) {
            if (spellsTimer[index][SPELL_TIMER_CURRENT] >= spellsTimer[index][SPELL_TIMER_ORIGIN]) {
                Reborn(100);
                spellsTimer[index][SPELL_TIMER_CURRENT] = 0;
                return;
            }
        }
    }
    break;
    case 46000:
    case 46030:
        me->DespawnOrUnsummon(5s);
        break;
    }
}

void AssistanceAI::updateTimer(uint32 diff)
{
    uint32 entry = me->GetEntry();
    if (entry < 40000)
        return;

    if (me->GetOwner() && me->GetOwner()->ToPlayer() && me->GetDistance(me->GetOwner()) > 50.0f) {
        me->NearTeleportTo(me->GetOwner()->GetPosition(), true);
        return;
    }

    // Update my level to owner's level
    if (me->GetLevel() < me->GetOwner()->GetLevel()) {
        me->SetLevel(me->GetOwner()->GetLevel());
        ((Guardian*)me)->InitStatsForLevel(me->GetLevel());
        OnLevelUp();
        return;
    }

    if (timerReady) {
        for (uint32 i = 0; i < MAX_CREATURE_SPELL; i++) {
            if (me->m_spells[i]) {
                if (spellsTimer[i][SPELL_TIMER_CURRENT] < spellsTimer[i][SPELL_TIMER_ORIGIN])
                    spellsTimer[i][SPELL_TIMER_CURRENT] += diff;
            }
            else {
                break;
            }
        }
        if (_lifeTimer > 0) {
            _lifeTimer -= diff;
            if (_lifeTimer <= 0) {
                ReadyToDie();
            }
        }
    }
    else {
        resetLifeTimer();
        for (uint32 i = 0; i < MAX_CREATURE_SPELL; i++) {
            if (me->m_spells[i]) {
                const SpellInfo* si = sSpellMgr->GetSpellInfo(me->m_spells[i]);

                if (si == nullptr)
                    continue;

                auraApplied[i] = false;

                if ((si->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_APPLY_AURA || si->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID || si->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_APPLY_AREA_AURA_OWNER)
                    && auraApplied[i] == false) {
                    if (si->GetRecoveryTime() == 0 &&
                        si->Id != 87278 &&
                        si->Id != 85891) {
                        if (si->GetEffect(EFFECT_0).TargetA.GetTarget() == TARGET_UNIT_MASTER)
                            castSpell(me->GetOwner(), i);
                        else
                            castSpell(me, i);
                        auraApplied[i] = true;
                    }
                }
                spellsTimer[i][SPELL_TIMER_CURRENT] = si->GetRecoveryTime() - si->StartRecoveryTime;
                spellsTimer[i][SPELL_TIMER_ORIGIN] = si->GetRecoveryTime();
            }
        }
        timerReady = true;
    }
}

void AssistanceAI::UseInstanceHealing() {
    for (uint32 i = 0; i < MAX_CREATURE_SPELL; i++) {
        const SpellInfo* si = sSpellMgr->GetSpellInfo(me->m_spells[i]);

        if (si == nullptr || !isSpellReady(i) || si->IsPassive())
            continue;

        if (si->GetEffect(EFFECT_0).Effect != SPELL_EFFECT_HEAL || si->CastTimeEntry->Base > 0)
            continue;

        if (si->RangeEntry->ID == 1 && me->GetHealthPct() < 100.0f) {
            castSpell(me, i);
            return;
        }
        Unit* owner = me;
        if (me->GetOwner()->ToPlayer() == nullptr)
            owner = me->GetOwner();
        Unit* t = SelectLeastHpPctFriendly(owner, 30.0f, false);
        if (t == nullptr)
            continue;

        if (si->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_APPLY_AURA && t->HasAura(si->Id)) {
            continue;
        }

        castSpell(t, i);
        return;
    }
}

Creature* AssistanceAI::GetMe() {
    return me;
}

// Unit is idle. Only heal spells can cast
void AssistanceAI::ResetPosition(bool force)
{
    UseInstanceHealing();

    if (AIFlag == AI_ACTION_FLAG::AI_ACTION_HOLD_POSITION)
        return;

    if (!isMovable) {
        me->StopMoving();
        return;
    }

    if (force || !(me->GetVictim() && me->EnsureVictim()->IsAlive())) {
        //MovementGenerator* mg = me->GetMotionMaster()->;
        if (!(me->IsCharmed() || me->IsSummon() || me->IsGuardian())) {
            return;
        }

        if (!force) {
            me->AttackStop();
            me->SetTarget(ObjectGuid::Empty);
            if (me->HasUnitState(UNIT_STATE_FOLLOW) && isInCombat == false) {
                return;
            }
        }

        me->StopMoving();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveFollow(me->GetCharmerOrOwner(), _followDistance, _followAngle);
        if (!force) {
            isInCombat = false;
        }
    }
}

bool AssistanceAI::AddOneTimeSpell(int32 spellId) {
    for (int i = 0; i < MAX_CREATURE_SPELL; i++) {
        if (oneTimeSpells[i] == 0) {
            oneTimeSpells[i] = spellId;
            return true;
        }
    }

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

void AssistanceAI::AddSpellWithLevelLimit(int32 spellid, int32 level) {
    if (me->GetLevel() < level)
        return;

    for (int i = 0; i < 8; i++) {
        if (me->m_spells[i] == 0) {
            me->m_spells[i] = spellid;
            break;
        }
    }
}

void AssistanceAI::OnLevelUp() {
    switch (me->GetEntry()) {
    case 45004:
        for (int i = 0; i < 8; i++) me->m_spells[i] = 0;
        handleUndeadRaceTalent();
        break;
    }
}

void AssistanceAI::handleUndeadRaceTalent() {

    Player* owner = me->GetOwner() ? me->GetOwner()->ToPlayer() : nullptr;
    if (!owner)
        return;

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

        AddSpellWithLevelLimit(355, 10);
        AddSpellWithLevelLimit(78, 1);
        AddSpellWithLevelLimit(87260, 15);
        if (_class == ASSISTANCE_CLASS::TANK) {
            me->SetAttackTime(WeaponAttackType::BASE_ATTACK, 1500);
            AddSpellWithLevelLimit(46968, 60);
            AddSpellWithLevelLimit(871, 28);
            AddSpellWithLevelLimit(6343, 6);
        }
        else {
            AddSpellWithLevelLimit(12294, 40);
            AddSpellWithLevelLimit(12970, 0);
            AddSpellWithLevelLimit(46924, 60);
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
        //me->SetVirtualItem(0, 3268);
        //me->SetVirtualItem(1, 3268);
        me->m_spells[0] = 1752;
        me->m_spells[1] = 6774;
        me->m_spells[2] = 2669;
        break;
    case CLASS_PRIEST:
        _class = ASSISTANCE_CLASS::HEALER;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        //me->SetVirtualItem(0, 3415);
        me->SetPowerType(Powers::POWER_MANA);
        break;
    case CLASS_DEATH_KNIGHT:
        _class = ASSISTANCE_CLASS::DPS;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_MELEE;
        me->SetPowerType(Powers::POWER_RUNIC_POWER);
        //me->SetVirtualItem(0, 3822);
        me->m_spells[0] = 47541;
        me->m_spells[1] = 81171;
        me->m_spells[2] = 81174;
        me->m_spells[3] = 56222;
        me->m_spells[4] = 81172;
        me->m_spells[5] = 81173;
        break;
    case CLASS_MAGE:
        _class = ASSISTANCE_CLASS::DPS;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        //me->SetVirtualItem(0, 20724);
        me->SetPowerType(Powers::POWER_MANA);
        me->m_spells[0] = 120;
        me->m_spells[1] = 133;
        me->m_spells[2] = 10193;
        break;
    case CLASS_WARLOCK:
        _class = ASSISTANCE_CLASS::DPS;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        //me->SetVirtualItem(0, 2549);
        me->SetPowerType(Powers::POWER_MANA);
        me->m_spells[0] = 172;
        me->m_spells[1] = 348;
        me->m_spells[2] = 5720;
        break;
    }

    owner->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);
}

void AssistanceAI::JustAppeared() {
    uint32 effSpell = 0;
    bool playerOwner = false;
    const char* greeting = getCustomGreeting(me->GetEntry());
    Player* owner = me->GetOwner() ? me->GetOwner()->ToPlayer() : nullptr;

    if (greeting)
        me->Say(greeting, Language::LANG_UNIVERSAL, me->GetOwner());

    _followAngle = getSpecialFollowAngle();
    _followDistance = 1.2f;

    updateTimer(0);

    if (!owner) {
        Unit* o = me->GetCharmerOrOwner();

        for (int i = 0; i < 3; i++) {
            if (o) {
                if (o->ToPlayer()) {
                    owner = o->ToPlayer();
                    break;
                }
                o = o->GetCharmerOrOwner();
            } else
                break;
        }
    }
    else
        playerOwner = true;

    if (owner) {
        int crit = 0;
        Aura* aura;
        if (playerOwner)
            crit = owner->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + 1) / 2 + owner->GetFloatValue(PLAYER_CRIT_PERCENTAGE) / 2;

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
                crit += (owner->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + 1)* (aura->GetSpellInfo()->GetEffect(EFFECT_0).BasePoints + 1)/100);
            
            break;
        }

        UnitAddCriticalRate(me, crit);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        me->SetByteValue(UNIT_FIELD_BYTES_2, 1, owner->GetByteValue(UNIT_FIELD_BYTES_2, 1));
    }

    _class = ASSISTANCE_CLASS::NONE;
    _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_MELEE;


    // Set class and attack type
    switch (me->GetEntry()) {
    case 46002: // Wild Ghost
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        if (owner && owner->GetRace() == Races::RACE_BLOODELF) {

        }
        break;
    case 46016:
        me->SetCanDualWield(true);
        break;
    case 45004:
        handleUndeadRaceTalent();
        break;
    case 44000:
    case 45003:
    case 45005:
    case 45009:
    case 45016:
    case 46000:
    case 46001:
    case 46006:
    case 46009:
    case 46011:
    case 46012:
    case 46015:
    case 46028:
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        break;
    case 46018:
        _awakeTimer = 1500;
        effSpell = 85907;
        break;
    case 46032:
    case 46023: // Hellfire
        me->SetVisible(false);
        _awakeTimer = 1900;
        break;
    case 46003:
    case 46004:
    case 46005: // Warlock pets. Check gnome talent
        if (me->GetCharmerOrOwnerOrSelf()->HasAura(87253))
            if (Creature* pet = getOwnerPet()) {
                uint32 entry = me->GetEntry();
                if (pet && checkGnomeWarlockTelantPets(pet, me)) {
                    effSpell = 87285;
                    //me->ToTempSummon()->m_timer = (uint32)-1;
                    //me->ToTempSummon()->m_lifetime = (uint32)-1;
                    _followDistance = 1;
                    _followAngle = float(M_PI * 3 / 2);
                }
            }
        break;
    case 46025:
    case 46026:
        effSpell = 87285;
        //me->ToTempSummon()->GetTimer()= (uint32)-1;
        //me->ToTempSummon()->m_lifetime = (uint32)-1;
        _followDistance = 1;
        _followAngle = float(M_PI * 3 / 2);
        if (me->GetEntry() == 46025)
            _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        break;
    case 46020:
    case 46021:
    case 46019:
        _awakeTimer = 1500;
        effSpell = 85907;
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        break;
    case 46030:
        effSpell = 85907;
        _followDistance = 0.5f;
        _followAngle = float(M_PI * 5 / 4);
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        break;
    //case 46007:
    case 46029: // Demon Portal
        isMovable = false;
        me->CastSpell(me, 81108, true);
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        break;
    case 46017:
        _awakeTimer = 4000;
        break;
    case 46024:
        effSpell = 89001;
        me->AddUnitMovementFlag(MOVEMENTFLAG_HOVER);
        //me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SWIMMING);
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        break;
    case 46031:
        effSpell = 89001;
        me->AddUnitMovementFlag(MOVEMENTFLAG_HOVER);
        _followDistance = 0.1f;
        me->SetFloatValue(UNIT_FIELD_HOVERHEIGHT, 4);
        me->CastSpell(me, 72585);
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        canAttack = false;
        break;
    default:
        _type = ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_MELEE;
    }

    if (effSpell > 0) {
        me->CastSpell(me, effSpell);
    }
    AssistantsSpell(0, nullptr);

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

void AssistanceAI::UpdateAI(uint32 diff/*diff*/)
{
    if (_updateTimer > diff) {
        _updateTimer -= diff;
        return;
    }
    else {
        _updateTimer = AAI_DEFAULT_UPDATE_TIMER;
    }

    // We are not awaken. Do nothing
    if (_awakeTimer > 0) {
        me->StopMoving();
        _awakeTimer -= diff;
        if (_awakeTimer <= 0) {
            me->SetVisible(true);
        }
        return;
    }

    if (!canAttack)
        return ResetPosition();

    // update spells cool down
    updateTimer(diff);

    bool needJoinCombat = false;
    // we are not in combat. return
    if (me->GetOwner()) {
        if (!me->GetOwner()->IsInCombat()) {
            return ResetPosition();
        }
        needJoinCombat = true;
    }
    if (!me->IsInCombat() && !needJoinCombat)
        return ResetPosition();

    if (me->HasUnitState(UNIT_STATE_CASTING)) {
        return;
    }

    Unit* victim = GetVictim();
    if (victim) {
        if (victim->GetDistance(me->GetOwner()) > 50) {
            me->AttackStop();
            return ResetPosition();
        }

        if (!isInCombat) {
            isInCombat = true;
            me->StopMoving();
            me->GetMotionMaster()->Clear();
        }

        if (isTargetChanged == true) {
            me->Attack(victim, !isCaster());
            if (!isCaster()) {
                if (AIFlag == AI_ACTION_FLAG::AI_ACTION_NONE || isTargetChanged)
                    me->GetMotionMaster()->MoveChase(victim);
            }
        }
    }
    else {
        return ResetPosition();
    }

    bool res = AssistantsSpell(diff, victim);

    if (isCaster()) {
        if (res == true) {
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
