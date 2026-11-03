#pragma once

#include "ObjectGuid.h"
#include "SharedDefines.h"
#include <vector>
#include <string>

// ============================================================================
// Custom Models — структуры данных кастомного контента.
// Повторяют принципы AzerothCore: данные грузятся из БД в статические
// контейнеры при старте сервера, логика обращается к ним только через
// сервис-синглтон (см. CustomContentService).
// ============================================================================

// Кастомный спавн NPC/моба (аналог creature + creature_template, но своим ключом)
struct CustomCreatureSpawnDef
{
    uint32    spawnId      = 0;
    uint32    entry        = 0;       // custom_creature_template.entry
    uint32    map          = 0;
    float     posX         = 0.f;
    float     posY         = 0.f;
    float     posZ         = 0.f;
    float     orientation  = 0.f;
    uint32    phaseMask    = 1;
    uint32    curHealth    = 0;       // 0 = использовать maxhealth из шаблона
    std::string scriptName;           // имя ScriptScript C++-скрипта (EmptyCallContext name)
};

// Шаблон кастомного существа — аналог creature_template (3.3.5a)
struct CustomCreatureTemplateDef
{
    uint32 entry = 0;
    uint32 type  = TYPE_NORMAL;       // unit_type как в AC
    std::string name;
    uint32 minLevel = 1;
    uint32 maxLevel = 1;
    uint32 faction  = 14;             // faction template id
    uint32 flags    = 0;              // UnitFlags
    uint32 family   = CREATURE_FAMILY_NONE;
    uint32 rank     = NORMAL_CREATURE;
    int32  dmgMin   = 1;
    int32  dmgMax   = 2;
    uint32 armor    = 0;
    uint32 hpMin    = 100;
    uint32 hpMax    = 100;
    float  speed    = 1.0f;
    uint32 AIId     = 0;              // id встроенного AI по принципам AC
    std::string aiName;               // C++ AI factory name
};

// Кастомный предмет — аналог item_template (сокращённый, расширяется по мере надобности)
struct CustomItemDef
{
    uint32 entry      = 0;
    std::string name;
    uint32 class_     = ITEM_CLASS_CONSUMABLE; // ItemClass
    uint32 subclass   = 0;
    uint8  quality    = ITEM_QUALITY_UNCOMMON;
    uint8  inventory  = 0;                     // INVTYPE_*
    uint32 requiredLevel = 1;
    uint32 statType1  = 0;                     // ItemPropertyType
    int32  statValue1 = 0;
    uint32 spellId_1  = 0;                     // on-use / proc spell
    uint8  spellTrigger_1 = 0;                 // ItemSpellTrigger (AC: SPELLTRIGGER_*)
    uint32 maxCount   = 0;
    uint32 stackCount = 1;
    uint32 buyPrice   = 0;                     // медь, как в AC
    uint32 bonding    = 0;                     // ITEM_BONDING_*
};

// Кастомное существование предмета в мире: лут-контейнеры и торговцы
struct CustomItemSourceDef
{
    uint32 id          = 0;
    uint32 itemType    = 0;   // 0=fishingpool, 1=gameobject_loot, 2=creature_loot, 3=vendor
    uint32 sourceEntry = 0;   // entry GO/creature/vendor
    uint32 itemEntry   = 0;
    float  chance      = 100.f;
    int32  countMin    = 1;
    int32  countMax    = 1;
};

// Простая кастомная «валюта»/счётчик игрока (аналог принципа playercreateinfo_*
// и custom tables поверх character_* таблиц AC)
struct CustomPlayerFlagDef
{
    uint32 charGuidLow = 0;
    uint32 flagId      = 0;
    uint32 value       = 0;
};
