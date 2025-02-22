#!/bin/bash -efu

CLANG_FORMAT=$(which clang-format)
CLANG_FORMAT_CONFIG="./.clang-format"

if [ -z "$CLANG_FORMAT" ]; then
    echo "clang-format не установлен, вы проиграли :((("
    exit 1
fi

if [ ! -f "$CLANG_FORMAT_CONFIG" ]; then
    echo ".clang-format не найден, вы проиграли :((("
    exit 1
fi

echo "Запустили форматирование ... "
find "./src" "./include" -name "*.cpp" -o -name "*.hpp" | xargs "$CLANG_FORMAT" -i -style=file

ehco "Форматирование окончено!"