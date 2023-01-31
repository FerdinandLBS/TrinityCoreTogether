#include "Define.h"
#include "Group.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Item.h"
#include "Player.h"
#include "GossipDef.h"
#include "WorldSession.h"
#include "ScriptedGossip.h"
#include "TCTogetherHandler.h"
#include <AI\CoreAI\AssistanceAI.h>
// .. more includes

#pragma execution_character_set("utf-8")


void BuildSpecailTeleport(Player* player, Item* item, uint32 sender)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "黑石塔 55 - 65", GOSSIP_SENDER_INFO, MW_GOSSIP_ACTION_TRANS + 200);

    SendGossipMenuFor(player, 4, item->GetGUID());
}

void BuildTransMenu(Player* player, Item* item, uint32 sender)
{
    if (!player || !player->IsAlive())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    uint8 level = player->GetLevel();
    menu->ClearMenus();

    if (player->GetTeam() == ALLIANCE)
    {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "暴风城", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 1);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "铁炉堡", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 2);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "达纳苏斯", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 3);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "埃索达", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 4);
    }
    else
    {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "奥格瑞玛", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 5);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "雷霆崖", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 6);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "幽暗城", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 7);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "银月城", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 8);
    }

    switch (player->GetRace()) {
    case RACE_BLOODELF:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "苏伦的养殖场", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    }

    if (level > 13)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "棘齿城", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 9);

    if (level > 35)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "藏宝海湾", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 10);

    if (level > 39)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "热沙岗", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 11);

    if (level > 10)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, "|cffFF8800GM岛|r", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 14);

    if (player->GetLevel() >= 58)
        AddGossipItemFor(player, GOSSIP_ICON_TABARD, "|cffFF0005外域|r", MW_GOSSIP_SENDER_TRANS_OUTLAN, MW_GOSSIP_ACTION_SUB_MENU);

    if (player->GetLevel() >= 69)
        AddGossipItemFor(player, GOSSIP_ICON_TABARD, "|cff0143FF诺森德|r", MW_GOSSIP_SENDER_TRANS_NORTHLAND, MW_GOSSIP_ACTION_SUB_MENU);


    AddGossipItemFor(player, GOSSIP_ICON_TABARD, "|cff05F3255人副本|r", MW_GOSSIP_SENDER_TRANS_FIVE_DUNGEON, MW_GOSSIP_ACTION_SUB_MENU);
    AddGossipItemFor(player, GOSSIP_ICON_TABARD, "|cffFF0BFF团队副本|r", MW_GOSSIP_SENDER_TRANS_RAID, MW_GOSSIP_ACTION_SUB_MENU);

    AddGossipItemFor(player, GOSSIP_ICON_TALK, "<<< 后退", MW_GOSSIP_SENDER_MAIN, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, item->GetGUID());
}

void BuildMainMenu(Player* player, Item* item, uint32 sender)
{
    if (!player || !player->IsAlive())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "|cff05E605传送服务|r", MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);
    AddGossipItemFor(player, GOSSIP_ICON_TALK, "关闭", MW_GOSSIP_SENDER_CLOSE, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, item->GetGUID());
}

void BuildRaidTeleport(Player* player, Item* item, uint32 sender)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "熔火之心(60)", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 301);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "黑翼之巢(80)", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 302);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "安其拉(60)", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 303);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "祖尔格拉布(60)", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 304);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "祖阿曼(70)", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 305);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "卡拉赞(70)", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 306);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "奥妮克希亚巢穴(80)", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 307);

    AddGossipItemFor(player, GOSSIP_ICON_TALK, "<<< 后退", MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 2, item->GetGUID());
}

void BuildNorthlandTeleport(Player* player, Item* item, uint32 sender)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();
    AddGossipItemFor(player, GOSSIP_ICON_CHAT_13, "达拉然", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 101);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "乌特加德堡垒", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 104);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "魔枢", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 103);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "龙眠神殿", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 102);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "古代王国", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 105);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "达克萨隆要塞", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 111);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "古达克", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 106);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "风暴群山", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 107);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "纳克萨玛斯", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 110);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "银色比武场", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 108);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "冰冠堡垒", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 109);

    AddGossipItemFor(player, GOSSIP_ICON_TALK, "<<< 后退", MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 4, item->GetGUID());
}

