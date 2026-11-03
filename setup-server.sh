#!/usr/bin/env bash
# ============================================================================
# Клонирование эталонного серверного ПО AzerothCore (WoW 3.3.5a) и установка
# наших модулей из папки modules/.
#
# Использование (из корня этого репозитория):
#   ./setup-server.sh [каталог_назначения]   # по умолчанию ../azerothcore-wotlk
#
# Шаги:
#   1. git clone https://github.com/azerothcore/azerothcore-wotlk (ветка master = 3.3.5a)
#   2. копирование каждого модуля из modules/ в <форк>/modules/
#   3. запуск install.sh каждого модуля внутри дерева форка
#   4. конфигурация и сборка через acore.sh
# ============================================================================

set -e

AC_REPO_URL="https://github.com/azerothcore/azerothcore-wotlk"
AC_BRANCH="master"                      # master AzerothCore — это база 3.3.5a
TARGET_DIR="${1:-../azerothcore-wotlk}"
MODULES_SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ----------------------------------------------------------------------------
# 1. Эталонное серверное ПО
# ----------------------------------------------------------------------------
if [ ! -d "$TARGET_DIR/.git" ]; then
    echo "==> Клонирование эталонного AzerothCore в $TARGET_DIR ..."
    git clone --branch "$AC_BRANCH" --single-branch "$AC_REPO_URL" "$TARGET_DIR"
else
    echo "==> Форк уже существует в $TARGET_DIR, обновляем..."
    git -C "$TARGET_DIR" pull --ff-only || true
fi

# ----------------------------------------------------------------------------
# 2..3. Установка наших модулей
# ----------------------------------------------------------------------------
for module_dir in "$MODULES_SRC_DIR"/modules/*/; do
    module_name="$(basename "$module_dir")"
    dest="$TARGET_DIR/modules/$module_name"

    echo "==> Установка модуля: $module_name"
    mkdir -p "$dest"
    rsync -a --delete \
        --exclude '.git' \
        "$module_dir" "$dest/"

    if [ -f "$dest/install.sh" ]; then
        chmod +x "$dest/install.sh"
        ( cd "$TARGET_DIR" && AC_MODULES_INSTALL_DIR="$TARGET_DIR" "./modules/$module_name/install.sh" )
    else
        echo "    (нет install.sh — модуль подключается только через CMakeLists.txt)"
    fi
done

# ----------------------------------------------------------------------------
# 4. Конфигурация и сборка
# ----------------------------------------------------------------------------
echo ""
echo "== Готово. Дальнейшие шаги в форке =="
cat <<EOF
  cd $TARGET_DIR
  cp /dev/null conf/common.local.conf 2>/dev/null; ./acore.sh runner auth   # или редактор конфига
  ./acore.sh cmake          # генерация с учётом modules/
  ./acore.sh build world    # сборка worldserver с нашими модулями
  ./acore.sh run-worldserver
EOF
