#include "ScriptMgr.h"
#include "Containers.h"
#include "DBCStores.h"
#include "Item.h"
#include "Group.h"
#include "GameObject.h"
#include "GameObjectAI.h"
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
#include "TCTogetherHandler.h"

#include "bot_ai.h"
#include "botmgr.h"
#include <unordered_map>
#include <map>
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
    SpellDelayCastEvent(uint32 spellId, Unit* caster, SpellCastTargets targets, CastSpellExtraArgs args = true) {
        this->m_spellId = spellId;
        this->m_caster = caster;
        this->m_targets = targets;
        this->m_args = args;
    }
    ~SpellDelayCastEvent() {

    }

    bool Execute(uint64, uint32) {
        this->m_caster->CastSpell(m_targets.GetObjectTarget(), m_spellId, m_args);
        return true;
    };

    void Abort(uint64) {

    };

    bool IsDeletable() const {
        return true;
    };

protected:
    uint32 m_spellId;
    Unit* m_caster;
    CastSpellExtraArgs m_args;
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

    bool Execute(uint64, uint32) {
        std::list<Creature*> minions;
        m_caster->GetAllMinionsByEntry(minions, this->m_cid);
        if (minions.size()) {
            minions.front()->SetLevel(m_caster->GetLevel());
        }
        return true;
    };

    void Abort(uint64) {

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

    bool Execute(uint64, uint32) {
        std::list<Creature*> minions;
        m_caster->GetAllMinionsByEntry(minions, this->m_entry);
        if (minions.size()) {
            m_caster->EnterVehicle(minions.front());
        }
        return true;
    };

    void Abort(uint64 e_time) {
        (void)e_time;
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

        void HandleProc(AuraEffect*)
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

        void HandleProc(AuraEffect const*, ProcEventInfo& eventInfo)
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

class spell_human_race_talent_aura : public SpellScriptLoader
{
public:
    spell_human_race_talent_aura() : SpellScriptLoader("spell_human_race_talent_aura") { }


    class spell_human_race_talent_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_human_race_talent_aura_AuraScript);

        void CleanDamageShareLink(Unit* actor) {
            Unit::AuraEffectList list = actor->GetAuraEffectsByType(SPELL_AURA_SPLIT_DAMAGE_PCT);
            for (auto eff : list) {
                Unit* caster = eff->GetCaster();
                if (caster == nullptr || !caster->IsAlive()) {
                    actor->RemoveAura(eff->GetBase());
                }
            }
        }

        void HandleProc(AuraEffect const*)
        {
            PreventDefaultAction();
            
            Unit* actor = GetCaster();

            if (!actor->IsPlayer() || !actor->IsAlive())
                return;

            std::list<Creature*> list;
            Player* player = actor->ToPlayer();
            Group* group = player->GetGroup();
            uint32 count = 0;
            player->GetAllMinionsByEntry(list, 45006);
            count = (group!=nullptr?group->GetMembersCount():0)/2 + 1;
            if (count > 10) count = 10;

            player->SetAuraStack(81208, player, count);
            int n = list.size() - 2;
            CleanDamageShareLink(actor);
            if (n < 0) {
                for (int i = 0; i < -n; i++) {
                    player->CastSpell(player, 81191, true);
                }
            } else {
                for (std::list<Creature*>::const_iterator i = list.begin(); i != list.end() && n > 0; i++, n--) {
                    (*i)->CastSpell(*i, 7791, true);
                    (*i)->DespawnOrUnsummon(Milliseconds(1000));
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_human_race_talent_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_human_race_talent_aura_AuraScript();
    }
};


class spell_gnome_race_talent_aura : public SpellScriptLoader
{
public:
    spell_gnome_race_talent_aura() : SpellScriptLoader("spell_gnome_race_talent_aura") { }


    class spell_gnome_race_talent_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gnome_race_talent_aura_AuraScript);

        void HandleProc(AuraEffect const*)
        {
            PreventDefaultAction();
            Unit* caster = GetCaster();
            if (!caster->IsAlive())
                return;

            uint32 spell = 81252 + rand() % 4;
            caster->SetAuraStack(spell, caster, caster->GetAuraCount(spell) + 1);
            if (caster->GetAura(spell)) {
                caster->GetAura(spell)->RefreshTimers(true);
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_gnome_race_talent_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_gnome_race_talent_aura_AuraScript();
    }
};

class spell_gnome_race_talent_trigger_aura : public SpellScriptLoader
{
public:
    spell_gnome_race_talent_trigger_aura() : SpellScriptLoader("spell_gnome_race_talent_trigger_aura") { }


    class spell_gnome_race_talent_trigger_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gnome_race_talent_trigger_aura_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* actor = eventInfo.GetActor();
            Unit* victim;
            int spell;
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            HealInfo* healInfo = eventInfo.GetHealInfo();

            if (!actor || (!dmgInfo && !healInfo)) return;
            if (dmgInfo) {
                if (dmgInfo->GetAbsorb() + dmgInfo->GetDamage() == 0)
                    return;
            }
            if (healInfo) {
                if (healInfo->GetAbsorb() + healInfo->GetHeal() == 0)
                    return;
            }

            spell = aurEff->GetSpellInfo()->Id;
            switch (spell) {
            case 81252:
                victim = actor->GetVictim();
                if (!victim) {
                    if (actor->GetThreatManager().GetThreatListSize()) {
                        victim = actor->GetThreatManager().GetCurrentVictim();
                    }
                }
                if (!victim) {
                    victim = ObjectAccessor::GetCreature(*actor, actor->GetTarget());
                }
                if (!victim)
                    return;

                if (SpellCastResult::SPELL_CAST_OK != actor->CastSpell(victim, 81256, true))
                    return;
                break;
            case 81253:
                actor->CastSpell(actor, 81257, true);
                break;
            case 81254:
                SpellCastResult spResult;
                if (actor->GetTarget() && actor->IsFriendlyTo(ObjectAccessor::GetCreature(*actor, actor->GetTarget()))) {
                    spResult = actor->CastSpell(ObjectAccessor::GetCreature(*actor, actor->GetTarget()), 81258, true);
                    if (spResult == SpellCastResult::SPELL_CAST_OK) {
                        break;
                    }
                }
                actor->CastSpell(actor, 81258, true);
                break;
            case 81255:
                actor->CastSpell(actor, 81259, true);
                break;
            default:
                return;
            }
            uint32 auraStack = actor->GetAuraCount(spell);
            if (auraStack == 1) {
                actor->RemoveAura(spell);
            }
            else {
                actor->SetAuraStack(spell, actor, auraStack - 1);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_gnome_race_talent_trigger_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_gnome_race_talent_trigger_aura_AuraScript();
    }
};

class spell_bloodelf_race_talent_aura : public SpellScriptLoader
{
public:
    spell_bloodelf_race_talent_aura() : SpellScriptLoader("spell_bloodelf_race_talent_aura") { }


    class spell_bloodelf_race_talent_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_bloodelf_race_talent_aura_AuraScript);

        void HandleProc(AuraEffect const*)
        {
            PreventDefaultAction();
            Unit* caster = GetCaster();
            if (!caster->IsAlive())
                return;

            std::list<Creature*> list;
            caster->GetAllMinionsByEntry(list, 45007);
            if (list.size() == 0) {
                caster->CastSpell(nullptr, 81305);

                caster->GetAllMinionsByEntry(list, 45007);
                if (list.size()) {
                    int count = caster->GetAuraCount(81306);
                    Creature* minion = list.front();

                    minion->SetAuraStack(81301, minion, count);
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_bloodelf_race_talent_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_bloodelf_race_talent_aura_AuraScript();
    }
};

class spell_bloodelf_race_talent_trigger_aura : public SpellScriptLoader
{
public:
    spell_bloodelf_race_talent_trigger_aura() : SpellScriptLoader("spell_bloodelf_race_talent_trigger_aura") { }


    class spell_bloodelf_race_talent_trigger_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_bloodelf_race_talent_trigger_aura_AuraScript);

        void HandleCasterDie(Unit* caster) {
            std::list<Creature*> list;
            Creature* minion;
            Position position = caster->GetPosition();

            caster->GetAllMinionsByEntry(list, 45007);
            if (list.size() == 0)
                return;
            minion = list.front();
            minion->GetMotionMaster()->Clear();
            minion->GetMotionMaster()->MoveCharge(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ());

            uint32 count = minion->GetAuraCount(81301);
            if (count == 0) count = 1;
            minion->CastSpell(minion, 81308, true);
            minion->CastSpell(caster, 81308, true);
            minion->KillSelf();

            CastSpellExtraArgs arg;
            arg.AddSpellBP0(caster->GetLevel() * count);
            arg.SetTriggerFlags(TriggerCastFlags::TRIGGERED_FULL_MASK);
            caster->CastSpell(caster, 81307, arg);
            caster->SetFullPower(caster->GetPowerType());
            caster->RemoveAura(81306);
            Player* player = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
            if (player) {
                player->GetSession()->SendNotification("???????%s????", player->GetName());
            }
;        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            //Unit* actor = eventInfo.GetActor();
            Unit* caster = aurEff->GetCaster();
            uint32 amount = 0;
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            HealInfo* healInfo = eventInfo.GetHealInfo();

            if (!caster || (!dmgInfo && !healInfo)) return;
            if (caster->IsAlive() == false) return;
            if (dmgInfo) {
                if (dmgInfo->GetAbsorb() + dmgInfo->GetDamage() == 0)
                    return;

                amount = dmgInfo->GetDamage() + dmgInfo->GetAbsorb();
            }
            if (healInfo) {
                if (healInfo->GetAbsorb() + healInfo->GetHeal() == 0)
                    return;

                amount = (healInfo->GetHeal() + healInfo->GetAbsorb())*2/3;
            }
            if (dmgInfo && dmgInfo->GetVictim() == caster) {
                if (dmgInfo->GetDamage() > caster->GetHealth()) {
                    dmgInfo->ModifyDamage(-(int)(dmgInfo->GetDamage()));
                    return HandleCasterDie(caster);
                }

                return;
            }

            Creature* minion;
            std::list<Creature*> list;
            caster->GetAllMinionsByEntry(list, 45007);
            if (list.size() == 0)
                return;

            minion = list.front();
            if (!minion->IsAlive())
                return;

            uint32 auraStack = minion->GetAuraCount(81301);
            if (auraStack < 90) {
                minion->CastSpell(minion, 81301, true);
                caster->CastSpell(caster, 81306, true);
               // minion->SetAuraStack(81301, minion, auraStack + 1);
               // caster->SetAuraStack(81306, caster, auraStack + 1);
            }

            if (rand_chance() < (double)(amount * 100 / caster->GetMaxHealth())) {
                AssistanceAI* ai = (AssistanceAI*)minion->GetAI();
                ai->AddOneTimeSpell(81302);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_bloodelf_race_talent_trigger_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_bloodelf_race_talent_trigger_aura_AuraScript();
    }
};

//ARCANE_TORRENT
class spell_bloodelf_arcane_torrent : public SpellScriptLoader
{
public:
    spell_bloodelf_arcane_torrent() : SpellScriptLoader("spell_bloodelf_arcane_torrent") { }

    class spell_bloodelf_arcane_torrent_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bloodelf_arcane_torrent_SpellScript);

        void HandleBeforeCast() {
        }

        void HandleAfterCast()
        {
            std::list<Creature*> list;
            Creature* minion;
            uint32 auraCount = 0;
            Unit* owner = GetCaster();
            if (!owner)
                return;

            owner->GetAllMinionsByEntry(list, 45007);
            if (list.size() == 0)
                return;

            minion = list.front();
            auraCount = minion->GetAuraCount(81301);
            if (auraCount == 0)
                return;

            SpellCastTargets targets;
            CastSpellExtraArgs args;
            targets.SetDst(owner->GetPosition());

            
            args.AddSpellBP0(auraCount / 10 * owner->GetLevel() + owner->GetStat(Stats::STAT_AGILITY)
            + owner->GetStat(Stats::STAT_INTELLECT) + owner->GetStat(Stats::STAT_SPIRIT) + owner->GetStat(Stats::STAT_STAMINA) + owner->GetStat(Stats::STAT_STRENGTH));
            args.SetTriggerFlags(TriggerCastFlags::TRIGGERED_FULL_MASK);
            owner->m_Events.AddEvent(new SpellDelayCastEvent(81308, owner, targets),
                owner->m_Events.CalculateTime(Milliseconds(200)));
            owner->m_Events.AddEvent(new SpellDelayCastEvent(81304, owner, targets, args),
                owner->m_Events.CalculateTime(Milliseconds(300)));
            owner->m_Events.AddEvent(new SpellDelayCastEvent(81308, owner, targets),
                owner->m_Events.CalculateTime(Milliseconds(500)));
            owner->m_Events.AddEvent(new SpellDelayCastEvent(81309, owner, targets, args),
                owner->m_Events.CalculateTime(Milliseconds(600)));
            owner->m_Events.AddEvent(new SpellDelayCastEvent(81308, owner, targets),
                owner->m_Events.CalculateTime(Milliseconds(800)));
            owner->m_Events.AddEvent(new SpellDelayCastEvent(81310, owner, targets, args),
                owner->m_Events.CalculateTime(Milliseconds(900)));

            if ((unsigned)rand() % 100 < auraCount * 2) {
                minion->CastSpell(owner, 81308, true);
                owner->GetSpellHistory()->ResetAllCooldowns();
            }

            minion->SetAuraStack(81301, minion, 1);
            owner->RemoveAura(81306);
        }

        void Register() override
        {
            BeforeCast += SpellCastFn(spell_bloodelf_arcane_torrent_SpellScript::HandleBeforeCast);
            AfterCast += SpellCastFn(spell_bloodelf_arcane_torrent_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_bloodelf_arcane_torrent_SpellScript();
    }
};

class spell_troll_race_talent_trigger_aura : public SpellScriptLoader
{
public:
    spell_troll_race_talent_trigger_aura() : SpellScriptLoader("spell_troll_race_talent_trigger_aura") { }


    class spell_troll_race_talent_trigger_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_troll_race_talent_trigger_aura_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* caster = aurEff->GetCaster();
            uint32 amount = 0;
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            HealInfo* healInfo = eventInfo.GetHealInfo();

            if (!caster || (!dmgInfo && !healInfo)) return;
            if (caster->IsAlive() == false) return;
            if (dmgInfo) {
                if (dmgInfo->GetAbsorb() + dmgInfo->GetDamage() == 0)
                    return;
                if (dmgInfo->GetVictim() == caster)
                    return;
                amount += dmgInfo->GetDamage() * 10 / caster->GetMaxHealth();
            }
            if (healInfo) {
                if (healInfo->GetAbsorb() + healInfo->GetHeal() == 0)
                    return;

                amount += (healInfo->GetAbsorb() + healInfo->GetHeal()) * 5 / caster->GetMaxHealth();
            }

            uint32 auraStack = caster->GetAuraCount(81380);
            auraStack = (((auraStack + amount) <= 100) ? (auraStack + amount) : 100);
            caster->SetAuraStack(81380, caster, auraStack);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_troll_race_talent_trigger_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_troll_race_talent_trigger_aura_AuraScript();
    }
};

//troll_spirit_call
class spell_troll_spirit_call : public SpellScriptLoader
{
public:
    spell_troll_spirit_call() : SpellScriptLoader("spell_troll_spirit_call") { }

    class spell_troll_spirit_call_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_troll_spirit_call_SpellScript);

        void HandleAfterCast()
        {
            uint32 auraCount = 0;
            Spell* sp = GetSpell();
            Unit* owner = GetCaster();
            if (!owner)
                return;

            auraCount = owner->GetAuraCount(81380);

            if (sp->m_spellInfo->Id == 81384) {
                std::list<Creature*> cl;
                owner->GetAllMinionsByEntry(cl, 45002);
                if (cl.size() > 0) {
                    TempSummon* c = cl.front()->ToTempSummon();
                    //c->SetRespawnTime
                }
            }

            owner->RemoveAura(81380);
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_troll_spirit_call_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_troll_spirit_call_SpellScript();
    }
};

class spell_tauren_race_talent_aura : public SpellScriptLoader
{
public:
    spell_tauren_race_talent_aura() : SpellScriptLoader("spell_tauren_race_talent_aura") { }


    class spell_tauren_race_talent_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_tauren_race_talent_aura_AuraScript);

        void HandleProc(AuraEffect const*)
        {
            Unit* caster = GetCaster();
            if (!caster->IsAlive())
                return;

            if (caster->IsInCombat()) {
                caster->RemoveAura(81371);
                return;
            }

            Player* p = caster->ToPlayer();
            if (!p)
                return;

            uint16 skill = p->GetSkillValue(762);
            uint32 stacks = 7 + (skill > 150 ? 150 : skill) / 6;
            Aura* aura = caster->GetAura(81371);
            if (aura) {
                uint8 curr = aura->GetStackAmount();
                if (curr > stacks) {
                    p->SetAuraStack(81371, p, stacks);
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_tauren_race_talent_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_tauren_race_talent_aura_AuraScript();
    }
};

class AuraStackSetEvent : public BasicEvent
{
public:
    AuraStackSetEvent(uint32 spellId, Unit* caster, Unit* targets, uint32 amount) {
        this->m_spellId = spellId;
        this->m_caster = caster;
        this->m_targets = targets;
        this->m_amount = amount;
    }
    ~AuraStackSetEvent() {

    }

    bool Execute(uint64, uint32) {
        if (m_targets->IsAlive())
            this->m_caster->SetAuraStack(m_spellId, m_targets, m_amount);
        return true;
    };

    void Abort(uint64) {

    };

    bool IsDeletable() const {
        return true;
    };

protected:
    uint32 m_spellId;
    Unit* m_caster;
    uint32 m_amount;
    Unit* m_targets;
};

class spell_tauren_race_talent_stack_aura : public SpellScriptLoader
{
public:
    spell_tauren_race_talent_stack_aura() : SpellScriptLoader("spell_tauren_race_talent_stack_aura") { }


    class spell_tauren_race_talent_stack_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_tauren_race_talent_stack_aura_AuraScript);

        void SetStackAmount(uint32 spell, Unit* target, int32 amount, bool isCharged = false) {

            Aura* aura = target->GetAura(spell);

            if (aura) {
                if (isCharged) {
                    aura->SetCharges(aura->GetCharges() + 1);
                } else {
                    aura->m_forceStackDisplay = true;
                    if (amount == -1 && aura->GetStackAmount() == aura->GetSpellInfo()->StackAmount)
                        aura->SetStackAmount(aura->GetSpellInfo()->StackAmount + 1);
                    else if (amount > 0)
                        aura->SetStackAmount(amount);
                }
            }
        }

#define SPELL_CANDIDATE_TARGET_TARGET 0 
#define SPELL_CANDIDATE_TARGET_CASTER 1
#define SPELL_CANDIDATE_TARGET_PET    2

        typedef struct __spell_candidate_st {
            uint32 id;
            uint32 target;
            uint32 amount;
            bool isCharged;
        }spell_candidate_t;

        bool ProcessBuffAndTriggeredBuff(const SpellInfo* procSpell, spell_candidate_t* candidates) {
            bool isAura = false;
            uint32 amount = 0;
            bool isCharged = false;
            uint32 i = 0;

            if (procSpell->ProcCharges != 0) {
                isCharged = true;
            }
            amount = procSpell->StackAmount;
            if (amount == 0 && !isCharged) {
                amount = 2;
            }
            else {
                amount = -1;
            }

            for (auto eff : procSpell->GetEffects()) {
                if (eff.Effect == SPELL_EFFECT_APPLY_AURA) {
                    isAura = true;
                    if (eff.TargetA.GetTarget() == TARGET_UNIT_CASTER || eff.TargetA.GetTarget() == TARGET_UNIT_CASTER_AREA_RAID) {
                        candidates[i].target = SPELL_CANDIDATE_TARGET_CASTER;
                    }
                    else if (eff.TargetA.GetTarget() == TARGET_UNIT_PET) {
                        candidates[i].target = SPELL_CANDIDATE_TARGET_PET;
                    }
                    
                    candidates[i].id = procSpell->Id;
                    candidates[i].amount = amount;
                    candidates[i].isCharged = isCharged;
                    i++;
                }
                else if (eff.Effect == SPELL_EFFECT_TRIGGER_SPELL) {
                    const SpellInfo* triggeredSpell = sSpellMgr->GetSpellInfo(eff.TriggerSpell);
                    if (triggeredSpell) {
                        spell_candidate_t c[3] = { 0 };
                        bool res = ProcessBuffAndTriggeredBuff(triggeredSpell, c);
                        if (res) {
                            isAura = true;
                            candidates[i].id = c[0].id;
                            candidates[i].target = c[0].target;
                            candidates[i].amount = c[0].amount;
                            candidates[i].isCharged = c[0].isCharged;
                            i++;
                        }
                    }  
                }
            }
            return isAura;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* caster = aurEff->GetCaster();
            Unit* target;
            spell_candidate_t spellIds[3] = { 0 };

            if (!caster || caster->IsAlive() == false) return;
            const Spell* spell = eventInfo.GetProcSpell();
            const SpellInfo* procSpell;
            if (!spell) return;
            procSpell = eventInfo.GetSpellInfo();

            if (!ProcessBuffAndTriggeredBuff(procSpell, spellIds))
                return;

            switch (procSpell->Id) {
            case 2825: { // Blood Thirsty
                UnitList list;
                caster->GetPartyMembers(list);
                for (auto u : list) {
                    SetStackAmount(2825, u, 2);
                }
                break;
            }
            case 2687:
                SetStackAmount(29131, caster, 2);
                break;
            default:
                target = spell->m_targets.GetUnitTarget();
                for (auto can : spellIds) {
                    if (can.id) {
                        if (can.target == SPELL_CANDIDATE_TARGET_CASTER) {
                            SetStackAmount(can.id, caster, can.amount, can.isCharged);
                        }
                        else if (can.target == SPELL_CANDIDATE_TARGET_PET) {
                            if (caster->GetPetGUID()) {
                                target = ObjectAccessor::GetCreature(*caster, caster->GetPetGUID());
                                if (target) {
                                    SetStackAmount(can.id, target, can.amount, can.isCharged);
                                }
                            }
                        }
                        else if (target) {
                            SetStackAmount(can.id, target, can.amount, can.isCharged);
                        }
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_tauren_race_talent_stack_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_tauren_race_talent_stack_aura_AuraScript();
    }
};

class dwarf_race_talent_gameobject_script : public GameObjectScript
{
public:
    dwarf_race_talent_gameobject_script() : GameObjectScript("dwarf_race_talent_gameobject_script") { }

    struct dwarf_race_talent_gameobject_scriptAI : public GameObjectAI
    {
        dwarf_race_talent_gameobject_scriptAI(GameObject* go) : GameObjectAI(go) { }

        bool OnGossipHello(Player*) override
        {
            return true;
        }

        void InitializeAI() {
            Reset();
            // Do initialization
            TempSummon* ts[5];

            for (uint32 i = 0; i < 5; i++) {
                ts[i] = me->SummonCreature(45008 + i, me->GetPosition(), TempSummonType::TEMPSUMMON_TIMED_DESPAWN, 60s);
                if (ts[i] != nullptr) {
                    Position p(me->GetPosition());
                    //p.RelocateOffset(Position(i * 5 - 10, i * 5 - 10, 0));
                    ts[i]->GetMotionMaster()->Clear();
                    //ts[i]->MovePositionToFirstCollision(p, 10, i* float(M_PI * 5));
                    //ts[i]->GetMotionMaster()->MovePoint(1003, p);
                    ts[i]->GetMotionMaster()->MoveRandom(10);
                    ts[i]->SetFaction(57);
                    if (i == 4) {
                        ts[i]->SetNpcFlag(UNIT_NPC_FLAG_VENDOR);
                    }
                    else {
                        ts[i]->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);
                    }
                }
            }
        }
        bool OnReportUse(Player* player) {
            if (player->IsInCombat())
                return false;

            BuildDwarfRaceTalentMenu(player, me, 0);
            return false;
        }

    };
    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new dwarf_race_talent_gameobject_scriptAI(go);
    }
};

class spell_bot_auto_format : public SpellScriptLoader
{
public:
    spell_bot_auto_format() : SpellScriptLoader("spell_bot_auto_format") { }

    class spell_bot_auto_format_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bot_auto_format_SpellScript);

        void HandleAfterCast()
        {
            Player* owner = this->GetCaster()->ToPlayer();
            if (!owner || !owner->GetBotMgr())
                return;

            Unit* target = owner->GetSelectedUnit();
            if (!target || target->GetEntry() < 70000) {
                std::unordered_map<ObjectGuid /*guid*/, Creature* /*bot*/>* map = owner->GetBotMgr()->GetBotMap();
                auto iter = map->begin();
                while (iter != map->end()) {
                    ((bot_ai*)iter->second->GetAI())->SetBotCommandState(BOT_COMMAND_FOLLOW);

                    ++iter;
                }
            }
            else {
                if (owner->GetBotMgr()->GetBot(target->GetGUID()))
                    ((bot_ai*)target->GetAI())->SetBotCommandState(BOT_COMMAND_FOLLOW);
                else
                    target->Whisper("???????", Language::LANG_UNIVERSAL, owner, false);
            }
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_bot_auto_format_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_bot_auto_format_SpellScript();
    }
};

class spell_bot_hold_format : public SpellScriptLoader
{
public:
    spell_bot_hold_format() : SpellScriptLoader("spell_bot_hold_format") { }

    class spell_bot_hold_format_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bot_hold_format_SpellScript);

        SpellCastResult checkCastHandler() {
            Player* owner = this->GetCaster()->ToPlayer();
            if (!owner)
                return SPELL_FAILED_BAD_TARGETS;

            return SPELL_CAST_OK;
        }

        void HandleAfterCast()
        {
            Player* owner = this->GetCaster()->ToPlayer();
            if (!owner)
                return;


            Unit* target = owner->GetSelectedUnit();
            Spell* sp = GetSpell();
            if (!sp)
                return;

            Position p;
            bot_ai* ai;
            p = sp->m_targets.GetDstPos()->GetPosition();
            if (target && target->ToCreature() && target->ToCreature()->IsNPCBot()) {
                ai = (bot_ai*)target->GetAI();
                if (ai->GetBotOwner() != owner) {
                    owner->GetSession()->SendAreaTriggerMessage("?????NPCBot");
                    return;
                }
                ((bot_ai*)target->GetAI())->SetBotCommandState(BOT_COMMAND_STAY);
                p.m_positionY += 1.0f;
                target->GetMotionMaster()->MovePoint(owner->GetMapId(), p);
            }
            else {
                std::unordered_map<ObjectGuid /*guid*/, Creature* /*bot*/>* map = owner->GetBotMgr()->GetBotMap();
                auto iter = map->begin();
                int i = 0, round = 1;
                float a = 0, r = 0.5f;
                while (iter != map->end()) {
                    p = sp->m_targets.GetDstPos()->GetPosition();
                    ((bot_ai*)iter->second->GetAI())->SetBotCommandState(BOT_COMMAND_STAY);
                    iter->second->GetMotionMaster()->MovePoint(owner->GetMapId(), p);
                    a = M_PI * 2 / (float)(5 * round) * (float)i;
                    p.m_positionX += sin(a) * r;
                    p.m_positionY += cos(a) * r;
                    ++iter;
                    r = 0.5f * round;
                    if (i >= round * 5) {
                        i = 0; round++; r += 0.5f;
                    }
                    else
                        i++;
                }
            }
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_bot_hold_format_SpellScript::checkCastHandler);
            AfterCast += SpellCastFn(spell_bot_hold_format_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_bot_hold_format_SpellScript();
    }
};

class spell_bot_surroud_format : public SpellScriptLoader
{
public:
    spell_bot_surroud_format() : SpellScriptLoader("spell_bot_surroud_format") { }

    class spell_bot_surroud_format_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bot_surroud_format_SpellScript);

        void HandleAfterCast()
        {
            float followDis = 0.72f;
            float angle = 0;
            float stepAngle;
            int i = 0;
            Player* owner = this->GetCaster()->ToPlayer();
            if (!owner || !owner->GetBotMgr())
                return;

            Unit* target = owner->GetSelectedUnit();
            if (!target) {
                followDis = 0.f;
                target = owner;
            }
            else {
                if (!target->IsHostileTo(owner)) {
                    followDis = 0.3f;
                }
            }

            stepAngle = 2 * M_PI / owner->GetNpcBotsCount();
            std::unordered_map<ObjectGuid /*guid*/, Creature* /*bot*/>* map = owner->GetBotMgr()->GetBotMap();
            auto iter = map->begin();
            while (iter != map->end())
            {
                Position p;

                if (target == owner) {
                    if (i < 4) {
                        stepAngle = (float)M_PI / 2;
                        followDis = 4.f;
                    }
                    else if (i < 12) {
                        stepAngle = (float)M_PI / 4;
                        followDis = 8.f;
                    }
                    else if (i < 28) {
                        stepAngle = (float)M_PI / 8;
                        followDis = 12.f;
                    }
                    else {
                        stepAngle = (float)M_PI / 16;
                        followDis = 16.f;
                    }
                    if (i == 4 || i == 12 || i == 28)
                        angle = 0;
                    iter->second->GetMotionMaster()->MovePoint(owner->GetMapId(), target->GetNearPosition(followDis, angle));
                    angle += stepAngle;
                }
                else {
                    iter->second->GetMotionMaster()->MovePoint(owner->GetMapId(), target->GetNearPosition(owner->GetBotMgr()->GetBotExactAttackRange(), angle));
                    angle += stepAngle;
                }
                ((bot_ai*)iter->second->GetAI())->SetBotCommandState(BOT_COMMAND_STAY);
                i++;
                ++iter;
            }
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_bot_surroud_format_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_bot_surroud_format_SpellScript();
    }
};

