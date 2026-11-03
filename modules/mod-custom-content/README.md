# mod-custom-content — кастомный контент для AzerothCore (WoW 3.3.5a)

Модуль формата AzerothCore (`modules/mod-*`) с собственными моделями и игровым
контентом, созданными по принципам эталонного ядра AC. Серверное ПО берётся из
https://github.com/azerothcore/azerothcore-wotlk (см. `setup-server.sh` в корне
репозитория).

## Структура модуля (по конвенции skeleton-module / elven)

```
modules/mod-custom-content/
├── include.sh                          # точка входа CMake-интеграции (пустая = чистый new API)
├── src/
│   ├── MP_loader.cpp                   # Addmod_custom_contentScripts() — генерируемая AC загрузчиком точка входа
│   └── custom_content_scripts.cpp      # CreatureScript + ItemScript: логика кастомных существ и предметов
├── scripts/
│   ├── models/
│   │   ├── CustomModels.h              # структуры данных (аналоги creature_template/item_template)
│   │   ├── CustomContentService.{h,cpp}# синглтон-реестр, LoadFromDB при старте (принцип AC)
│   │   └── ...
│   └── sql/
│       ├── updates/                    # миграции схемы custom_* таблиц (копируются в pending_db_world)
│       └── overrides/                  # правки «поверх эталона» с backup/revert (применяются вручную)
└── README.md
```

## Принципы AzerothCore, которые соблюдены

1. **Формат модуля**: папка внутри `modules/`, загрузчик `Add<module>Scripts()`
   вызывается автоматически из генерируемого `ModulesLoader.cpp` (CMake собирает
   все `.cpp` модуля статически в цель `modules`). Никаких патчей ядра.
2. **Данные в БД, логика в C++**: справочники грузятся один раз при старте
   (`LoadFromDB`), рантайм только читает — как `sObjectMgr`, `sItemMgr`.
3. **Привязка скриптов по имени**: `CreatureScript`/`ItemScript` регистрируются
   в `ScriptMgr`; AI выбирается по `ainame` из таблицы шаблонов.
4. **Кастомные entry вне ванильного диапазона** (900000+) — не конфликтуют с
   оригинальными данными 3.3.5a.
5. **Переопределение эталона — отдельными скриптами** с регистрацией
   apply/revert и автоматическим backup оригинальных значений (триггер).
   Ванильные таблицы остаются эталонными и воспроизводимыми.

## Установка

Из корня этого репозитория:

```bash
./setup-server.sh            # клонирует azerothcore-wotlk и установит все модули из modules/
cd ../azerothcore-wotlk
./acore.sh cmake && ./acore.sh build world
```

SQL миграция схемы (`scripts/sql/updates/*.sql`) применяется автоматически при
старте worldserver через механизм pending_db_world. Override-скрипты
(`scripts/sql/overrides/`) применяются вручную после ревью:

```bash
mysql -u root -p azerothcore_world < modules/mod-custom-content/scripts/sql/overrides/001-vanilla-overrides-example.sql
```
