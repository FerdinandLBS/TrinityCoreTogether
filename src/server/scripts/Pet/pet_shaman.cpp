/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

/*
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "npc_pet_sha_".
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

enum ShamanSpells
{
    SPELL_SHAMAN_ANGEREDEARTH   = 36213,
    SPELL_SHAMAN_FIREBLAST      = 57984,
    SPELL_SHAMAN_FIRENOVA       = 12470,
    SPELL_SHAMAN_FIRESHIELD     = 13376
};

enum ShamanEvents
{
    // Earth Elemental
    EVENT_SHAMAN_ANGEREDEARTH   = 1,
    // Fire Elemental
    EVENT_SHAMAN_FIRENOVA       = 1,
    EVENT_SHAMAN_FIRESHIELD     = 2,
    EVENT_SHAMAN_FIREBLAST      = 3
};

enum ShamanDruidPartnerForm
{
    SHAMAN_DRUID_FORM_NORMAL,
    SHAMAN_DRUID_FORM_TREE,
    SHAMAN_DRUID_FORM_WILDKIN,
    SHAMAN_DRUID_FORM_CAT,
    SHAMAN_DRUID_FORM_BEAR
};

enum ShamanDruidSpell
{
    SPELL_SHAMAN_DRUID_WILDKIN = 81000,
    SPELL_SHAMAN_DRUID_BEAR  = 81001,
    SPELL_SHAMAN_DRUID_CAT = 81002,
    SPELL_SHAMAN_DRUID_TREE = 81003,
    SPELL_SHAMAN_DRUID_FURY = 81004,
    SPELL_SHAMAN_DRUID_MOONFIRE = 81005,
    SPELL_SHAMAN_DRUID_HEAL = 81006,
    SPELL_SHAMAN_DRUID_REJUV = 81007,
    SPELL_SHAMAN_DRUID_CLAW = 81008,
    SPELL_SHAMAN_DRUID_SPIRITFIRE = 81009,
    SPELL_SHAMAN_DRUID_TAUNT = 81010,
    SPELL_SHAMAN_DRUID_SWEEP = 81011
};

struct npc_pet_shaman_earth_elemental : public ScriptedAI
{
    npc_pet_shaman_earth_elemental(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
        _events.ScheduleEvent(EVENT_SHAMAN_ANGEREDEARTH, 0s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (_events.ExecuteEvent() == EVENT_SHAMAN_ANGEREDEARTH)
        {
            DoCastVictim(SPELL_SHAMAN_ANGEREDEARTH);
            _events.ScheduleEvent(EVENT_SHAMAN_ANGEREDEARTH, 5s, 20s);
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

struct npc_pet_shaman_fire_elemental : public ScriptedAI
{
    npc_pet_shaman_fire_elemental(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
        _events.ScheduleEvent(EVENT_SHAMAN_FIRENOVA, 5s, 20s);
        _events.ScheduleEvent(EVENT_SHAMAN_FIREBLAST, 5s, 20s);
        _events.ScheduleEvent(EVENT_SHAMAN_FIRESHIELD, 0s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SHAMAN_FIRENOVA:
                    DoCastVictim(SPELL_SHAMAN_FIRENOVA);
                    _events.ScheduleEvent(EVENT_SHAMAN_FIRENOVA, 5s, 20s);
                    break;
                case EVENT_SHAMAN_FIRESHIELD:
                    DoCastVictim(SPELL_SHAMAN_FIRESHIELD);
                    _events.ScheduleEvent(EVENT_SHAMAN_FIRESHIELD, 2s);
                    break;
                case EVENT_SHAMAN_FIREBLAST:
                    DoCastVictim(SPELL_SHAMAN_FIREBLAST);
                    _events.ScheduleEvent(EVENT_SHAMAN_FIREBLAST, 5s, 20s);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

struct npc_pet_shaman_druid_partner : public ScriptedAI
{
    npc_pet_shaman_druid_partner(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
    }

    ShamanDruidPartnerForm GetDruidForm() {
        if (me->GetAura(SPELL_SHAMAN_DRUID_WILDKIN))
            return SHAMAN_DRUID_FORM_WILDKIN;
        if (me->GetAura(SPELL_SHAMAN_DRUID_BEAR))
            return SHAMAN_DRUID_FORM_BEAR;
        if (me->GetAura(SPELL_SHAMAN_DRUID_CAT))
            return SHAMAN_DRUID_FORM_CAT;
        if (me->GetAura(SPELL_SHAMAN_DRUID_TREE))
            return SHAMAN_DRUID_FORM_TREE;

        return SHAMAN_DRUID_FORM_NORMAL;
    }

    Unit* SelectLeastHpPctFriendly(Unit* who, float range, bool isCombat) {
        Unit* unit = nullptr;
        Trinity::MostHPMissingPctInRange u_check(who, range, isCombat);
        Trinity::UnitLastSearcher<Trinity::MostHPMissingPctInRange> searcher(who, unit, u_check);
        Cell::VisitAllObjects(who, searcher, range);

        return unit;
    }


    bool UpdateVictim2(ShamanDruidPartnerForm form) {
        if (!IsEngaged())
            return false;

        if (!me->IsAlive())
        {
            EngagementOver();
            return false;
        }

        if (!me->HasReactState(REACT_PASSIVE))
        {
            if (Unit* victim = me->SelectVictim())
                if (victim != me->GetVictim()) {
                    if (form == SHAMAN_DRUID_FORM_WILDKIN)
                        me->Attack(victim, true);
                    else
                        AttackStart(victim);
                }

            return me->GetVictim() != nullptr;
        }
        else if (!me->IsInCombat())
        {
            EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
            return false;
        }
        else if (me->GetVictim())
            me->AttackStop();

        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        ShamanDruidPartnerForm form = GetDruidForm();
        if (form == SHAMAN_DRUID_FORM_TREE) {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            Unit* target = SelectLeastHpPctFriendly(me, 35.f, false);
            if (target) {
                if (target->GetAura(SPELL_SHAMAN_DRUID_REJUV) == nullptr) {
                    if (target->GetHealthPct() < 30.f) {
                        me->CastSpell(target, SPELL_SHAMAN_DRUID_HEAL, false);
                        return;
                    }
                    me->CastSpell(target, SPELL_SHAMAN_DRUID_REJUV, false);
                }
                if (target->GetHealthPct() < 92.f)
                    me->CastSpell(target, SPELL_SHAMAN_DRUID_HEAL, false);
            }
            return;
        }

        if (!UpdateVictim2(form))
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        _events.Update(diff);
        Unit* victim = me->GetVictim();
        switch (form) {
        case SHAMAN_DRUID_FORM_WILDKIN:
            if (victim && victim->GetAura(SPELL_SHAMAN_DRUID_MOONFIRE) == nullptr) {
                DoSpellAttackIfReady(SPELL_SHAMAN_DRUID_MOONFIRE);
            }
            DoSpellAttackIfReady(SPELL_SHAMAN_DRUID_FURY);
            return;
        case SHAMAN_DRUID_FORM_CAT:
            if (victim && victim->GetAura(SPELL_SHAMAN_DRUID_SPIRITFIRE) == nullptr) {
                DoCastVictim(SPELL_SHAMAN_DRUID_SPIRITFIRE);
            }
            DoCastVictim(SPELL_SHAMAN_DRUID_CLAW);
            break;
        case SHAMAN_DRUID_FORM_BEAR:
            DoCastVictim(SPELL_SHAMAN_DRUID_TAUNT);
            DoCastVictim(SPELL_SHAMAN_DRUID_SWEEP);
            break;
        default:
            ;// do nothing
        }
        
        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

void AddSC_shaman_pet_scripts()
{
    RegisterCreatureAI(npc_pet_shaman_earth_elemental);
    RegisterCreatureAI(npc_pet_shaman_fire_elemental);
    RegisterCreatureAI(npc_pet_shaman_druid_partner);
}
