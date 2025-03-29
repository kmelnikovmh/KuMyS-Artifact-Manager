#!/bin/bash

handle_error() {
    echo -e "\nError: $1" >&2
    exit 1
}

# Переходим в папку проекта
cd proxy-server || handle_error "Failed to cd to proxy-server"

# Запуск
if ! cmake ./; then
    handle_error "Failed to build"
fi
if ! make; then
    handle_error "Failed to build"
fi

echo -e "\nProxy server build completed."