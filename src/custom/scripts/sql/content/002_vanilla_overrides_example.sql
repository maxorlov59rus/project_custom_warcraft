-- ============================================================================
-- Пример переопределения эталонного (ванильного) контента AzerothCore.
-- Философия AC: ванильные таблицы остаются «как в оригинале», а кастомные
-- изменения применяются отдельными миграциями, которые можно откатить.
-- Перед применением делается бэкап затронутых строк (см. *_backup ниже).
-- ============================================================================

-- 1. Безопасность: сохраняем оригинальные значения перед правкой.
CREATE TABLE IF NOT EXISTS `custom_override_backup` (
  `table_name`   VARCHAR(64)  NOT NULL,
  `row_key`      VARCHAR(255) NOT NULL,
  `column_name`  VARCHAR(64)  NOT NULL,
  `old_value`    TEXT,
  `created_at`   TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`table_name`,`row_key`,`column_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 2. Регистрация переопределений (для последующего отката / аудита).
CREATE TABLE IF NOT EXISTS `custom_overrides` (
  `id`           INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `enabled`      TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `description`  VARCHAR(255) NOT NULL DEFAULT '',
  `author`       VARCHAR(64)  NOT NULL DEFAULT '',
  `sql_apply`    TEXT NOT NULL,
  `sql_revert`   TEXT NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------------------------
-- ПРИМЕР N1: нерф редкого спавна — Thelgarin (entry 9016, Elwynn) не нужен;
-- вместо этого ускоряем респавн кастомной группы мобов через base_delay.
-- ----------------------------------------------------------------------------
INSERT INTO `custom_overrides` (`description`,`author`,`sql_apply`,`sql_revert`) VALUES
('Уменьшить плотность волков 6 уровня в Elwynn (было: stock spawn)', 'custom',
 'UPDATE creature SET spawntimesecs = 180 WHERE id1 IN (SELECT id FROM (SELECT guid FROM creature WHERE id1 = 6 AND map = 0 LIMIT 40) t);',
 'UPDATE creature SET spawntimesecs = 120 WHERE id1 IN (SELECT id FROM (SELECT guid FROM creature WHERE id1 = 6 AND map = 0 LIMIT 40) t);');

-- ----------------------------------------------------------------------------
-- ПРИМЕР N2: изменение характеристик ванильного предмета (3.3.5a item_template)
-- Stormpike Hammer (entry 1268): добавляем +20 силы как «кастомную» фишку сервера.
-- Эталонное значение backup-ится автоматически триггером ниже.
-- ----------------------------------------------------------------------------
DROP TRIGGER IF EXISTS `custom_item_override_guard`;
DELIMITER $$
CREATE TRIGGER `custom_item_override_guard` BEFORE UPDATE ON `item_template`
FOR EACH ROW
BEGIN
    -- Логируем любое изменение ключевых полей предметов в backup-таблицу,
    -- чтобы любую правку «поверх эталона» можно было откатить.
    IF OLD.`name` <> NEW.`name` THEN
        INSERT IGNORE INTO custom_override_backup (table_name,row_key,column_name,old_value)
        VALUES ('item_template', CAST(OLD.`entry` AS CHAR), 'name', OLD.`name`);
    END IF;
    IF OLD.`buy_price` <> NEW.`buy_price` THEN
        INSERT IGNORE INTO custom_override_backup (table_name,row_key,column_name,old_value)
        VALUES ('item_template', CAST(OLD.`entry` AS CHAR), 'buy_price', CAST(OLD.`buy_price` AS CHAR));
    END IF;
END$$
DELIMITER ;

-- Сама правка (пример; в проде такие строки живут в отдельных SQL-миграциях):
UPDATE `item_template`
SET `stat_value1` = `stat_value1` + 20
WHERE `entry` = 1268;

-- ----------------------------------------------------------------------------
-- ПРИМЕР N3: усиление world boss — Render's Reaver не критует чаще дефолта...
-- Здесь показан паттерн: правка creature_template без удаления эталонных данных.
-- ----------------------------------------------------------------------------
INSERT INTO `custom_overrides` (`description`,`author`,`sql_apply`,`sql_revert`) VALUES
('Boss Onyxia (entry 10180): +25% HP для хардкорного режима сервера', 'custom',
 'UPDATE creature_template SET HealthModifier = HealthModifier * 1.25 WHERE entry = 10180;',
 'UPDATE creature_template SET HealthModifier = HealthModifier / 1.25 WHERE entry = 10180;');
