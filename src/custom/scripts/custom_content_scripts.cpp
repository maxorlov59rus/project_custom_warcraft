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
// AI кастомного босс-существа «Хранитель Рунного Алтаря» (entry 900002).
// Паттерн AC: enum spell -> FillInitialPowers -> UpdateInstanceDifficulty не
// нужен для custom entry, фазы считаются через таймеры ScriptedAI.
// ============================================================================

enum RuneGuardianSpells
{
    SPELL_RUNE_NOVA      = 88767, // существующий ванильный аналог (не создаём новый ДБ-объект)
    SPELL_GUARDIAN_GAZE  = 990001, // placeholder кастомного дебаффа (проверяется при старте)
};

class custom_rune_guardian_ai : public BossAI
{
public:
    custom_rune_guardian_ai(Creature* creature) : BossAI(creature, 0) {}

    void Reset() override
    {
        _Reset();
        events.ScheduleEvent(EVENT_RUNE_NOVA, 8s);
        events.ScheduleEvent(EVENT_GUARDIAN_GAZE, 15s);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _JustEngagedWith();
        Talk(EMOTE_AGGRO);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateAIInternal(diff)) // AC helper: canMoveInStunned etc checks
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_RUNE_NOVA:
                    DoCastAOE(SPELL_RUNE_NOVA);
                    events.Repeat(10s);
                    break;
                case EVENT_GUARDIAN_GAZE:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 30.0f, true))
                        DoCast(target, SPELL_GUARDIAN_GAZE);
                    events.Repeat(20s);
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventCategory _unused{};
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
// Регистрация. В AC используется ScriptLoader (AddSC_*); здесь — локальная
// фабрика, вызываемая из bootstrap-модуля при старте worldserver.
// ============================================================================

void AddCustomScripts_SpellScript()
{
    new custom_item_rune_essence();
}

// Фабрика AI по aiName из custom_creature_template (привязка как в AC:
// AiName -> factory map в CreatureAIRegistry).
CreatureAI* CreateCustomCreatureAI(Creature* creature)
{
    if (const CustomCreatureTemplateDef* def = sCustomContent->GetCreatureTemplate(creature->GetEntry()))
    {
        if (def->aiName == "custom_rune_guardian_ai")
            return new custom_rune_guardian_ai(creature);
        // новые AI добавляются сюда по мере появления контента
    }
    return nullptr; // nullptr => стандартный AI по AIId (поведение AC)
}

void RegisterCustomAIFactory()
{
    // В форке подключается одной строкой в CreatureAIRegistry до старта карт.
    // Пример псевдокода AC:
    //   CreatureAIRegistry::RegisterFactory("custom_rune_guardian_ai", ...);
}
