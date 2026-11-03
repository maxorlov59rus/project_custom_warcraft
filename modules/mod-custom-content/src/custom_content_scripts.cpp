/* This file is dedicated to custom content scripts.
 * Структура повторяет стандартный паттерн AzerothCore:
 *   - класс наследует Script AI + PlayerScript/CreatureScript/GOScript;
 *   - фабрика RegisterCustomScript привязывает скрипт по имени из БД
 *     (scriptname в custom_creature_spawn), как это делает ScriptMgr::CreateScript.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "CustomContentService.h"

// ============================================================================
// Заклинания кастомного босс-существа «Хранитель Рунного Алтаря» (entry 900002).
// Паттерн AC: enum spell -> таймеры EventMap в ScriptedAI.
// ============================================================================

enum RuneGuardianSpells
{
    SPELL_RUNE_NOVA      = 88767, // существующий аналог из БД заклинаний (не создаём новый ДБ-объект)
    SPELL_GUARDIAN_GAZE  = 990001, // placeholder кастомного дебаффа (проверяется при старте)
};

// ============================================================================
// Спелл-скрипт кастомного предмета «Эссенция Рунного Хранителя»:
// on-use бафф по принципам ItemScript в AC (см. src/server/scripts/Spells).
// ============================================================================

class custom_item_rune_essence : public ItemScript
{
public:
    custom_item_rune_essence() : ItemScript("custom_item_rune_essence") {}

    // триггер USE (SPELLTRIGGER_USE, как в AC item_template.spell_trigger_1)
    bool OnUse(Player* player, Item* /*item*/, SpellCustomEffectsModification& /*modifiers*/) override
    {
        player->CastSpell(player, 88767, /*triggered*/ true);
        return true;
    }
};

// ============================================================================
// Скрипт-обёртка для ScriptMgr: кастомные существа управляются через
// CreatureScript (привязка по entry из custom_creature_spawn), что является
// стандартным способом добавлять поведение существам без правки ядра —
// так работают почти все модули в modules/ дерева AzerothCore.
// ============================================================================

class custom_content_creature_script : public CreatureScript
{
public:
    custom_content_creature_script() : CreatureScript("custom_content_creature_script") {}

    struct CustomContentCreatureAI : public ScriptedAI
    {
        CustomContentCreatureAI(Creature* creature) : ScriptedAI(creature)
        {
            // Если у шаблона задан ainame с известной фабрикой — используем его,
            // иначе стандартное ScriptedAI-поведение (паттерн AC: fallback).
        }

        void Reset() override
        {
            _events.Reset();
            const CustomCreatureTemplateDef* def = sCustomContent->GetCreatureTemplate(creature->GetEntry());
            if (def && def->aiName == "custom_rune_guardian_ai")
            {
                _isRuneGuardian = true;
                _events.ScheduleEvent(EVENT_RUNE_NOVA, 8000);
                _events.ScheduleEvent(EVENT_GUARDIAN_GAZE, 15000);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateAIInternal(diff))
                return;

            if (!_isRuneGuardian)
            {
                DoMeleeAttackIfReady();
                return;
            }

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RUNE_NOVA:
                        DoCastAOE(SPELL_RUNE_NOVA);
                        _events.Repeat(10000);
                        break;
                    case EVENT_GUARDIAN_GAZE:
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 30.0f, true))
                            DoCast(target, SPELL_GUARDIAN_GAZE);
                        _events.Repeat(20000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        enum Events { EVENT_RUNE_NOVA = 1, EVENT_GUARDIAN_GAZE = 2 };
        EventMap _events;
        bool _isRuneGuardian = false;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        // Привязка только к кастомным entry из нашего реестра — ванильные
        // существа эталонного контента не затрагиваются.
        if (sCustomContent->GetCreatureTemplate(creature->GetEntry()))
            return new CustomContentCreatureAI(creature);
        return nullptr;
    }
};

// ============================================================================
// Точка регистрации модуля. Вызывается из MP_loader.cpp
// (Addmod_custom_contentScripts), который генерируется CMake-загрузчиком
// ModulesLoader.cpp дерева AzerothCore.
// ============================================================================

void AddCustomContentScripts()
{
    // 1. Загрузка справочников кастомного контента из БД (принцип AC:
    //    данные грузятся один раз при старте, рантайм только читает).
    sCustomContent->LoadFromDB();

    // 2. Регистрация скриптов в ScriptMgr (ItemScript + CreatureScript).
    new custom_item_rune_essence();
    new custom_content_creature_script();
}
