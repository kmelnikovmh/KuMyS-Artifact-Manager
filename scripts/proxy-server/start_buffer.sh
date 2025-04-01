#!/bin/bash

# Не изменять, ибо надо будет поменять и в nginx
buffer_listener_nginx_port=63360
buffer_listener_main_port=63370

handle_error() {
    echo -e "\nError: $1" >&2
    exit 1
}

# Переходим в папку проекта
cd proxy-server/build || handle_error "Failed to cd to proxy-server"

# Проверяем корректность аргументов
main_server_ip=$1
main_server_port=$2
if [ $# -ne 2 ]; then
    handle_error "Usage: $0 <IPv4> <Port>"
fi
ip_regex='^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$'
# Поправить то, что могут определенные названия приложений поступать, а не только ip
#if [[ ! $1 =~ $ip_regex ]]; then
#    handle_error "Invalid IPv4 format: $1"
#fi
if ! [[ $2 =~ ^[0-9]+$ ]] || [ $2 -lt 1 ] || [ $2 -gt 65535 ]; then
    handle_error "Invalid port number: $2 (must be 1-65535)"
fi

# Проверяем занятность портов
if sudo lsof -i :$buffer_listener_nginx_port > /dev/null; then
    handle_error "Port ${buffer_listener_nginx_port} is already in use"
fi
if sudo lsof -i :$buffer_listener_main_port > /dev/null; then
    handle_error "Port ${buffer_listener_main_port} is already in use"
fi

# Определяем ip буфера
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

# Запуск
if ! ./buffer "${buffer_listener_nginx_port}" "${main_server_ip}" "${main_server_port}" "${buffer_listener_main_port}" "${current_ip}"; then
    handle_error "Failed to start validation module"
fi
