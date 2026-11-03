/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

// Точка входа модуля по конвенции AzerothCore (см. skeleton-module):
// CMake генерирует ModulesLoader.cpp, который вызывает
// Add<ИмяМодуляСПодчёркиваниями>Scripts() из каждой статически линкуемой
// директории modules/*. Имя функции ДОЛЖНО совпадать с именем папки модуля,
// где дефисы заменены на подчёркивания: mod-custom-content -> Addmod_custom_contentScripts().

// Из custom_content_scripts.cpp — скрипты, AI и сервис кастомного контента
void AddCustomContentScripts();

void Addmod_custom_contentScripts()
{
    AddCustomContentScripts();
}