void BuildOutlandTeleport(Player* player, Item* item, uint32 sender)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();
    AddGossipItemFor(player, GOSSIP_ICON_CHAT_13, "沙塔斯", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 201);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "地狱火堡垒", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 202);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "盘牙水库", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 203);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "奥金顿", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 204);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "风暴战舰", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 205);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "戈鲁尔巢穴", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 206);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "黑暗神庙", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 207);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "时光之穴", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 208);

    AddGossipItemFor(player, GOSSIP_ICON_TALK, "<<< 后退", MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 4, item->GetGUID());
}

void BuildDungeonTeleport(Player* player, Item* item, uint32 sender)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "怒焰裂谷 15 - 21", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 51);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "死亡矿井 15 - 21", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 52);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "哀嚎洞穴 15 - 25", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 53);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "影牙城堡 16 - 26", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 54);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "黑暗深渊 20 - 30", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 55);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "暴风城监狱 20 - 30", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 56);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "诺莫瑞根 24 - 34", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 57);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "剃刀沼泽 25 - 30", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 59);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "血色修道院 26 - 40", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 58);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "剃刀高地 34 - 40", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 64);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "奥达曼 35 - 40", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 61);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "祖尔法拉克 43 - 46", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 66);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "玛拉顿 43 - 48", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 60);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "厄运之槌 54 - 58", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 62);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "通灵学院 59 - 61", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 63);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "斯坦索姆 56 - 60", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 65);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "黑石深渊 49 - 57", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 67);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "沉没的神庙 50- 60", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 68);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "黑石塔 55 - 65", MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 69);
     
    AddGossipItemFor(player, GOSSIP_ICON_TALK, "<<< 后退", MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 4, item->GetGUID());
}

