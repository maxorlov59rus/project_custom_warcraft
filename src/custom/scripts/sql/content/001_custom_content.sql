-- ============================================================================
-- Кастомный контент для AzerothCore 3.3.5a — схема таблиц + пример данных.
-- Принципы AC: справочные таблицы в world-БД, привязка скриптов по имени
-- (как ScriptMgr / CreatureAI scripts), загрузка при старте сервера.
-- Диапазон кастомных entry: 900000+ (не пересекается с retail 3.3.5a).
-- ============================================================================

CREATE TABLE IF NOT EXISTS `custom_creature_template` (
  `entry`       INT UNSIGNED NOT NULL,
  `name`        VARCHAR(255) NOT NULL DEFAULT '',
  `minlevel`    TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `maxlevel`    TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `faction`     MEDIUMINT UNSIGNED NOT NULL DEFAULT 14,
  `hpmin`       INT UNSIGNED NOT NULL DEFAULT 100,
  `hpmax`       INT UNSIGNED NOT NULL DEFAULT 100,
  `dmgmin`      INT NOT NULL DEFAULT 1,
  `dmgmax`      INT NOT NULL DEFAULT 2,
  `armor`       INT UNSIGNED NOT NULL DEFAULT 0,
  `speed`       FLOAT NOT NULL DEFAULT 1.0,
  `ainame`      VARCHAR(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Кастомные шаблоны существ';

CREATE TABLE IF NOT EXISTS `custom_creature_spawn` (
  `id`          INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `entry`       INT UNSIGNED NOT NULL,
  `map`         SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `x`           FLOAT NOT NULL DEFAULT 0,
  `y`           FLOAT NOT NULL DEFAULT 0,
  `z`           FLOAT NOT NULL DEFAULT 0,
  `o`           FLOAT NOT NULL DEFAULT 0,
  `phasemask`   INT UNSIGNED NOT NULL DEFAULT 1,
  `curhealth`   INT UNSIGNED NOT NULL DEFAULT 0,
  `scriptname`  VARCHAR(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `idx_entry` (`entry`),
  KEY `idx_map` (`map`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Кастомные спавны существ';

CREATE TABLE IF NOT EXISTS `custom_item_template` (
  `entry`         INT UNSIGNED NOT NULL,
  `name`          VARCHAR(255) NOT NULL DEFAULT '',
  `class`         TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `subclass`      TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `quality`       TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `requiredlevel` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `spellid_1`     MEDIUMINT UNSIGNED NOT NULL DEFAULT 0,
  `buyprice`      INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Кастомные предметы';

CREATE TABLE IF NOT EXISTS `custom_item_source` (
  `id`           INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `itemtype`     TINYINT UNSIGNED NOT NULL DEFAULT 2 COMMENT '0=fish,1=go_loot,2=creature_loot,3=vendor',
  `sourceentry`  INT UNSIGNED NOT NULL,
  `itementry`    INT UNSIGNED NOT NULL,
  `chance`       FLOAT NOT NULL DEFAULT 100,
  `countmin`     INT NOT NULL DEFAULT 1,
  `countmax`     INT NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  KEY `idx_source` (`sourceentry`),
  KEY `idx_item` (`itementry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Источники кастомных предметов';

-- ----------------------------------------------------------------------------
-- Пример контента: элитный редкий эспавн в Восточных Королевских землях
-- ----------------------------------------------------------------------------
INSERT INTO `custom_creature_template`
  (`entry`,`name`,`minlevel`,`maxlevel`,`faction`,`hpmin`,`hpmax`,`dmgmin`,`dmgmax`,`armor`,`speed`,`ainame`)
VALUES
  (900001,'Теневой Охотник Азро', 60, 62, 14, 45000, 45000, 800, 1200, 9000, 1.14, 'custom_shadow_hunter_ai'),
  (900002,'Хранитель Рунного Алтаря', 63, 63, 14, 90000, 90000, 1100, 1600, 12000, 1.0, 'custom_rune_guardian_ai');

INSERT INTO `custom_creature_spawn`
  (`entry`,`map`,`x`,`y`,`z`,`o`,`scriptname`)
VALUES
  (900001, 0, -11032.1, 372.6, 28.4, 1.57, 'custom_npc_shadow_hunter'),
  (900002, 0, -8910.3, -152.9, 81.2, 3.14, 'custom_npc_rune_altar');

INSERT INTO `custom_item_template`
  (`entry`,`name`,`class`,`subclass`,`quality`,`requiredlevel`,`spellid_1`,`buyprice`)
VALUES
  (910001,'Руна Теневого Охотника', 1, 2, 4, 60, 0, 500000),        -- меч, эпик
  (910002,'Эссенция Рунного Хранителя', 0, 5, 3, 1, 88767, 0);      -- consumable с заклинанием

INSERT INTO `custom_item_source`
  (`itemtype`,`sourceentry`,`itementry`,`chance`,`countmin`,`countmax`)
VALUES
  (2, 900001, 910001, 8.0, 1, 1),   -- 8% с Теневого Охотника
  (2, 900002, 910002, 100.0, 2, 5), -- гарантированно 2-5 эссенций
