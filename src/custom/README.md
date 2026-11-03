# Кастомный контент-модуль (AzerothCore 3.3.5a)

Модуль подключается как стандартный AC-скрипт-модуль: каталог добавляется
в `src/server/scripts/Custom/CMakeLists.txt` через `AC_ADD_SCRIPT(...)` либо
отдельным `add_subdirectory`.

## Состав

| Файл | Назначение |
|---|---|
| `models/CustomModels.h` | структуры данных кастомных шаблонов/спавнов/предметов |
| `models/CustomContentService.{h,cpp}` | синглтон-реестр, LoadFromDB при старте сервера |
| `scripts/custom_content_scripts.cpp` | BossAI + ItemScript + фабрика AI по aiName |
| `scripts/sql/content/001_custom_content.sql` | схема custom_* таблиц + пример контента |
| `scripts/sql/content/002_vanilla_overrides_example.sql` | паттерн переопределения эталонного контента с бэкапом и откатом |

## Принципы AzerothCore, которые соблюдены

1. **Данные в БД, логика в C++.** Справочники грузятся один раз при старте
   (`LoadFromDB`), рантайм только читает — как `sCreatureMgr`, `sItemMgr`.
2. **Привязка скриптов по имени.** `scriptname`/`ainame` из БД → фабрика,
   как `ScriptMgr::CreateScript` и `AiName` в `creature_template`.
3. **Кастомные entry вне ванильного диапазона** (900000+), чтобы не
   конфликтовать с оригинальными данными 3.3.5a.
4. **Переопределение эталона — отдельными миграциями** с регистрацией
   apply/revert и автоматическим backup оригинальных значений (триггер).
   Ванильные таблицы остаются «эталонными» и воспроизводимыми.

## Порядок применения

```bash
mysql -u root -p azerothcore_world < src/custom/scripts/sql/content/001_custom_content.sql
mysql -u root -p azerothcore_world < src/custom/scripts/sql/content/002_vanilla_overrides_example.sql
# пересборка: ./acore.sh build world
```
