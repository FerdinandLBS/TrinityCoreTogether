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

void AddSC_TCTogether_script()
{
    new spell_ud_mage_enhance_aura();
    new spell_deamon_gate_warlock();
}