int generate_random_item_suffix(int curr, bool is_weapon) {
    int level = abs(curr) / 10 - 10;

    if (level <= 3) {
        if ((rand() % 100) < (40 - level * 10)) {
            level++;
        }

        if (level < 4)
            return -(rand() % 10 + level * 10 + 100);
    }

    // level = 4;
    if (is_weapon) {
        int ids[] = { 3,4,5,6,13,14 };
        return -(190 + ids[rand() % 6]);
    }
    else {
        int ids[] = { 1, 2,7,8,9,10,11, 13, 14 };
        return -(190 + ids[rand() % 9]);
    }
}

class item_weapon_levelup : SpellScriptLoader
{
public:
    item_weapon_levelup() : SpellScriptLoader("item_weapon_levelup") {}

    class item_weapon_levelup_SpellScript : public SpellScript
    {
        PrepareSpellScript(item_weapon_levelup_SpellScript);

        SpellCastResult HandleCheckCast() {
            Spell* spell = GetSpell();
            if (!spell)
                return SpellCastResult::SPELL_FAILED_ERROR;

            Player* player = GetCaster()->ToPlayer();
            Item* weapon = spell->m_targets.GetItemTarget();
            if (!weapon || !player) {
                return  SpellCastResult::SPELL_FAILED_BAD_TARGETS;
            }

            const ItemTemplate* proto = weapon->GetTemplate();
            if (proto->Quality < 3) {
                return  SpellCastResult::SPELL_FAILED_LOWLEVEL;
            }
            if (proto->RandomSuffix < 9001) {
                if (proto->RandomSuffix) {
                    return  SpellCastResult::SPELL_FAILED_ITEM_ALREADY_ENCHANTED;
                }
                else {
                    return  SpellCastResult::SPELL_FAILED_BAD_TARGETS;
                }
            }

            return SpellCastResult::SPELL_CAST_OK;
        }

