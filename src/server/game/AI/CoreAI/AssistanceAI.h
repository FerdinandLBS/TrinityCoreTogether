#ifndef ASSISTANCE_AI_H
#define ASSISTANCE_AI_H

#include "CreatureAI.h"

class Creature;

#define MAX_CREATURE_SPELL 8
#define MAX_TIMER_TYPE 2
#define ASSIST_GCD 1050
#define AAI_DEFAULT_UPDATE_TIMER 100

#define ADATA_ID_HUMAN_TALENT 0
#define ADATA_ID_STAT_VALUE_STR 1
#define ADATA_ID_STAT_VALUE_AGI 2
#define ADATA_ID_STAT_VALUE_STA 3
#define ADATA_ID_STAT_VALUE_INT 4
#define ADATA_ID_STAT_VALUE_SPI 5
#define ADATA_ID_STAT_PCT_STR 6
#define ADATA_ID_STAT_PCT_AGI 7
#define ADATA_ID_STAT_PCT_STA 8
#define ADATA_ID_STAT_PCT_INT 9
#define ADATA_ID_STAT_PCT_SPI 10

#define AA_FLAG_NONE      0x00000000
#define AA_FLAG_ROOT      0x00000001
#define AA_FLAG_NO_ATTACK 0x00000002

#define AAI_UNIT_HELLHOUND 46003 // Hellhound
#define AAI_UNIT_SUCCBUS   46004 // Succubus
#define AAI_UNIT_FELGUARD  46005 // Felguard
#define AAI_UNIT_DOOMGUARD 46006 // Doomguard
#define AAI_UNIT_ETERNITY  46015 // Eye of Eternity
#define AAI_UNIT_FRENZIED_PRIEST 46016 // Frenzied Priest
#define AAI_UNIT_IMP       46025 // Imp
#define AAI_UNIT_VOIDWALKER 46026 // Voidwalker

class TC_GAME_API AssistsAddon
{
public:
    AssistsAddon() {
        for (int i = 0; i < MAX_STATS; i++) {
            stat_inherited[i] = 0;
        }
        distance = 0.f;
        angle = 0.f;
        eff_spell = 0;
        awake_time = 0;
        flag = 0;
        _class = 0;
        attack_type = 0;
        text[0] = text[1] = text[2] = "";
    }
    ~AssistsAddon() {}

    void setFollowInfo(float d, float a) { distance = d; angle = a; }
    void getFollowInfo(float& d, float& a) {
        if (distance != 0.f) {
            d = distance;
            a = angle;
        }
    }

    void setAwakeTime(float t) { awake_time = t; }
    int getAwakeTime() { return awake_time; }

    void setFlag(int32 f) { flag = f; }
    bool hasFlag(int f) { return flag & f;  }

    void setEffSpell(int32 sp) { eff_spell = sp; }
    int getEffSpell() { return eff_spell; }

    void setAttackType(int t) { attack_type = t; }
    int getAttackType() { return attack_type; }

    void setClass(int c) { _class = c; }
    int getClass() { return _class; }

    void setStatInherited(Stats stat, float value) { stat_inherited[stat] = value; }
    float getStatInherited(Stats stat) { return stat_inherited[stat]; }

    std::string getText(int index) {
        if (index >= 0 && index < 3)
            return text[index];
        else
            return "";
    }
    void setText(int index, std::string txt) {
        if (index >= 0 && index < 3)
            text[index] = txt;
    }
private:
    float stat_inherited[MAX_STATS];
    float distance;
    float angle;
    int eff_spell;
    int awake_time;
    int flag;
    int _class;
    int attack_type;
    std::string text[3];
};

#define AAI_SPELL_ONETIME     0
#define AAI_SPELL_NONE_COMBAT 1
#define AAI_SPELL_HEALING     2
#define AAI_SPELL_DAMAGING    3
#define AAI_SPELL_GEN_POWER   4
#define AAI_SPELL_PROTECTING  5
#define AAI_SPELL_CONTROL     6
#define AAI_SPELL_MAX_TYPES   7

class TC_GAME_API AssistanceAI : public CreatureAI
{
public:
    enum ASSISTANCE_CLASS {
        NONE = 0,
        TANK = 1,
        HEALER = 2,
        DPS = 3
    };

    enum ASSISTANCE_ATTACK_TYPE {
        ATTACK_TYPE_MELEE = 0,
        ATTACK_TYPE_CASTER = 1
    };

