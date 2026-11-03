#include "CustomContentService.h"
#include "Log/Log.h"
#include "Database/DatabaseEnv.h"

// В реальном AzerothCore-форке здесь используется PreparedQueryResult через
// SqlStorage. Для переносимости этого модуля применяем прямой запрос к
// кастомным таблицам (custom_*), созданным в scripts/sql/content/.

CustomContentService* CustomContentService::instance()
{
    static CustomContentService sInstance;
    return &sInstance;
}

bool CustomContentService::LoadFromDB()
{
    _creatureTemplates.clear();
    _items.clear();
    _spawnsByMap.clear();
    _vendorSources.clear();
    _lootSources.clear();

    WorldDatabase& worldDb = WorldDatabase::GetStore(); // AC-style accessor

    // ---- custom_creature_template ----
    if (QueryResult res = worldQuery.customCreatureTemplate.Load())
    {
        do
        {
            Field* f = res->Fetch();
            CustomCreatureTemplateDef def;
            def.entry     = f[0].GetUInt32();
            def.name      = f[1].GetString();
            def.minLevel  = f[2].GetUInt32();
            def.maxLevel  = f[3].GetUInt32();
            def.faction   = f[4].GetUInt32();
            def.hpMin     = f[5].GetUInt32();
            def.hpMax     = f[6].GetUInt32();
            def.dmgMin    = f[7].GetInt32();
            def.dmgMax    = f[8].GetInt32();
            def.armor     = f[9].GetUInt32();
            def.speed     = f[10].GetFloat();
            def.aiName    = f[11].GetString();

            if (!def.entry)
            {
                LOG_ERROR("custom", "custom_creature_template: entry=0, пропущено");
                continue;
            }
            if (def.minLevel > def.maxLevel)
                LOG_ERROR("custom", "custom_creature_template entry %u: minLevel > maxLevel", def.entry);

            _creatureTemplates.emplace(def.entry, std::move(def));
        } while (res->NextRow());
    }

    // ---- custom_creature_spawn ----
    if (QueryResult res = worldQuery.customCreatureSpawn.Load())
    {
        do
        {
            Field* f = res->Fetch();
            CustomCreatureSpawnDef spawn;
            spawn.spawnId     = f[0].GetUInt32();
            spawn.entry       = f[1].GetUInt32();
            spawn.map         = f[2].GetUInt32();
            spawn.posX        = f[3].GetFloat();
            spawn.posY        = f[4].GetFloat();
            spawn.posZ        = f[5].GetFloat();
            spawn.orientation = f[6].GetFloat();
            spawn.phaseMask   = f[7].GetUInt32();
            spawn.curHealth   = f[8].GetUInt32();
            spawn.scriptName  = f[9].GetString();

            if (!_creatureTemplates.contains(spawn.entry))
            {
                LOG_ERROR("custom", "custom_creature_spawn id=%u ссылается на неизвестный entry=%u",
                          spawn.spawnId, spawn.entry);
                continue;
            }
            _spawnsByMap[spawn.map].push_back(std::move(spawn));
        } while (res->NextRow());
    }

    // ---- custom_item_template ----
    if (QueryResult res = worldQuery.customItemTemplate.Load())
    {
        do
        {
            Field* f = res->Fetch();
            CustomItemDef def;
            def.entry        = f[0].GetUInt32();
            def.name         = f[1].GetString();
            def.class_       = f[2].GetUInt32();
            def.subclass     = f[3].GetUInt32();
            def.quality      = f[4].GetUInt8();
            def.requiredLevel= f[5].GetUInt32();
            def.spellId_1    = f[6].GetUInt32();
            def.buyPrice     = f[7].GetUInt32();
            _items.emplace(def.entry, std::move(def));
        } while (res->NextRow());
    }

    // ---- custom_item_source ----
    if (QueryResult res = worldQuery.customItemSource.Load())
    {
        do
        {
            Field* f = res->Fetch();
            CustomItemSourceDef src;
            src.id          = f[0].GetUInt32();
            src.itemType    = f[1].GetUInt32();
            src.sourceEntry = f[2].GetUInt32();
            src.itemEntry   = f[3].GetUInt32();
            src.chance      = f[4].GetFloat();
            src.countMin    = f[5].GetInt32();
            src.countMax    = f[6].GetInt32();

            if (!_items.contains(src.itemEntry))
            {
                LOG_ERROR("custom", "custom_item_source id=%u: неизвестный item=%u", src.id, src.itemEntry);
                continue;
            }
            if (src.itemType == 3)
                _vendorSources[src.sourceEntry].push_back(std::move(src));
            else
                _lootSources[src.sourceEntry].push_back(std::move(src));
        } while (res->NextRow());
    }

    _loaded = true;
    LOG_INFO("custom", "Загружено кастомного контента: шаблонов существ=%zu, спавнов карт=%zu, предметов=%zu",
             _creatureTemplates.size(), _spawnsByMap.size(), _items.size());
    return true;
}

const CustomCreatureTemplateDef* CustomContentService::GetCreatureTemplate(uint32 entry) const
{
    auto it = _creatureTemplates.find(entry);
    return it != _creatureTemplates.end() ? &it->second : nullptr;
}

const CustomItemDef* CustomContentService::GetItemDef(uint32 entry) const
{
    auto it = _items.find(entry);
    return it != _items.end() ? &it->second : nullptr;
}

static const std::vector<CustomCreatureSpawnDef> g_emptySpawns;
static const std::vector<CustomItemSourceDef>    g_emptySources;

const std::vector<CustomCreatureSpawnDef>& CustomContentService::GetSpawnsForMap(uint32 mapId) const
{
    auto it = _spawnsByMap.find(mapId);
    return it != _spawnsByMap.end() ? it->second : g_emptySpawns;
}

const std::vector<CustomItemSourceDef>& CustomContentService::GetSourcesForVendor(uint32 vendorEntry) const
{
    auto it = _vendorSources.find(vendorEntry);
    return it != _vendorSources.end() ? it->second : g_emptySources;
}

const std::vector<CustomItemSourceDef>& CustomContentService::GetSourcesForLoot(uint32 sourceEntry) const
{
    auto it = _lootSources.find(sourceEntry);
    return it != _lootSources.end() ? it->second : g_emptySources;
}
