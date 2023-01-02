#include "ScriptMgr.h"
#include "Containers.h"
#include "DBCStores.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerAI.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

#include "CombatAI.h"
#include "AssistanceAI.h"
#include "Random.h"
#include "MotionMaster.h"
#include "WorldSession.h"
#include "Pet.h"
#include "DynamicObject.h"

//#include "SpellUtility.h"

//#include "bot_ai.h"

bool IsChanneling(Unit const* u = nullptr) { return u->GetCurrentSpell(CURRENT_CHANNELED_SPELL); }
bool IsCasting(Unit const* u = nullptr) { return (u->HasUnitState(UNIT_STATE_CASTING) || IsChanneling(u) || u->IsNonMeleeSpellCast(false, false, true, false, false)); }

void SummonRandomDemon(Unit* caster, Unit* originCaster) {
    int index = rand() % 6;
    int spell = 81100;

    if (originCaster) {
        CastSpellExtraArgs args;
        if (originCaster)
            args.SetOriginalCaster(originCaster->GetGUID());
        caster->CastSpell(caster, spell + index, args);
    }
    else {
        caster->CastSpell(caster, spell + index);
    }
}

class SpellDelayCastEvent : public BasicEvent
{
public:
    SpellDelayCastEvent(uint32 spellId, Unit* caster, SpellCastTargets targets) {
        this->m_spellId = spellId;
        this->m_caster = caster;
        this->m_targets = targets;
    }
    ~SpellDelayCastEvent() {

    }

    bool Execute(uint64 e_time, uint32 p_time) {
        this->m_caster->CastSpell(m_targets.GetObjectTarget(), m_spellId, true);
        return true;
    };

    void Abort(uint64 e_time) {

    };

    bool IsDeletable() const {
        return true;
    };

protected:
    uint32 m_spellId;
    Unit* m_caster;
    SpellCastTargets m_targets;
};

class SetLevelEvent : public BasicEvent
{
public:
    SetLevelEvent(uint32 cid, Unit* caster) {
        this->m_cid = cid;
        this->m_caster = caster;
    }
    ~SetLevelEvent() {

    }

    bool Execute(uint64 e_time, uint32 p_time) {
        std::list<Creature*> minions;
        m_caster->GetAllMinionsByEntry(minions, this->m_cid);
        if (minions.size()) {
            minions.front()->SetLevel(m_caster->GetLevel());
        }
        return true;
    };

    void Abort(uint64 e_time) {

    };

    bool IsDeletable() const {
        return true;
    };

protected:
    uint32 m_cid;
    Unit* m_caster;
};

class EnterVehicleEvent : public BasicEvent
{
public:
    EnterVehicleEvent(uint32 entry, Unit* caster) {
        this->m_entry = entry;
        this->m_caster = caster;
    }
    ~EnterVehicleEvent() {

    }

    bool Execute(uint64 e_time, uint32 p_time) {
        std::list<Creature*> minions;
        m_caster->GetAllMinionsByEntry(minions, this->m_entry);
        if (minions.size()) {
            m_caster->EnterVehicle(minions.front());
        }
        return true;
    };

    void Abort(uint64 e_time) {

    };

    bool IsDeletable() const {
        return true;
    };

protected:
    uint32 m_entry;
    Unit* m_caster;
}; 

class spell_mini_thor_nuc : public SpellScriptLoader
{
public:
    spell_mini_thor_nuc() : SpellScriptLoader("spell_mini_thor_nuc") { }

    class spell_mini_thor_nuc_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mini_thor_nuc_SpellScript);

        void HandleAfterCast()
        {
            Unit* owner = GetCaster();
            if (!owner)
                return;

            const Spell* spell = GetSpell();
            if (!spell) return;
            owner->m_Events.AddEvent(new SpellDelayCastEvent(81159, owner, spell->m_targets),
                owner->m_Events.CalculateTime(Milliseconds(1300)));
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_mini_thor_nuc_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_mini_thor_nuc_SpellScript();
    }
};

class spell_mini_thor_vehicle : public SpellScriptLoader
{
public:
    spell_mini_thor_vehicle() : SpellScriptLoader("spell_mini_thor_vehicle") { }

    class spell_mini_thor_vehicle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mini_thor_vehicle_SpellScript);

        void HandleAfterCast()
        {
            Unit* owner = GetCaster();
            if (!owner)
                return;

            const Spell* spell = GetSpell();
            if (!spell) return;
            owner->m_Events.AddEvent(new SetLevelEvent(45003, owner),
                owner->m_Events.CalculateTime(Milliseconds(100)));
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_mini_thor_vehicle_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_mini_thor_vehicle_SpellScript();
    }
};

class spell_hellfire_vehicle_warlock : public SpellScriptLoader
{
public:
    spell_hellfire_vehicle_warlock() : SpellScriptLoader("spell_hellfire_vehicle_warlock") { }

    class spell_hellfire_vehicle_warlock_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hellfire_vehicle_warlock_SpellScript);

        void HandleAfterCast()
        {
            Unit* owner = GetCaster();
            if (!owner)
                return;

            const Spell* spell = GetSpell();
            if (!spell) return;
            SpellDelayCastEvent* _spellEvent = new SpellDelayCastEvent(81110, owner, spell->m_targets);
            owner->m_Events.AddEvent(_spellEvent, Milliseconds(1900));
            owner->m_Events.AddEvent(new EnterVehicleEvent(45002, owner), Milliseconds(2200));
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_hellfire_vehicle_warlock_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hellfire_vehicle_warlock_SpellScript();
    }
};

