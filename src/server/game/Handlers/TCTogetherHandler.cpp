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
#include "TemporarySummon.h"
#include <AI\CoreAI\AssistanceAI.h>
// .. more includes

#define TCTSTR_UPGRADE_GUARD  20090
#define TCTSTR_UPGRADE_MAGE   20091
#define TCTSTR_UPGRADE_HEALER 20092

#define TCTSTR_HIRE_DWARF     20093
#define TCTSTR_TRANS_EAGLE_NEXT         20094
#define TCTSTR_TRANS_BLACKROCK_MOUNTAIN 20095
#define TCTSTR_TRANS_ALTER_OF_MAKERS    20096

#define TCTSTR_TRANS_SERVICE   20100
#define TCTSTR_CLOSE           20101
#define TCTSTR_BACK            20102
#define TCTSTR_TRANS_GMISLAND  20110
#define TCTSTR_TRANS_OUTLAND   20111
#define TCTSTR_TRANS_NORTHLAND 20112
#define TCTSTR_TRANS_DUNGEON   20113
#define TCTSTR_TRANS_RAID      20114
#define TCTSTR_TRANS_STORMWIND 20120
#define TCTSTR_TRANS_IRONFORGE 20121
#define TCTSTR_TRANS_DRANASSUS 20122
#define TCTSTR_TRANS_EXORDA    20123
#define TCTSTR_TRANS_ORGRIMMA  20124
#define TCTSTR_TRANS_THUNDERBLUFF 20125
#define TCTSTR_TRANS_UNDERCITY 20126
#define TCTSTR_TRANS_SILVERMOON 20127
#define TCTSTR_TRANS_BE_RIDING 20128
#define TCTSTR_TRANS_UD_RIDING 20129
#define TCTSTR_TRANS_TAUR_RIDING 20130
#define TCTSTR_TRANS_TR_RIDING 20131
#define TCTSTR_TRANS_DW_RIDING 20132
#define TCTSTR_TRANS_HU_RIDING 20133
#define TCTSTR_TRANS_GN_RIDING 20134
#define TCTSTR_TRANS_RATCHET   20150
#define TCTSTR_TRANS_BOOTYBAY  20151
#define TCTSTR_TRANS_GADGETZAN 20152
#define TCTSTR_TRANS_SHATTRATH 20160
#define TCTSTR_TRANS_OUTLAND_SIZE 8
#define TCTSTR_TRANS_DALARAN   20170
#define TCTSTR_TRANS_DALARAN_SIZE 11
#define TCTSTR_TRANS_RAID_START 20190
#define TCTSTR_TRANS_RAID_SIZE 7
#define TCTSTR_TRANS_DUNGEON_START 20200
#define TCTSTR_TRANS_DUNGEON_SIZE 19

static std::string GetTrinityString(uint32 entry) {
    const TrinityString* ts = sObjectMgr->GetTrinityString(entry);
    if (ts != nullptr) {
        return ts->Content[0];
    }
    return "";
}

void BuildTransMenu(Player* player, Item* item, uint32)
{
    if (!player || !player->IsAlive())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    uint8 level = player->GetLevel();
    menu->ClearMenus();

    if (player->GetTeam() == ALLIANCE) {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_STORMWIND), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 1);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_IRONFORGE), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 2);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_DRANASSUS), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 3);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_EXORDA), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 4);
    } else {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_ORGRIMMA), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 5);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_THUNDERBLUFF), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 6);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_UNDERCITY), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 7);
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_SILVERMOON), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 8);
    }

    switch (player->GetRace()) {
    case RACE_BLOODELF:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_BE_RIDING), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    case RACE_UNDEAD_PLAYER:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_UD_RIDING), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    case RACE_HUMAN:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_HU_RIDING), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    case RACE_TROLL:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_TR_RIDING), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    case RACE_TAUREN:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_TAUR_RIDING), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    case RACE_DWARF:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_DW_RIDING), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    case RACE_GNOME:
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_GN_RIDING), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 20);
        break;
    }

    if (level > 13)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_RATCHET), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 9);

    if (level > 35)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_BOOTYBAY), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 10);

    if (level > 39)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_GADGETZAN), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 11);

    if (level > 10)
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_GMISLAND), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 14);

    if (player->GetLevel() >= 58)
        AddGossipItemFor(player, GOSSIP_ICON_TABARD, GetTrinityString(TCTSTR_TRANS_OUTLAND), MW_GOSSIP_SENDER_TRANS_OUTLAN, MW_GOSSIP_ACTION_SUB_MENU);

    if (player->GetLevel() >= 69)
        AddGossipItemFor(player, GOSSIP_ICON_TABARD, GetTrinityString(TCTSTR_TRANS_NORTHLAND), MW_GOSSIP_SENDER_TRANS_NORTHLAND, MW_GOSSIP_ACTION_SUB_MENU);


    AddGossipItemFor(player, GOSSIP_ICON_TABARD, GetTrinityString(TCTSTR_TRANS_DUNGEON), MW_GOSSIP_SENDER_TRANS_FIVE_DUNGEON, MW_GOSSIP_ACTION_SUB_MENU);
    AddGossipItemFor(player, GOSSIP_ICON_TABARD, GetTrinityString(TCTSTR_TRANS_RAID), MW_GOSSIP_SENDER_TRANS_RAID, MW_GOSSIP_ACTION_SUB_MENU);

    AddGossipItemFor(player, GOSSIP_ICON_TALK, GetTrinityString(TCTSTR_BACK), MW_GOSSIP_SENDER_MAIN, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, item->GetGUID());
}