void GossipActionDoTrans(Player* player, Item* item, uint32 action) {
    switch (action) {
    case MW_GOSSIP_ACTION_TRANS + 1: // StormWind City
        player->TeleportTo(0, -8730.59f, 722.68f, 101.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 2: // TLB
        player->TeleportTo(0, -4799.36f, -1107.36f, 502.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 3: // DNSS
        player->TeleportTo(1, 9961, 2055, 1329, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 4: // 埃索达
        player->TeleportTo(530, -3998.3f, -11864.1f, 1, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 5: // 奥格瑞玛
        player->TeleportTo(1, 1676.25f, -4313.45f, 62.0f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 6: // 雷霆崖
        player->TeleportTo(1, -1150.877197f, 15.459573f, 180.088318f, 1.300810f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 7: // 幽暗城
        player->TeleportTo(0, 1596.05835f, 240.41658f, -13.89129f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 8: // 银月城
        player->TeleportTo(530, 9930.45f, -7129.1f, 48, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 9: // 棘齿城
        player->TeleportTo(1, -977, -3788, 6, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 10:
        player->TeleportTo(0, -14302, 518, 9, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 11:
        player->TeleportTo(1, -7156.56f, -3825.1f, 8.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 14: // GM岛 
        player->TeleportTo(1, 16222.1f, 16252.1f, 12.5872f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 20: // BE 骑术
        player->TeleportTo(530, 9258.043f, -7472.375f, 35.55f, 4.63f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 51: // 怒焰裂谷
        player->TeleportTo(389, 3.8f, -14.8f, -17.f, 6.f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 52: // 矿井
        player->TeleportTo(36, -16, -383, 62, 6.f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 53: // 哀嚎
        player->TeleportTo(43, -163.49f, 132.89f, -73.66f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 54: // 影牙
        player->TeleportTo(33, -229.1f, 2109.17f, 77, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 55: // 深渊
        player->TeleportTo(48, -151.88f, 106.95f, -39.3f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 56: // 监狱
        player->TeleportTo(34, 54.2f, 0.28f, -18, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 57: // 诺莫瑞根
        player->TeleportTo(90, -327.5f, -4.7f, -152.3f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 58: // 血色
        player->TeleportTo(0, 2894.34f, -809.55f, 160.33f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 59: // 剃刀沼泽
        player->TeleportTo(47, 1943, 1544.63f, 82, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 60: // 玛拉顿
        player->TeleportTo(349, 1019.69f, -458.3f, -43, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 61: // 奥达曼
        player->TeleportTo(70, -226.8f, 49.1f, -45.9f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 62: // 厄运
        player->TeleportTo(429, -201.11f, -328.66f, -2.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 63: // 通灵学院
        player->TeleportTo(289, 196.39f, 127, 135, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 64: // 剃刀高地
        player->TeleportTo(129, 2592.55f, 1107.5f, 51.5f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 65: // 斯坦索姆
        player->TeleportTo(329, 3394.13f, -3380.16f, 143.0f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 66: // 祖尔法拉克
        player->TeleportTo(209, 1213.52f, 841.59f, 9, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 67: // 黑石深渊
        player->TeleportTo(230, 458.3f, 26.5f, -70.64f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 68: // 沉没的神庙
        player->TeleportTo(109, -319.23f, 99.9f, -131.85f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 69: // 黑石塔
        player->TeleportTo(229, 78.5f, -225.0f, 50.0f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 101: // 达拉燃
        player->TeleportTo(571, 5797, 795, 664, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 102: // 龙眠神殿
        player->TeleportTo(571, 3546.607178f, 273.218842f, 342.722f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 103: // 魔枢
        player->TeleportTo(571, 3831.737061f, 6960.383789f, 104.784271f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 104: // 乌特加德堡垒
        player->TeleportTo(571, 1260.176636f, -4843.805664f, 215.763993f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 105: // 古代王国
        player->TeleportTo(571, 3695.932129f, 2143.285889f, 34.147270f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 106: // 古达克
        player->TeleportTo(571, 6938.497559f, -4452.765137f, 450.868896f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 107: // 风暴群山
        player->TeleportTo(571, 8949.208008f, -1266.415894f, 1025.499391f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 108: // 银色比武场
        player->TeleportTo(571, 8486.941406f, 775.859863f, 558.568299f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 109: // 冰冠堡垒
        player->TeleportTo(571, 5864.67f, 2169.83f, 636.1f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 110: // 纳克萨玛斯
        player->TeleportTo(571, 3666.089844f, -1269.738403f, 243.508927f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 111: // 达克萨隆要塞
        player->TeleportTo(571, 4772.635742f, -2046.703125f, 238.28464f, 0.061439f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 201: // 沙塔斯
        player->TeleportTo(530, -1859.95f, 5438.85f, -10.3f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 202: // 地狱火
        player->TeleportTo(530, -321.64f, 3082.49f, 32.6f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 203: // 水库
        player->TeleportTo(530, 764.034058f, 6866.363770f, -68.277512f, 6.266417f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 204: // 奥金顿
        player->TeleportTo(530, -3377.06f, 4954.24f, -66.5f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 205: // 风暴战舰
        player->TeleportTo(530, 3101.006592f, 1537.525879f, 190.31f, 4.649131f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 206: // 格鲁尔
        player->TeleportTo(565, 62.784199f, 35.462002f, -3.983500f, 1.418440f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 207: // 黑庙
        player->TeleportTo(564, 96.45f, 1002.35f, -86.8f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 208: // 时光之穴
        player->TeleportTo(1, -8509.349606f, -4356.310059f, -208.358994f, 6);
        break;
        // Raid telepot
    case MW_GOSSIP_ACTION_TRANS + 301: // 熔火之心
        player->TeleportTo(409, 1087.588f, -477.341f, -107.0f, 0.786652f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 305: // 祖阿曼
        player->TeleportTo(530, 6832.783203f, -7858.009766f, 163.976166f, 4.697f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 306: // 卡拉赞
        player->TeleportTo(532, -11102.0f, -1998.19f, 50.05f, 0.533f);
        break; 
    case MW_GOSSIP_ACTION_TRANS + 307: // 奥妮克西娅
        player->TeleportTo(1, -4745.300293f, -3753.068604f, 50.219667f, 4.697f);
        break;
        /*
        case MW_GOSSIP_ACTION_TRANS + 64: //
        player->TeleportTo(, f, f, f, 6);
        break;*/
    }
    CloseGossipMenuFor(player);
}

extern void UnitAddHealthPct(Unit*, int);

void UpgradeHumanRaceTalentMinion(Player* player, Creature* m, uint32 action) {
    if (!player || !m)
        return;

    Gender gender = m->GetGender();
    AssistanceAI* ai = (AssistanceAI*)m->GetAI();

    switch (action) {
    case MW_GOSSIP_ACTION_DO + 1:
        m->SetDisplayId(gender == GENDER_FEMALE ? 3257 : 2072);
        m->SetPowerType(Powers::POWER_RAGE);
        m->SetVirtualItem(0, 3455);
        m->SetVirtualItem(1, 1203);
        ai->SetData(0, 0);
        m->m_spells[0] = 56222;
        m->m_spells[1] = 81194;
        UnitAddHealthPct(m, 160);
        m->CastSpell(m, 81193, true);
        break;
    case MW_GOSSIP_ACTION_DO + 2:
        m->SetDisplayId(gender == GENDER_FEMALE ? 3292 : 1484);
        m->SetVirtualItem(0, 812);
        m->SetPowerType(Powers::POWER_MANA);
        ai->_type = AssistanceAI::ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        m->m_spells[0] = gender == GENDER_FEMALE ? 81199 : 81200;
        m->m_spells[1] = gender == GENDER_FEMALE ? 81199 : 81200;
        ai->SetData(0, 1);
        break;
    case MW_GOSSIP_ACTION_DO + 3:
        m->SetDisplayId(gender == GENDER_FEMALE ? 1295 : 3253);
        m->SetPowerType(Powers::POWER_MANA);
        ai->_class = AssistanceAI::ASSISTANCE_CLASS::HEALER;
        ai->_type = AssistanceAI::ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        m->m_spells[0] = 81204;
        m->m_spells[1] = 81205;
        m->SetVirtualItem(0, 812);
        ai->SetData(0, 2);
        break;
    }
    m->CastSpell(m, 24312, true);
    m->RemoveNpcFlag(NPCFlags::UNIT_NPC_FLAG_GOSSIP);
    m->UpdateDisplayPower();
    m->RemoveAura(81192);
    m->UpdateAllStats();
    m->SetFullHealth();
    m->SetObjectScale(1);
    player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);
}

void BuildHumanRaceTalentGossip(Player* player, Creature* creature, uint32 sender)
{
    if (!player || !player->IsAlive() || !creature->IsAlive())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|cffFF0005晋升 - 卫兵|r", MW_GOSSIP_HUM_TALENT_MAIN, MW_GOSSIP_ACTION_DO + 1);
    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|cff0500FF晋升 - 魔法士|r", MW_GOSSIP_HUM_TALENT_MAIN, MW_GOSSIP_ACTION_DO + 2);
    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|cff05C405晋升 - 信徒|r", MW_GOSSIP_HUM_TALENT_MAIN, MW_GOSSIP_ACTION_DO + 3);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, creature->GetGUID());
}

void GossipTCTogetherHumanRaceTalentSelected(Player* player, Creature* creature, uint32 sender, uint32 action) {

}

void GossipTCTogetherCreature(Player* player, Creature* creature, uint32 sender, uint32 action) {
    switch (sender) {
    case MW_GOSSIP_HUM_TALENT_MAIN:
        return UpgradeHumanRaceTalentMinion(player, creature, action);
    }
}

void GossipSelect_Item(Player* player, Item* item, uint32 sender, uint32 action)
{
    if (!player || player->isDead())
        return;
    if (!item)
        return;

    switch (sender) {
    case MW_GOSSIP_SENDER_MAIN:
        return BuildMainMenu(player, item, sender);
    case MW_GOSSIP_SENDER_TRANS:
        return BuildTransMenu(player, item, sender);
    case MW_GOSSIP_SENDER_TRANS_OUTLAN:
        return BuildOutlandTeleport(player, item, sender);
    case MW_GOSSIP_SENDER_TRANS_NORTHLAND:
        return BuildNorthlandTeleport(player, item, sender);
    case MW_GOSSIP_SENDER_TRANS_FIVE_DUNGEON:
        return BuildDungeonTeleport(player, item, sender);
    case MW_GOSSIP_SENDER_TRANS_RAID:
        return BuildRaidTeleport(player, item, sender);
    case MW_GOSSIP_SENDER_TRADE:
        break;
    case MW_GOSSIP_SENDER_ACTION_DO_TRANS:
        return GossipActionDoTrans(player, item, action);
    case MW_GOSSIP_SENDER_CLOSE:
        return CloseGossipMenuFor(player);
    }
}
