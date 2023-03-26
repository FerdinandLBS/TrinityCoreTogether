#ifndef __MYWOWHANDLER_H
#define __MYWOWHANDLER_H

#include "ScriptedGossip.h"

enum eMyWowGossipSender {
    MW_GOSSIP_SENDER_MAIN = 0,
    MW_GOSSIP_SENDER_TRANS,
    MW_GOSSIP_SENDER_TRANS_OUTLAN,
    MW_GOSSIP_SENDER_TRANS_NORTHLAND,
    MW_GOSSIP_SENDER_TRANS_FIVE_DUNGEON,
    MW_GOSSIP_SENDER_TRANS_RAID,
    MW_GOSSIP_SENDER_TRADE,
    MW_GOSSIP_SENDER_SUMMON,

    MW_GOSSIP_SENDER_CLOSE,
    MW_GOSSIP_SENDER_ACTION_DO_TRANS,


    MW_GOSSIP_ACTION_MINI_GAME,

    MW_GOSSIP_HUM_TALENT_MAIN,
    MW_GOSSIP_DWARF_TALENT_MAIN
};

enum eMyWowGossipAction {
    MW_GOSSIP_ACTION_SUB_MENU = GOSSIP_ACTION_INFO_DEF + 1,
    MW_GOSSIP_ACTION_TRANS,
    MW_GOSSIP_ACTION_DO
};

void BuildMainMenu(Player* player, Item* item, uint32 sender);
void GossipSelect_Item(Player* player, Item* item, uint32 sender, uint32 action);
void GossipTCTogetherCreature(Player* player, Creature* creature, uint32 sender, uint32 action);
void GossipTCTogetherGameObject(Player* player, GameObject* obj, uint32 sender, uint32 action);
void GossipActionDoTrans(Player* player, Item* item, uint32 action);
void BuildHumanRaceTalentGossip(Player* player, Creature* creature, uint32 sender);
void UpgradeHumanRaceTalentMinion(Player* player, Creature* creature, uint32 sender);
void BuildDwarfRaceTalentMenu(Player* player, GameObject* object, uint32 sender);
void HireDwarfRaceTalentMinion(Player* player, Creature* creature, uint32 action);

#endif