    enum AI_ACTION_FLAG {
        AI_ACTION_NONE = 0,
        AI_ACTION_HIDE = 1,
        AI_ACTION_PASSIVE = 2,
        AI_ACTION_HOLD_POSITION = 3,
        AI_ACTION_STG_MOVE = 4
    };

    enum AI_STATE {
        AI_STATE_STAND,
        AI_STATE_CHASE_VICTIM,
        AI_STATE_CHASE_OWNER
    };

public:
    static std::map<unsigned, AssistsAddon> assist_addons;
    uint32 AIFlag;
    explicit AssistanceAI(Creature* c) : CreatureAI(c), _updateTimer(0) {
        AIFlag = AI_ACTION_NONE;
        isInCombat = false;
        isMovable = true;
        isTargetChanged = false;
        canAttack = true;
        _awakeTimer = 0;
        _lifeTimer = -1;
        _returnDistance = 25.f;
        _followAngle = static_cast<float>((rand() % 32) * M_PI / 16);
        _followDistance = 1.2f;
        _gcd = 0;
        for (int i = 0; i < AAI_SPELL_MAX_TYPES; i++) {
            std::vector<int32> nullVector;
            _spells.insert(std::pair<unsigned, std::vector<int32>>(i, nullVector));
        }
        _state = AI_STATE_STAND;
    }

    Unit* SelectNextTarget(bool allowAutoSelect);
    void UpdateAI(uint32) override;
    static int32 Permissible(Creature const* creature);
    void JustDied(Unit* /*killer*/);
    void JustAppeared() override;

    void KilledUnit(Unit* /*victim*/);

    void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/);
    // Called at any Damage from any attacker (before damage apply)
    // Note: it for recalculation damage or special reaction at damage
    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/);

    void HealDone(Unit* /*done_to*/, uint32& /*addhealth*/);
    void SetData(uint32 id, uint32 value) {
        _custm_data.insert(std::pair<uint32, uint32>(id, value));
    }
    uint32 GetData(uint32 id) const {
        std::map<uint32, uint32>::const_iterator it = _custm_data.find(id);
        if (it == _custm_data.end())
            return 0;
        return _custm_data.at(id);
    }
    void Reborn(uint32 pct);
    void ResetPosition(bool force = false);
    bool AddOneTimeSpell(int32 spellId);
    void AddSpellWithLevelLimit(int32 spellid, int32 level);
    bool AttemptAddProperSpellForLevel(uint32 basespell);

    ASSISTANCE_CLASS _class;
    ASSISTANCE_ATTACK_TYPE _type;
private:
    bool isInitailized = false;
    bool isInCombat;
    bool isMovable;
    bool canAttack;
    bool isTargetChanged;
    std::map<unsigned, std::vector<int32>> _spells;
    int _effSpell;
    Player* _realowner;
    float _gcd;
    float _followAngle;
    float _followDistance;
    float _returnDistance;
    float _lifeTimer;
    SpellCastResult _lastSpellResult;
    float _awakeTimer;
    float _updateTimer;
    std::map<uint32, uint32> _custm_data;

    AI_STATE _state;

    bool checkOneTimeSpells();
    bool checkNoneCombatSpells();
    bool checkHealingSpells();
    bool checkDamagingSpells();
    bool checkProtectingSpells();
    bool checkGenPowerSpells();
    bool checkControlSpells();

    void InsertSpell(int type, int spellid);
    Creature* getOwnerPet();
    bool spellCasted(SpellCastResult result);
    void stopCombatWith(Unit* a);
    bool OnGossipHello(Player*);
    bool OnGossipSelect(Player*, uint32, uint32);
    Unit* GetVictim();
    Unit* getAttackerForHelper(Unit* unit);
    bool IsTargetValid(Unit* target, bool& endCombat);
    bool OnGossipSelectCode(Player*, uint32, uint32, char const*);
    bool canAttackTarget(Unit const* target);
    bool isSpellReady(int32 index);
    void updateTimer(uint32 diff);
    bool updateCombatStatus();
    void resetLifeTimer();
    void ReadyToDie();
    void SpellInfoAnalyzeAndInsert(const SpellInfo* si);
    void handleUndeadRaceTalent();
    void handleOrcRaceTalent();
    bool isCaster();
    void EngagementStart(Unit* who);
    float GetManaPct();
    void UseInstanceHealing();
    void OnLevelUp();
    void InitFromDB();
};

#endif
