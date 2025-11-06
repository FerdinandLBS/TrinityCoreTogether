-- Change all occurrences of world_database to your world database name.
-- Change all occurrences of characters_database to your characters database name.

-- This SQL will first store all quest items in a table. The table is temporary and is automatically deleted once SQL client disconnects.
-- It then selects all completed quests of a player and inserts the reward items of that quest to the player account's transmog library.

USE world;


CREATE TABLE IF NOT EXISTS `world`.`transmog_translate` (
  `entry` INT NOT NULL,
  `string` TEXT NULL,
  PRIMARY KEY (`entry`)); 
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('0', '你有未保存的幻化!');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('1', '|TInterface/ICONS/INV_Enchant_Disenchant:30:30:-18:0|t移除未保存的幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('2', '|TInterface/ICONS/Ability_Vanish:30:30:-18:0|t隐藏');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('3', '|TInterface/ICONS/Spell_Holy_Restoration:30:30:-18:0|t恢复原样');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('4', '|TInterface/ICONS/INV_Enchant_Disenchant:30:30:-18:0|t移除未保存的附魔效果');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('5', '|TInterface/ICONS/Ability_Vanish:30:30:-18:0|t隐藏附魔效果');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('6', '|TInterface/ICONS/INV_Misc_Book_11:30:30:-18:0|t幻化是如何工作的');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('7', '|TInterface/RAIDFRAME/UI-RAIDFRAME-MAINASSIST:30:30:-18:0|t管理套装');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('8', '|TInterface/ICONS/INV_Enchant_EssenceCosmicGreater:30:30:-18:0|t保存幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('9', '所有待进行幻化的已装备物品将保留其外观，并变为灵魂绑定、不可退款且不可交易的状态。\n\n您是否希望继续?');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('10', '|TInterface/ICONS/INV_Enchant_Disenchant:30:30:-18:0|t取消幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('11', '取消所有已装备物品的待保存幻化?');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('12', '装备物品将恢复至其原始外观，并带有待处理的更改。\n\n您是否要继续?');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('13', '|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:30:30:-18:0|t更新菜单');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('14', '|TInterface/ICONS/INV_Misc_Book_11:30:30:-18:0|t套装如何工作');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('15', '|TInterface/GuildBankFrame/UI-GuildBankFrame-NewTab:30:30:-18:0|t保存套装 ');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('16', '输入套装名称并点击确定');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('17', '|TInterface/ICONS/Ability_Spy:30:30:-18:0|t返回..');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('18', '|TInterface/ICONS/INV_Misc_Statue_02:30:30:-18:0|t使用套装');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('19', '使用套装将移除待处理的幻化，并将套装中的幻化作为待处理项添加。\n您是否希望继续？\n\n');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('20', '|TInterface/ICONS/INV_Misc_Statue_02:30:30:-18:0|t重命名套装');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('21', '输入一个名字\n\n原名称: ');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('22', '|TInterface/PaperDollInfoFrame/UI-GearManager-LeaveItem-Opaque:30:30:-18:0|t删除套装');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('23', '确认删除[');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('24', ' - 页码 ');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('25', '|TInterface/ICONS/Spell_ChargePositive:30:30:-18:0|t下一页');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('26', '|TInterface/ICONS/Spell_ChargeNegative:30:30:-18:0|t上一页');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('27', '|TInterface/ICONS/Ability_Spy:30:30:-18:0|t主菜单');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('28', '没有装备');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('29', '%s的外观已经加入收集.');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('30','物品已经幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('31','此类物品无法幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('32','这件物品无法被幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('33','鱼竿无法被幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('34','这个稀有度的物品无法幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('35','幻化条件没有被满足');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('36','物品必须拥有属性才能幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('37','你的阵营无法使用这件物品');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('38','你的职业无法使用这件物品');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('39','你的种族无法使用这件物品');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('40','你没有使用该物品所需的熟练度');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('41','你没有使用该物品所需的技能');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('42','你的技能等级不足以使用该物品');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('43','你没有使用该物品所需的法术');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('44','你的等级太低，无法使用该物品');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('45','物品等级太低');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('46','物品等级太高');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('47','物品类型不匹配');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('48','护甲类型不匹配');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('49','武器类型不匹配');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('50','物品装备槽位不允许幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('51','此槽位中的物品无法被幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('52','选择的幻化外观不存在');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('53','该槽位没有装备物品');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('54','你没有待处理的幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('55','交易过程中费用发生变化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('56','金币不足');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('57','套装名称过长或过短');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('58','你装备的物品没有待处理或已应用的幻化');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('59','无法保存更多套装');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('60','套装不存在');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('61','物品不符合附魔要求');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('62'	,'头部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('63'	,'肩部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('64'	,'衬衣');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('65'	,'胸部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('66'	,'腰部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('67'	,'腿部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('68'	,'脚部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('69'	,'腕部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('70'	,'手部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('71'	,'背部');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('72'	,'主手');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('73'	,'副手');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('74'	,'远程武器');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('75'	,'战袍');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('76', '[幻化]');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('77', '[未保存]');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('78', '[附魔]');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('79', '附魔');
INSERT INTO `world`.`transmog_translate` (`entry`, `string`) VALUES ('80', '幻化待生效');


CREATE TEMPORARY TABLE IF NOT EXISTS characters.transmog__quest_reward_items (INDEX(ID)) AS (
SELECT * FROM (
SELECT quest_template.id AS ID, RewardChoiceItemID1 AS RewardItemID FROM quest_template WHERE RewardChoiceItemID1 <> 0 AND RewardChoiceItemQuantity1 <> 0
UNION
SELECT quest_template.id AS ID, RewardChoiceItemID2 AS RewardItemID FROM quest_template WHERE RewardChoiceItemID2 <> 0 AND RewardChoiceItemQuantity2 <> 0
UNION
SELECT quest_template.id AS ID, RewardChoiceItemID3 AS RewardItemID FROM quest_template WHERE RewardChoiceItemID3 <> 0 AND RewardChoiceItemQuantity3 <> 0
UNION
SELECT quest_template.id AS ID, RewardChoiceItemID4 AS RewardItemID FROM quest_template WHERE RewardChoiceItemID4 <> 0 AND RewardChoiceItemQuantity4 <> 0
UNION
SELECT quest_template.id AS ID, RewardChoiceItemID5 AS RewardItemID FROM quest_template WHERE RewardChoiceItemID5 <> 0 AND RewardChoiceItemQuantity5 <> 0
UNION
SELECT quest_template.id AS ID, RewardChoiceItemID6 AS RewardItemID FROM quest_template WHERE RewardChoiceItemID6 <> 0 AND RewardChoiceItemQuantity6 <> 0
UNION
SELECT quest_template.id AS ID, RewardItem1 AS RewardItemID FROM quest_template WHERE RewardItem1 <> 0 AND RewardAmount1 <> 0
UNION
SELECT quest_template.id AS ID, RewardItem2 AS RewardItemID FROM quest_template WHERE RewardItem2 <> 0 AND RewardAmount2 <> 0
UNION
SELECT quest_template.id AS ID, RewardItem3 AS RewardItemID FROM quest_template WHERE RewardItem3 <> 0 AND RewardAmount3 <> 0
UNION
SELECT quest_template.id AS ID, RewardItem4 AS RewardItemID FROM quest_template WHERE RewardItem4 <> 0 AND RewardAmount4 <> 0
) x
);

USE characters;

REPLACE INTO custom_account_transmog (accountid, `type`, entry) SELECT c.account, 0, tqri.RewardItemID FROM transmog__quest_reward_items tqri INNER JOIN character_queststatus_rewarded cqr ON tqri.ID = cqr.quest INNER JOIN characters c ON c.guid = cqr.guid;
