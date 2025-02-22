#!/bin/bash -efu

CLANG_FORMAT=$(which clang-format)
CLANG_FORMAT_CONFIG="./.clang-format"
MAIN_SERVER="main-server"

if [ -z "$CLANG_FORMAT" ]; then
    echo "clang-format не установлен, вы проиграли :((("
    exit 1
fi

if [ ! -f "$CLANG_FORMAT_CONFIG" ]; then
    echo ".clang-format не найден, вы проиграли :((("
    exit 1
fi

echo "Запустили форматирование main-server... "
find "${MAIN_SERVER}/src" "${MAIN_SERVER}/include" -name "*.cpp" -o -name "*.hpp" | xargs "$CLANG_FORMAT" -i -style=file

echo "Форматирование окончено!"