        void HandleAfterCast() {
            Player* player = GetCaster()->ToPlayer();
            Spell* spell = GetSpell();
            Item* weapon = spell->m_targets.GetItemTarget();
            if (!weapon || !player) {
                return;
            }

            int origin_id = weapon->GetItemRandomPropertyId();
            if (origin_id > -110) origin_id = -110;
            int id = generate_random_item_suffix(origin_id, true);
            while (id == origin_id) id = generate_random_item_suffix(origin_id, true);

            player->ApplyEnchantment(weapon, false);
            weapon->SetItemRandomProperties(id);
            player->ApplyEnchantment(weapon, true);
        }

        void Register() override
        {
            AfterCast += SpellCastFn(item_weapon_levelup_SpellScript::HandleAfterCast);
            OnCheckCast += SpellCheckCastFn(item_weapon_levelup_SpellScript::HandleCheckCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new item_weapon_levelup_SpellScript();
    }
};

class item_armor_levelup : SpellScriptLoader
{
public:
    item_armor_levelup() : SpellScriptLoader("item_armor_levelup") {}

    class item_armor_levelup_SpellScript : public SpellScript
    {
        PrepareSpellScript(item_armor_levelup_SpellScript);

        SpellCastResult HandleCheckCast() {
            Spell* spell = GetSpell();
            if (!spell)
                return SpellCastResult::SPELL_FAILED_ERROR;

            Player* player = GetCaster()->ToPlayer();
            Item* weapon = spell->m_targets.GetItemTarget();
            if (!weapon || !player) {
                return  SpellCastResult::SPELL_FAILED_BAD_TARGETS;
            }

            const ItemTemplate* proto = weapon->GetTemplate();
            if (proto->Quality < 3) {
                return  SpellCastResult::SPELL_FAILED_LOWLEVEL;
            }
            if (proto->RandomSuffix < 9001) {
                if (proto->RandomSuffix) {
                    return  SpellCastResult::SPELL_FAILED_ITEM_ALREADY_ENCHANTED;
                }
                else {
                    return  SpellCastResult::SPELL_FAILED_BAD_TARGETS;
                }
            }

            return SpellCastResult::SPELL_CAST_OK;
        }
        void HandleAfterCast() {
            Player* player = GetCaster()->ToPlayer();
            Spell* spell = GetSpell();
            Item* weapon = spell->m_targets.GetItemTarget();
            if (!weapon || !player) {
                return;
            }

            int origin_id = weapon->GetItemRandomPropertyId();
            if (origin_id > -110) origin_id = -110;
            int id = generate_random_item_suffix(origin_id, false);
            while (id == origin_id) {
                id = generate_random_item_suffix(origin_id, false);
            }

            player->ApplyEnchantment(weapon, false);
            weapon->SetItemRandomProperties(id);
            player->ApplyEnchantment(weapon, true);
        }

