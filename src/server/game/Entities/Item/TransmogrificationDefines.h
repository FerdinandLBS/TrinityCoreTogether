#ifndef DEF_TRANSMOGRIFICATION_DEFINES_H
#define DEF_TRANSMOGRIFICATION_DEFINES_H

#include "Define.h"
#include <limits>

enum AppearanceType : uint32
{
    TRANSMOG_TYPE_ITEM,
    TRANSMOG_TYPE_ENCHANT,
    TRANSMOG_TYPE_COUNT,
};

enum TransmogResult
{
    TransmogResult_Ok = 0, // expected to be 0 while others are expected not to be 0
    TransmogResult_InvalidItemType,
    TransmogResult_ItemBlocked,
    TransmogResult_FishingPoleBlocked,
    TransmogResult_InvalidItemQuality,
    TransmogResult_RequiredEventNotActive,
    TransmogResult_ItemMustHaveStats,
    TransmogResult_InvalidFaction,
    TransmogResult_InvalidClass,
    TransmogResult_InvalidRace,
    TransmogResult_MissingProfiency,
    TransmogResult_MissingSkill,
    TransmogResult_TooLowSkill,
    TransmogResult_MissingSpell,
    TransmogResult_TooLowLevelPlayer,
    TransmogResult_TooLowLevelItem,
    TransmogResult_TooHighLevelItem,
    TransmogResult_ItemTypesDontMatch,
    TransmogResult_ArmorTypesDontMatch,
    TransmogResult_WeaponTypesDontMatch,
    TransmogResult_EquipSlotsDontMatch,
    TransmogResult_InvalidSlot,
    TransmogResult_NonexistantTransmog,
    TransmogResult_EmptySlot,
    TransmogResult_NoPendingTarnsmogs,
    TransmogResult_CostChangedDuringTransaction,
    TransmogResult_NotEnoughMoney,
    TransmogResult_TooLongSetName,
    TransmogResult_NoTransmogrifications,
    TransmogResult_AtMaxSets,
    TransmogResult_NonexistantSet,
    TransmogResult_ItemNotFitForEnchantRequirements,
};

const char* CanTransmogrifyResultMessage(TransmogResult result);

const uint32 InvisibleEntry = std::numeric_limits<uint32>::max();
const uint32 NormalEntry = InvisibleEntry - 1;
const uint32 RemovePending = NormalEntry - 1;
const uint32 AbsoluteMaxSets = 25;

class Item;
typedef std::vector<std::tuple<Item* /*item*/, uint32 /*transmog*/, AppearanceType /*transmog type*/>> PendingTransmogs;
typedef std::vector<std::tuple<uint8 /*slot*/, uint32 /*transmog*/, AppearanceType /*transmog type*/>> SetTransmogs;

#endif
