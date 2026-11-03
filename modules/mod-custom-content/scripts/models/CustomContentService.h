#pragma once

#include "CustomModels.h"
#include <unordered_map>
#include <vector>

// ============================================================================
// CustomContentService — синглтон-реестр кастомного контента.
//
// Принцип AzerothCore: все справочные данные грузятся один раз при старте
// (LoadFromDB) в память сервера; рантайм-логика только читает реестры.
// Аналогично ActionScripts/ScriptS: скрипты регистрируются фабриками и
// привязываются по имени (scriptName / aiName).
// ============================================================================

class CustomContentService
{
public:
    static CustomContentService* instance();

    // Загрузка из БД (вызывается из World::setInitialWorldDataUpdates либо
    // из AddScrpts bootstrap на старте). Возвращает false при ошибке SQL.
    bool LoadFromDB();

    // --- Реестры шаблонов ---
    const CustomCreatureTemplateDef* GetCreatureTemplate(uint32 entry) const;
    const CustomItemDef*             GetItemDef(uint32 entry) const;

    // --- Спавны, сгруппированные по карте (аналог Map::LoadCreatures) ---
    const std::vector<CustomCreatureSpawnDef>& GetSpawnsForMap(uint32 mapId) const;

    // --- Источники предметов ---
    const std::vector<CustomItemSourceDef>& GetSourcesForVendor(uint32 vendorEntry) const;
    const std::vector<CustomItemSourceDef>& GetSourcesForLoot(uint32 sourceEntry) const;

    bool IsLoaded() const { return _loaded; }

private:
    CustomContentService() = default;

    std::unordered_map<uint32, CustomCreatureTemplateDef> _creatureTemplates;
    std::unordered_map<uint32, CustomItemDef>             _items;
    std::unordered_map<uint32 /*map*/, std::vector<CustomCreatureSpawnDef>> _spawnsByMap;
    std::unordered_map<uint32 /*vendor*/, std::vector<CustomItemSourceDef>> _vendorSources;
    std::unordered_map<uint32 /*loot src*/, std::vector<CustomItemSourceDef>> _lootSources;

    static constexpr uint32 EMPTY_LIST_LIMIT = 0;
    bool _loaded = false;
};

#define sCustomContent CustomContentService::instance()