void BuildMainMenu(Player* player, Item* item, uint32)
{
    if (!player || !player->IsAlive())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_SERVICE), MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);
    //AddGossipItemFor(player, GOSSIP_ICON_TAXI, "|cffeff064????|r", MW_GOSSIP_ACTION_MINI_GAME, MW_GOSSIP_ACTION_SUB_MENU);
    AddGossipItemFor(player, GOSSIP_ICON_TALK, GetTrinityString(TCTSTR_CLOSE), MW_GOSSIP_SENDER_CLOSE, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, item->GetGUID());
}

void BuildRaidTeleport(Player* player, Item* item, uint32)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    for (uint32 i = 0; i < TCTSTR_TRANS_RAID_SIZE; i++) {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_RAID_START + i), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 301 + i);
    }

    AddGossipItemFor(player, GOSSIP_ICON_TALK, GetTrinityString(TCTSTR_BACK), MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 2, item->GetGUID());
}

void BuildNorthlandTeleport(Player* player, Item* item, uint32)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();
    for (int i = 0; i < TCTSTR_TRANS_DALARAN_SIZE; i++) {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_DALARAN + i), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 101 + i);
    }

    AddGossipItemFor(player, GOSSIP_ICON_TALK, GetTrinityString(TCTSTR_BACK), MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 4, item->GetGUID());
}

void BuildOutlandTeleport(Player* player, Item* item, uint32)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    for (uint32 i = 0; i < TCTSTR_TRANS_OUTLAND_SIZE; i++) {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_SHATTRATH + i), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 201 + i);
    }

    AddGossipItemFor(player, GOSSIP_ICON_TALK, GetTrinityString(TCTSTR_BACK), MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 4, item->GetGUID());
}

void BuildDungeonTeleport(Player* player, Item* item, uint32)
{
    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    for (uint32 i = 0; i < TCTSTR_TRANS_DUNGEON_SIZE; i++) {
        AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_DUNGEON_START + i), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 51 + i);
    }
     
    AddGossipItemFor(player, GOSSIP_ICON_TALK, GetTrinityString(TCTSTR_BACK), MW_GOSSIP_SENDER_TRANS, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, 4, item->GetGUID());
}