        void Register() override
        {
            AfterCast += SpellCastFn(item_armor_levelup_SpellScript::HandleAfterCast);
            OnCheckCast += SpellCheckCastFn(item_armor_levelup_SpellScript::HandleCheckCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new item_armor_levelup_SpellScript();
    }
};

class spell_lvl4_ench_echo_trigger_aura : public SpellScriptLoader
{
public:
    spell_lvl4_ench_echo_trigger_aura() : SpellScriptLoader("spell_lvl4_ench_echo_trigger_aura") { }


    class spell_lvl4_ench_echo_trigger_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lvl4_ench_echo_trigger_aura_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* actor = eventInfo.GetActor();
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            const Spell* sp = eventInfo.GetProcSpell();
            if (!sp || !actor || !dmgInfo || dmgInfo->GetDamage() == 0 || !dmgInfo->GetVictim()) return;

            actor->CastSpell(dmgInfo->GetVictim(), sp->GetSpellInfo()->Id, true);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_lvl4_ench_echo_trigger_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_lvl4_ench_echo_trigger_aura_AuraScript();
    }
};

class spell_lvl4_ench_random_trigger_aura : public SpellScriptLoader
{
public:
    spell_lvl4_ench_random_trigger_aura() : SpellScriptLoader("spell_lvl4_ench_random_trigger_aura") { }