class spell_deamon_gate_warlock : public SpellScriptLoader
{
public:
    spell_deamon_gate_warlock() : SpellScriptLoader("spell_deamon_gate_warlock") { }

    class spell_bloodelf_warlock_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bloodelf_warlock_SpellScript);

        void HandleAfterCast()
        {
            Unit* owner = GetCaster();
            if (!owner)
                return;

            SummonRandomDemon(owner, owner->GetOwner());
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_bloodelf_warlock_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_bloodelf_warlock_SpellScript();
    }
};

class spell_ud_mage_enhance_aura : public SpellScriptLoader
{
public:
    spell_ud_mage_enhance_aura() : SpellScriptLoader("spell_ud_mage_enhance_aura") { }


    class spell_ud_mage_enhance_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ud_mage_enhance_aura_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* u = eventInfo.GetActor();
            if (!u)
                return;

            const Spell* sp = eventInfo.GetProcSpell();
            if (!sp)
                return;

            std::list<Creature*> list;
            u->GetAllMinionsByEntry(list, 46000);
            if (list.size() == 0)
                return;

            for (auto m : list) {
                AssistanceAI* ai = (AssistanceAI*)m->GetAI();
                int cmask = eventInfo.GetSpellInfo()->GetCategory();
                switch (eventInfo.GetDamageInfo()->GetSchoolMask()) {
                case SpellSchoolMask::SPELL_SCHOOL_MASK_FROST:
                    switch (rand() % 5) {
                    case 0:
                        if (m->GetSpellHistory())
                            m->GetSpellHistory()->ResetCooldown(12472);
                        m->CastSpell(m, 12472, true);
                        break;
                    case 1:
                        ai->AddOneTimeSpell(87198);
                        break;
                    default:
                        ai->AddOneTimeSpell(85955);
                        break;
                    }
                    break;
                case SpellSchoolMask::SPELL_SCHOOL_MASK_ARCANE:
                    switch (rand() % 7) {
                    case 0:
                    case 1:
                    case 2:
                        if (ai->GetMe()->GetPower(POWER_MANA)*100 / ai->GetMe()->GetMaxPower(POWER_MANA) < 40) {
                            ai->GetMe()->CastSpell(m, 12051, true);
                        } else
                           ai->AddOneTimeSpell(87199);
                        break;
                    default:
                        ai->AddOneTimeSpell(87197);
                        break;
                    }
                    break;
                case SpellSchoolMask::SPELL_SCHOOL_MASK_FIRE:
                    switch (rand() % 6) {
                    case 0:
                    case 1:
                        ai->AddOneTimeSpell(87220);
                        break;
                    case 2:
                        if (m->GetSpellHistory())
                            m->GetSpellHistory()->ResetCooldown(85872);
                        ai->AddOneTimeSpell(85872);
                        break;
                    default:
                        ai->AddOneTimeSpell(85971);
                        break;
                    }
                    break;
                }
            }
            
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_ud_mage_enhance_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_ud_mage_enhance_aura_AuraScript();
    }
};

class spell_mini_thor_cannon : public SpellScriptLoader
{
public:
    spell_mini_thor_cannon() : SpellScriptLoader("spell_mini_thor_cannon") { }


    class spell_mini_thor_cannon_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mini_thor_cannon_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return true;
        }

        void HandleProc(AuraEffect* aurEff)
        {
            //PreventDefaultAction();
            const SpellInfo* si = GetSpellInfo();
            if (!si) return;

            Unit* caster = GetCaster();
            if (!caster) return;
            Position p = GetDynobjOwner()->GetPosition();
            p.Relocate(p.GetPositionX() + (rand_chance() - 50)/10, p.GetPositionY()+ (rand_chance() - 50) / 10);
            caster->CastSpell(p, 81153, true);
        }

        void Register() override
        {
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_mini_thor_cannon_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mini_thor_cannon_AuraScript();
    }
};

class spell_multiple_trigger_aura : public SpellScriptLoader
{
public:
    spell_multiple_trigger_aura() : SpellScriptLoader("spell_multiple_trigger_aura") { }


    class spell_multiple_trigger_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_multiple_trigger_aura_AuraScript);

        uint32 getRandSpel() {
            return RAND(81120, 81121, 81121, 81122);
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* actor = eventInfo.GetActor();
            const Spell* sp = eventInfo.GetProcSpell();
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            if (!sp || !actor || !dmgInfo || dmgInfo->GetSchoolMask() == SpellSchoolMask::SPELL_SCHOOL_MASK_NONE) return;

            uint32 spellid = getRandSpel();
            if (spellid == 81122) {
                actor->CastSpell(actor, spellid, true);
            }
            else {
                actor->CastSpell(dmgInfo->GetVictim(), spellid, true);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_multiple_trigger_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_multiple_trigger_aura_AuraScript();
    }
};

void AddSC_TCTogether_script()
{
    new spell_ud_mage_enhance_aura();
    new spell_deamon_gate_warlock();
    new spell_hellfire_vehicle_warlock();
    new spell_multiple_trigger_aura();
    new spell_mini_thor_vehicle();
    new spell_mini_thor_nuc();
    new spell_mini_thor_cannon();
}
