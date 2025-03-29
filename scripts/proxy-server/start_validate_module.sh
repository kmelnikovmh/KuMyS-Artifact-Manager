#!/bin/bash

# Не изменять, ибо надо будет поменять и в nginx
validate_module_port=5000

handle_error() {
    echo -e "\nError: $1" >&2
    exit 1
}

# Переходим в папку проекта
cd proxy-server || handle_error "Failed to cd to proxy-server"

# Проверяем занятность порта
if sudo lsof -i :$validate_module_port > /dev/null; then
    handle_error "Port ${validate_module_port} is already in use"
fi

# Запуск
if ! ./validate_module "${validate_module_port}"; then
    handle_error "Failed to start validation module"
fi