    class spell_lvl4_ench_random_trigger_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lvl4_ench_random_trigger_aura_AuraScript);

        uint32 getRandSpellForSpec(Unit* actor) {
            uint8 clazz;

            if (actor->IsCreature() && actor->ToCreature()->IsNPCBot()) {
                clazz = actor->ToCreature()->GetBotAI()->GetBotClass();
            }
            else if (actor->IsPlayer()) {
                clazz = actor->ToPlayer()->GetClass();
            }
            else {
                return 0;
            }
            switch (clazz) {
            case CLASS_WARRIOR:
                return RAND(2687, 23881, 1719, 11581, 20253, 1680);
            case CLASS_PALADIN:
                return RAND(27173, 10314, 10329, 31884, 27173);
            case CLASS_HUNTER:
                if (rand_chance() > 50.f) // balance for Hunter
                    return 0;
                return RAND(85931, 85932, 85933, 19574, 3045);
            case CLASS_ROGUE:
                return RAND(
                    51723, // Fan of Knives
                    27788, // Armor Energize
                    16511, // Bleeding
                    51723, // Fan of Knives
                    14183, // Precognition
                    16511  // Bleeding
                );
            case CLASS_PRIEST:
                return RAND(
                    15263, // Holy fire lvl 3
                    992,   // Pain lvl 4
                    984,   // punishment. lvl 2
                    34433, // Shadowfiend
                    85906, // summon preist
                    19276, // Devouring Plague. lvl 2
                    10060  // Power Infusion
                );
            case CLASS_DEATH_KNIGHT:
                return RAND(85916, 85909, 85892, 49039, 51411);
            case CLASS_SHAMAN:
                return RAND(10605, 15207, 25423, 25457, 51505);
            case CLASS_MAGE:
                return RAND(
                    837,   // Frostbolt lvl 3
                    8401,  // Firebolt  lvl 6
                    11366, // 
                    12043,
                    12472
                );
            case CLASS_WARLOCK:
                return RAND(
                    81100, // temp summon wild
                    6789,  // death coil. lvl 1 
                    686,   // shadow bolt. lvl 1
                    1106,  // shadow bolt. lvl 5
                    47193, // empower demon.
                    1014,  // curse of pain. lvl 2
                    27243, // seed of corruption. lvl 1
                    6222   // corruption. lvl 2
                );
            case CLASS_DRUID:
                if (actor->HasAura(768)) { // cheetah form
                    return RAND(
                        5180,  // Rage lvl 5
                        62078, // Swipe
                        16857, // Faerie Fire(wild)
                        50213, // Tiger
                        8950   // Starfire lvl 3
                    );
                }
                else if (actor->HasAura(9634)) { // bear form
                    return RAND(
                        8927,  // Moonfire lvl 5
                        16857, // Faerie Fire(wild)
                        48505, // Starfall 
                        9908,  // Swipe
                        8950   // Starfire lvl 3
                    );
                }
                else {
                    return RAND(
                        5180,  // Rage lvl 5
                        8927,  // Moonfire lvl 5
                        8951,  // Starfire lvl 4
                        9852,  // Entangling Roots
                        5180,  // Rage lvl 5
                        8927   // Moonfire lvl 5
                    );
                }
            default:
                return 0;
            }
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* actor = eventInfo.GetActor();
            const Spell* sp = eventInfo.GetProcSpell();
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            if (!sp || !actor || !dmgInfo || dmgInfo->GetSchoolMask() == SpellSchoolMask::SPELL_SCHOOL_MASK_NONE) return;

            int32 id = getRandSpellForSpec(actor);
            const SpellInfo* info = sSpellMgr->GetSpellInfo(id);
            if (info == 0) return;

            if (info->IsSelfCast()) {
                actor->CastSpell(actor, info->Id, true);
            }
            else if (info->IsTargetingArea()) {
                Unit* u = dmgInfo->GetVictim();
                if (u) {
                    actor->CastSpell(u->GetPosition(), info->Id, true);
                }
                else {
                    actor->CastSpell(actor->GetPosition(), info->Id, true);
                }
            }
            else {
                actor->CastSpell(dmgInfo->GetVictim(), info->Id, true);
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_lvl4_ench_random_trigger_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_lvl4_ench_random_trigger_aura_AuraScript();
    }
};

class spell_lvl4_ench_feedback_trigger_aura : public SpellScriptLoader
{
public:
    spell_lvl4_ench_feedback_trigger_aura() : SpellScriptLoader("spell_lvl4_ench_feedback_trigger_aura") { }


