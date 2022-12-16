#include "Define.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Item.h"
#include "Player.h"
#include "GossipDef.h"
#include "WorldSession.h"
#include "ScriptedGossip.h"
#include "TCTogetherHandler.h"
// .. more includes

#pragma execution_character_set("utf-8") 

class telebook_script_class : ItemScript
{
public:
    telebook_script_class() : ItemScript("telebook_script") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        player->PlayerTalkClass->ClearMenus();
        this->OnUseGossipMenuSend(player, item);
        return false;
    }

    void OnUseGossipMenuSend(Player* player, Item* item) {
        if (player->IsInFlight())
        {
            player->GetSession()->SendAreaTriggerMessage("不能在飞行时使用"); return;
        }
        else if (player->IsMounted())
        {
            player->GetSession()->SendAreaTriggerMessage("正在乘骑状态"); return;
        }
        else if (player->IsInCombat())
        {
            player->GetSession()->SendAreaTriggerMessage("不能在战斗中使用"); return;
        }

        BuildMainMenu(player, item, 0);
    }
};

void AddSC_telebook_script()
{
    new telebook_script_class();
}
