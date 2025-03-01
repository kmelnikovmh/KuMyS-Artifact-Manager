# KuMyS-Artifact-Manager

#### Многофункциональный менеджер артефактов

## ЗДЕСЬ МОГЛА БЫТЬ ВАША РЕКЛАМА!

## Претензия на гайдик
### Необходимые библиотеки: 
* main-server: 
* proxy-server: nginx/1.24.0, FastCGI++ by Eddie Carle

Протестировано на Ubuntu 24.04

### main-server
1) 

### proxy-server
1) Изменить список репозиториев apt get, запустив скрипт `apt_config.sh`
    ```
    ./scripts/apt_config.sh
    ```
2) Изменить настройку сервера nginx, запустив скрипт `nginx_config.sh` Предполагается, что `nginx.conf` расположен в каталоге `/etc/nginx/`
    ```
    ./scripts/nginx_config.sh
    ```
3) CMake ...

##
**Авторы**: `Дунаев С.` `Мельников К.` `Михаловский М.`
