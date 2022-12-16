#ifndef ASSISTANCE_AI_H
#define ASSISTANCE_AI_H

#include "CreatureAI.h"

class Creature;

#define MAX_CREATURE_SPELL 8
#define MAX_TIMER_TYPE 2
#define ASSIST_GCD 1050
#define AAI_DEFAULT_UPDATE_TIMER 100

class TC_GAME_API AssistanceAI : public CreatureAI
{
public:
    enum CREATURE_SPELL_TIMER {
        SPELL_TIMER_CURRENT = 0,
        SPELL_TIMER_ORIGIN = 1
    };

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

public:
    uint32 AIFlag;
    explicit AssistanceAI(Creature* c) : CreatureAI(c), _updateTimer(0) {
        AIFlag = AI_ACTION_NONE;
        isInCombat = false;
        isMovable = true;
        isTargetChanged = false;
        canAttack = true;
        _awakeTimer = 0;
        _lifeTimer = -1;
        for (int i = 0; i < MAX_CREATURE_SPELL; i++) {
            oneTimeSpells[i] = 0;
        }
    }
    Creature* GetMe();
    Unit* SelectNextTarget(bool allowAutoSelect);
    void UpdateAI(uint32) override;
    static int32 Permissible(Creature const* creature);
    void JustDied(Unit* /*killer*/);
    void JustAppeared() override;
    void Reborn(uint32 pct);
    void ResetPosition(bool force = false);
    bool AddOneTimeSpell(int32 spellId);

private:
    ASSISTANCE_CLASS _class;
    ASSISTANCE_ATTACK_TYPE _type;
    bool timerReady = false;
    bool isInCombat;
    bool isMovable;
    bool canAttack;
    bool isTargetChanged;
    int32 oneTimeSpells[MAX_CREATURE_SPELL];
    bool auraApplied[MAX_CREATURE_SPELL];
    int32 spellsTimer[MAX_CREATURE_SPELL][MAX_TIMER_TYPE];
    float _followAngle;
    float _followDistance;
    float _lifeTimer;
    SpellCastResult _lastSpellResult;
    float _awakeTimer;
    float _updateTimer;


    Creature* getOwnerPet();
    void stopCombatWith(Unit* a);
    Unit* GetVictim();
    Unit* getAttackerForHelper(Unit* unit);
    bool IsTargetValid(Unit* target, bool& endCombat);
    bool canAttackTarget(Unit const* target);
    bool hasSpell(uint32 id, uint32& index);
    bool castSpell(WorldObject* target, int32 index);
    bool castSpell(const Position& dest, int32 index);
    bool isSpellReady(int32 index);
    void updateTimer(uint32 diff);
    void resetLifeTimer();
    void ReadyToDie();
    bool isCaster();
    void EngagementStart(Unit* who);
    float GetManaPct();
    bool AssistantsSpell(uint32 diff, Unit* victim);
    void UseInstanceHealing();
    float getSpecialFollowAngle();
};

#endif
