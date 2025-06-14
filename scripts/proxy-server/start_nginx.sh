#!/bin/bash

nginx_port=63380
admin_panel_port=63340
validate_module_port=63350
buffer_listener_nginx_port=63360

handle_error() {
    echo -e "\nError: $1" >&2
    exit 1
}

# Обработка аргумента stop
if [ "$1" == "stop" ]; then
    if ! openresty -s quit; then
        handle_error "Failed to stop OpenResty gracefully\n\n"
    fi
    echo -e "Proxy server (nginx) stopped.\nDon't forget to stop other proxy server modules.\n\n"
    exit 0
fi


# Ищем путь конфигурационного файла nginx
script_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(dirname "$(dirname "$script_root")")"
config_file="$project_root/proxy-server/nginx.conf"
assets_dir="$project_root/proxy-server/admin-assets/"

# Переходим в папку с конфигурационным файлом nginx
cd proxy-server || handle_error "Failed to cd to proxy-server\n\n"

# Проверяем занятность порта
if sudo lsof -i :$nginx_port > /dev/null; then
    handle_error "Port ${nginx_port} is already in use\n\n"
fi

# Определяем ip сервера
get_current_ip() {
    local ip
    #MacOS
    ip=$(ifconfig en0 2>/dev/null | awk '/inet / {print $2}')
        [ -n "$ip" ] && echo "$ip" && return
    #Ubuntu
    ip=$(ifconfig eth0 2>/dev/null | awk '/inet / {print $2}')
        [ -n "$ip" ] && echo "$ip" && return
    echo "Could not determine IP" >&2
    return 1
}
current_ip=$(get_current_ip) || exit 1

# Меняем порты в конфигурационном файле на константы из скрипта
cp "$config_file" "${config_file}.bak" || handle_error "Backup failed\n\n"  # Резервная копия
sed -i.bak \
    -e "s|listen [0-9]\+;[[:space:]]*# request from client|listen ${nginx_port}; # request from client|" \
    -e "s|proxy_pass http://127.0.0.1:[0-9]\+/;[[:space:]]*# admin_panel|proxy_pass http://127.0.0.1:${admin_panel_port}/; # admin_panel|" \
    -e "s|fastcgi_pass 127.0.0.1:[0-9]\+;[[:space:]]*# validate_module|fastcgi_pass 127.0.0.1:${validate_module_port}; # validate_module|" \
    -e "s|proxy_pass http://127.0.0.1:[0-9]\+;[[:space:]]*# buffer|proxy_pass http://127.0.0.1:${buffer_listener_nginx_port}; # buffer|" \
    -e "s|alias .*# absolute assets path html|alias ${assets_dir}html/; # absolute assets path html|" \
    -e "s|alias .*# absolute assets path js|alias ${assets_dir}js/; # absolute assets path js|" \
    "$config_file" || handle_error "Port replacement failed"
rm -f "${config_file}.bak"  # Удаляем резервную копию

# Запуск
if ! openresty -c "$config_file"; then
    handle_error "Failed to start nginx\n\n"
fi

echo -e "Proxy server started.\nProxy waiting request to ${current_ip}:${nginx_port} from client.\n\n"