    class spell_lvl4_ench_feedback_trigger_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lvl4_ench_feedback_trigger_aura_AuraScript);

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
#define SET_SPELL_ID(a,b,c) {spellid[0]=a;spellid[1]=b;spellid[2]=c;}
            PreventDefaultAction();
            Unit* actor = eventInfo.GetActor();
            const Spell* sp = eventInfo.GetProcSpell();
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            uint8 clazz;
            uint32 power = 0;
            uint32 spellid[3] = {};
            const Creature* creature = actor->ToCreature();
            const Player* player = actor->ToPlayer();
            if (!creature) return;
            if (creature->IsNPCBot()) { 
                clazz = creature->GetBotAI()->GetBotClass();
                power = creature->GetBotAI()->GetBotSpellPower();
            }
            else if (player) {
                clazz = player->GetClass();
                power = player->GetBaseSpellPowerBonus();
            }
            else {
                return;
            }
            if (!dmgInfo)
                return;

            switch (clazz) {
            case CLASS_WARRIOR:
                SET_SPELL_ID(1680, 6343, 772)
                    break;
            case CLASS_PALADIN:
                SET_SPELL_ID(879, 20271, 26573)
                    break;
            case CLASS_HUNTER:
                SET_SPELL_ID(13795, 3044, 19574)
                    break;
            case CLASS_ROGUE:
                SET_SPELL_ID(1752, 51723, 16511)
                    break;
            case CLASS_PRIEST:
                SET_SPELL_ID(585, 589, 15237)
                    break;
            case CLASS_DEATH_KNIGHT:
                SET_SPELL_ID(45477, 47541, 48721)
                    break;
            case CLASS_SHAMAN: 
                SET_SPELL_ID(421, 403, 8050)
                    break;
            case CLASS_MAGE:
                SET_SPELL_ID(116, 133, 1449)
                    break;
            case CLASS_WARLOCK:
                SET_SPELL_ID(686, 172, 30283)
                    break;
            case CLASS_DRUID:
                SET_SPELL_ID(5176, 48505, 16857)
                    break;
            default:
                return;
            }

