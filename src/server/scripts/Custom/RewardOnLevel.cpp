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
#include "Player.h"
#include "ScriptMgr.h"

class RewardOnLevel : public PlayerScript
{
public:
    RewardOnLevel() : PlayerScript("RewardOnLevelScript") { }

    void OnLogin(Player* Player, bool firstLogin) override
    {
        /*if (sConfigMgr->GetBoolDefault("RewardOnLevel.enable", true))
        {

        }*/
    }

    virtual void OnLevelChanged(Player* player, uint8 oldlevel) override
    {
		uint8 diff = player->GetLevel() - oldlevel;

		player->AddItem(60103, 5 * diff);
		player->AddItem(60104, 10 * diff);
	}

    void OnGiveXP(Player* player, uint32& amount, Unit* victim) override
    {
    }
};

void AddSC_RewardOnLevel()
{
    new RewardOnLevel;
}