void GossipActionDoTrans(Player* player, Item*, uint32 action) {
    switch (action) {
    case MW_GOSSIP_ACTION_TRANS + 1: // StormWind City
        player->TeleportTo(0, -8730.59f, 722.68f, 101.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 2: // Ironforge Keep
        player->TeleportTo(0, -4799.36f, -1107.36f, 502.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 3: // Darnassus
        player->TeleportTo(1, 9945.995f, 2590.0857f, 1316.5952f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 4: // Exodar
        player->TeleportTo(530, -3998.3f, -11864.1f, 1, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 5: // Orgrimmar
        player->TeleportTo(1, 1676.25f, -4313.45f, 62.0f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 6: // Thunder Bluff
        player->TeleportTo(1, -1150.877197f, 15.459573f, 180.088318f, 1.300810f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 7: // Under Cidy
        player->TeleportTo(0, 1596.05835f, 240.41658f, -13.89129f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 8: // SilverMoon
        player->TeleportTo(530, 9930.45f, -7129.1f, 48, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 9: // Ratchet
        player->TeleportTo(1, -977, -3788, 6, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 10: // Bootbay
        player->TeleportTo(0, -14302, 518, 9, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 11: // Gagetzan
        player->TeleportTo(1, -7156.56f, -3825.1f, 8.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 14: // GM Island
        player->TeleportTo(1, 16222.1f, 16252.1f, 12.5872f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 20: // Riding Skill
        switch (player->GetRace()) {
        case RACE_BLOODELF:
            player->TeleportTo(530, 9258.043f, -7472.375f, 35.85f, 4.63f);
            break;
        case RACE_UNDEAD_PLAYER:
            player->TeleportTo(0, 2257.284F, 290.17767F, 34.41499F, 4.63f);
            break;
        case RACE_HUMAN:
            player->TeleportTo(0, -9452.896f, -1365.998f, 46.967625f, 4.63f);
            break;
        case RACE_TROLL:
            player->TeleportTo(1, -800.48224f, -4903.387f, 19.881464f, 4.63f);
            break;
        case RACE_TAUREN:
            player->TeleportTo(1, -2241.4563f, -392.91983f, -9.023722f, 4.63f);
            break;
        case RACE_DWARF:
            player->TeleportTo(0, -5538.6973f, -1314.2871f, 399.46607f, 4.63f);
            break;
        case RACE_GNOME:
            player->TeleportTo(0, -5485.5596f, -669.82056f, 393.2037f, 4.63f);
            break;
        }
        break;
    case MW_GOSSIP_ACTION_TRANS + 51: // Ragefire Chasm
        player->TeleportTo(389, 3.8f, -14.8f, -17.f, 6.f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 52: // Deadmines
        player->TeleportTo(36, -16, -383, 62, 6.f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 53: // Wailing Caverns
        player->TeleportTo(43, -163.49f, 132.89f, -73.66f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 54: // Shadowfang Keep
        player->TeleportTo(33, -229.1f, 2109.17f, 77, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 55: // Blackfathom Deeps
        player->TeleportTo(48, -151.88f, 106.95f, -39.3f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 56: // Stormwind jail
        player->TeleportTo(34, 54.2f, 0.28f, -18, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 57: // Gnomeregan
        player->TeleportTo(90, -327.5f, -4.7f, -152.3f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 58: // Scarlet Monastery
        player->TeleportTo(0, 2894.34f, -809.55f, 160.33f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 59: // Razorfen Downs
        player->TeleportTo(47, 1943, 1544.63f, 82, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 60: // Razorfen Kraul
        player->TeleportTo(129, 2592.55f, 1107.5f, 51.5f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 61: // Audaman
        player->TeleportTo(70, -226.8f, 49.1f, -45.9f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 62: // Zul'Farrak
        player->TeleportTo(209, 1213.52f, 841.59f, 9, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 63: // Maraudon
        player->TeleportTo(349, 1019.69f, -458.3f, -43, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 64: // Dire Maul
        player->TeleportTo(429, -201.11f, -328.66f, -2.7f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 65: // Scholomance
        player->TeleportTo(289, 196.39f, 127, 135, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 66: // Stratholme
        player->TeleportTo(329, 3394.13f, -3380.16f, 143.0f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 67: // Blackrock Depths
        player->TeleportTo(230, 458.3f, 26.5f, -70.64f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 68: // The Temple of Atal'Hakkar
        player->TeleportTo(109, -319.23f, 99.9f, -131.85f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 69: // Blackrock tower
        player->TeleportTo(229, 78.5f, -225.0f, 50.0f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 101: // ???
        player->TeleportTo(571, 5797, 795, 664, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 102: // ????
        player->TeleportTo(571, 3546.607178f, 273.218842f, 342.722f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 103: // ??
        player->TeleportTo(571, 3831.737061f, 6960.383789f, 104.784271f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 104: // ??????
        player->TeleportTo(571, 1260.176636f, -4843.805664f, 215.763993f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 105: // ????
        player->TeleportTo(571, 3695.932129f, 2143.285889f, 34.147270f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 106: // ???
        player->TeleportTo(571, 6938.497559f, -4452.765137f, 450.868896f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 107: // ????
        player->TeleportTo(571, 8949.208008f, -1266.415894f, 1025.499391f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 108: // ?????
        player->TeleportTo(571, 8486.941406f, 775.859863f, 558.568299f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 109: // ????
        player->TeleportTo(571, 5864.67f, 2169.83f, 636.1f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 110: // ?????
        player->TeleportTo(571, 3666.089844f, -1269.738403f, 243.508927f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 111: // ??????
        player->TeleportTo(571, 4772.635742f, -2046.703125f, 238.28464f, 0.061439f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 201: // ???
        player->TeleportTo(530, -1859.95f, 5438.85f, -10.3f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 202: // ???
        player->TeleportTo(530, -321.64f, 3082.49f, 32.6f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 203: // ??
        player->TeleportTo(530, 764.034058f, 6866.363770f, -68.277512f, 6.266417f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 204: // ???
        player->TeleportTo(530, -3377.06f, 4954.24f, -66.5f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 205: // ????
        player->TeleportTo(530, 3101.006592f, 1537.525879f, 190.31f, 4.649131f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 206: // ???
        player->TeleportTo(565, 62.784199f, 35.462002f, -3.983500f, 1.418440f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 207: // ??
        player->TeleportTo(564, 96.45f, 1002.35f, -86.8f, 6);
        break;
    case MW_GOSSIP_ACTION_TRANS + 208: // ????
        player->TeleportTo(1, -8509.349606f, -4356.310059f, -208.358994f, 6);
        break;
        // Raid telepot
    case MW_GOSSIP_ACTION_TRANS + 301: // ????
        player->TeleportTo(409, 1087.588f, -477.341f, -107.0f, 0.786652f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 305: // ???
        player->TeleportTo(530, 6832.783203f, -7858.009766f, 163.976166f, 4.697f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 306: // ???
        player->TeleportTo(532, -11102.0f, -1998.19f, 50.05f, 0.533f);
        break; 
    case MW_GOSSIP_ACTION_TRANS + 307: // ?????
        player->TeleportTo(1, -4745.300293f, -3753.068604f, 50.219667f, 4.697f);
        break;
        /*
        case MW_GOSSIP_ACTION_TRANS + 64: //
        player->TeleportTo(, f, f, f, 6);
        break;*/

    /* Transport for Dwarf race talent 500 - 502 */
    case MW_GOSSIP_ACTION_TRANS + 500:
        player->TeleportTo(0, 286.153f, -2006.471f, 194.2f, 4.697f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 501:
        player->TeleportTo(0, -6951.664f, -1054.764f, 241.9f, 4.697f);
        break;
    case MW_GOSSIP_ACTION_TRANS + 502:
        player->TeleportTo(571, 7762.9536f, -2138.737f, 1233.33f, 4.697f);
        break;
    }
    CloseGossipMenuFor(player);
}

extern void UnitAddHealthPct(Unit*, int);

void UpgradeHumanRaceTalentMinion(Player* player, Creature* m, uint32 action) {
    if (!player || !m)
        return;

    Gender gender = m->GetGender();
    AssistanceAI* ai = (AssistanceAI*)m->GetAI();
    if (!ai)
        return;

    switch (action) {
    case MW_GOSSIP_ACTION_DO + 1:
        m->SetDisplayId(gender == GENDER_FEMALE ? 3257 : 2072);
        m->SetPowerType(Powers::POWER_RAGE);
        m->SetVirtualItem(0, 3455);
        m->SetVirtualItem(1, 1203);
        ai->SetData(0, 0);
        ai->AttemptAddProperSpellForLevel(56222);
        ai->AttemptAddProperSpellForLevel(81194);
        //UnitAddHealthPct(m, 160);
        //ai->SetData(ADATA_ID_STAT_PCT_AGI, 100);
        //ai->SetData(ADATA_ID_STAT_PCT_STA, 50);
        m->CastSpell(m, 81193, true); // Improve threats
        break;
    case MW_GOSSIP_ACTION_DO + 2:
        m->SetDisplayId(gender == GENDER_FEMALE ? 3292 : 1484);
        m->SetVirtualItem(0, 812);
        m->SetPowerType(Powers::POWER_MANA);
        ai->_type = AssistanceAI::ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        ai->AttemptAddProperSpellForLevel(gender == GENDER_FEMALE ? 81199 : 81200);
        ai->SetData(0, 1);
        //ai->SetData(ADATA_ID_STAT_PCT_INT, 100);
        //ai->SetData(ADATA_ID_STAT_PCT_SPI, 120);
        break;
    case MW_GOSSIP_ACTION_DO + 3:
        m->SetDisplayId(gender == GENDER_FEMALE ? 1295 : 3253);
        m->SetPowerType(Powers::POWER_MANA);
        ai->_class = AssistanceAI::ASSISTANCE_CLASS::HEALER;
        ai->_type = AssistanceAI::ASSISTANCE_ATTACK_TYPE::ATTACK_TYPE_CASTER;
        ai->AttemptAddProperSpellForLevel(81204);
        ai->AttemptAddProperSpellForLevel(81205);
        m->SetVirtualItem(0, 812);
        ai->SetData(0, 2);
        //ai->SetData(ADATA_ID_STAT_PCT_INT, 80);
        //ai->SetData(ADATA_ID_STAT_PCT_SPI, 160);
        break;
    }
    m->CastSpell(m, 24312, true);
    ((Guardian*)m)->InitStatsForLevel(m->GetLevel());
    m->RemoveNpcFlag(NPCFlags::UNIT_NPC_FLAG_GOSSIP);
    m->UpdateDisplayPower();
    m->RemoveAura(81192);
    m->UpdateAllStats();
    m->SetFullHealth();
    m->SetObjectScale(1);
    player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_POWER_TYPE);
    player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);
}

void BuildHumanRaceTalentGossip(Player* player, Creature* creature, uint32)
{
    if (!player || !player->IsAlive() || !creature->IsAlive())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|cffFF0005" + GetTrinityString(TCTSTR_UPGRADE_GUARD) + "|r", MW_GOSSIP_HUM_TALENT_MAIN, MW_GOSSIP_ACTION_DO + 1);
    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|cff0500FF" + GetTrinityString(TCTSTR_UPGRADE_MAGE) + "|r", MW_GOSSIP_HUM_TALENT_MAIN, MW_GOSSIP_ACTION_DO + 2);
    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|cff05C405" + GetTrinityString(TCTSTR_UPGRADE_HEALER) + "|r", MW_GOSSIP_HUM_TALENT_MAIN, MW_GOSSIP_ACTION_DO + 3);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, creature->GetGUID());
}

void GossipTCTogetherHumanRaceTalentSelected(Player* player, Creature* creature, uint32 sender, uint32 action) {
    (void)player;
    (void)creature;
    (void)sender;
    (void)action;
}

void HireDwarfRaceTalentMinion(Player* player, Creature* creature, uint32) {
    std::list<Creature*> list;

    for (int i = 0; i < 5; i++) {
        player->GetAllMinionsByEntry(list, 45008 + i);
        if (list.size() > 0) {
            creature->Whisper(GetTrinityString(TCTSTR_HIRE_DWARF), Language::LANG_DWARVISH, player);
            return;
        }
    }

    uint32 entry = creature->GetEntry();
    Position position = creature->GetPosition();
    creature->DespawnOrUnsummon();
    player->CastSpell(creature, entry + 81360 - 45008, true);
}

void GossipTCTogetherCreature(Player* player, Creature* creature, uint32 sender, uint32 action) {
    switch (sender) {
    case MW_GOSSIP_HUM_TALENT_MAIN:
        return UpgradeHumanRaceTalentMinion(player, creature, action);
    case MW_GOSSIP_DWARF_TALENT_MAIN:
        ; //return HireDwarfRaceTalentMinion(player, creature, action);
    }
}

void GossipTCTogetherGameObject(Player* player, GameObject* obj, uint32, uint32 action) {
    switch (obj->GetEntry()) {
    case 250000:
        GossipActionDoTrans(player, nullptr, action);
        break;
    }
}

void BuildDwarfRaceTalentMenu(Player* player, GameObject* object, uint32)
{
    if (!player || !player->IsAlive() || player->IsInCombat())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    uint8 level = player->GetLevel();
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_IRONFORGE), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 2);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_EAGLE_NEXT), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 500);
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_BLACKROCK_MOUNTAIN), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 501);
    if (level > 70)
    AddGossipItemFor(player, GOSSIP_ICON_TAXI, GetTrinityString(TCTSTR_TRANS_ALTER_OF_MAKERS), MW_GOSSIP_SENDER_ACTION_DO_TRANS, MW_GOSSIP_ACTION_TRANS + 502);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, object->GetGUID());
}

void BuildMiniGameMenu(Player*, Item*) {
    /*if (!player || !player->IsAlive() || player->IsInCombat())
        return;

    PlayerMenu* menu = player->PlayerTalkClass;
    uint8 level = player->GetLevel();
    menu->ClearMenus();

    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "????", MW_GOSSIP_ACTION_MINI_GAME, 0);
    AddGossipItemFor(player, GOSSIP_ICON_TALK, GetTrinityString(TCTSTR_BACK), MW_GOSSIP_SENDER_MAIN, MW_GOSSIP_ACTION_SUB_MENU);

    SendGossipMenuFor(player, GOSSIP_ICON_TAXI, item->GetGUID());*/
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
    case MW_GOSSIP_ACTION_MINI_GAME:
        if (action == MW_GOSSIP_ACTION_SUB_MENU) {
            return BuildMiniGameMenu(player, item);
        }
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