            int32 val = dmgInfo->GetDamage() / 5 + power / 3;
            CastSpellExtraArgs args(aurEff);
            args.AddSpellMod(SpellValueMod::SPELLVALUE_BASE_POINT0, val);
            args.AddSpellMod(SpellValueMod::SPELLVALUE_BASE_POINT1, val);
            args.AddSpellMod(SpellValueMod::SPELLVALUE_BASE_POINT2, val);
            actor->CastSpell(dmgInfo->GetVictim(), spellid[rand() % 3], args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_lvl4_ench_feedback_trigger_aura_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_lvl4_ench_feedback_trigger_aura_AuraScript();
    }
};

void AddSC_TCTogether_script()
{
    new spell_deamon_gate_warlock();
    new spell_hellfire_vehicle_warlock();
    new spell_multiple_trigger_aura();
    new spell_mini_thor_vehicle(); 
    new spell_mini_thor_nuc();
    new spell_mini_thor_cannon();
    new spell_human_race_talent_aura();
    new spell_gnome_race_talent_aura();
    new spell_gnome_race_talent_trigger_aura();
    new spell_bloodelf_arcane_torrent();
    new spell_bloodelf_race_talent_aura();
    new spell_bloodelf_race_talent_trigger_aura();
    new spell_tauren_race_talent_aura();
    new spell_tauren_race_talent_stack_aura();
    new dwarf_race_talent_gameobject_script();

    new spell_bot_auto_format();
    new spell_bot_hold_format();
    new spell_bot_surroud_format();

    new item_armor_levelup();
    new item_weapon_levelup();
    new spell_lvl4_ench_echo_trigger_aura();
    new spell_lvl4_ench_random_trigger_aura();
    new spell_lvl4_ench_feedback_trigger_aura();
}
