#!/bin/bash

# Не изменять validate_module_port, ибо надо будет поменять и в nginx
validate_module_port=63350
blocked_ips_filepath=/kumys/proxy-server/admin-assets/blocked-ip-list.txt

handle_error() {
    echo -e "\nError: $1" >&2
    exit 1
}

# Переходим в папку проекта
cd proxy-server/build || handle_error "Failed to cd to proxy-server"

# Проверяем занятность порта
if sudo lsof -i :$validate_module_port > /dev/null; then
    handle_error "Port ${validate_module_port} is already in use"
fi

# Запуск
if ! ./validate-module "${validate_module_port}" "${blocked_ips_filepath}"; then
    handle_error "Failed to start validation module"
fi