# KuMyS-Artifact-Manager

#### Многофункциональный менеджер артефактов
Описание ...


## Руководство по эксплуатации
### Необходимые библиотеки: 
* common: CppRestSDK, Folly
* main-server: ...
* proxy-server: Openresty (nginx, lua), FastCGI++ by Eddie Carle
<!-- Надо будет закрепить версии -->

Протестировано на Docker Ubuntu 24.04

### Docker
...

1) Собрать все контейнеры:
    ```bash
    docker-compose up -d --build
    ```
2) Зайти в соответствующий контейнер: app, proxy
    ```bash
    docker exec -it <name> bash
    ```

### Main-server
1) Собрать проект:
    ```bash
    cmake .. && make
    ```
2) Запустить сервер:
    ```bash
    ./kumys_server
    ```

### Proxy-server

#### Инструкция по запуску
**Важно**: на данном этапе разработки скрипты зависимы от папки, в которой они лежат
1) Собрать проект:
    ```
    ./scripts/proxy-server/build_modules.sh
    ```
2) Запустить nginx. Остановить его можно дополнительным аргументом stop\
    **Важно**: перед повторным запуском необходимо остановить уже запущенный
    ```bash
    ./scripts/proxy-server/start_nginx.sh
    ```
    ```bash
    ./scripts/proxy-server/start_nginx.sh stop
    ```
3) В отдельном терминале запустить модуль валидации запросов:
    ```bash
    ./scripts/proxy-server/start_validate_module.sh
    ```
4) В отдельном терминале запустить буффер между nginx и main-server'ом, указав в аргументах ip и port main-server'а:
    ```bash
    ./scripts/proxy-server/start_buffer.sh <ip> <port>
    ```

В данной инструкции не отражена возможность изменить порты модулей, как и возможность запустить модули на разных машинах. Todo

Краткая схема того, как связаны модули на дефолтных портах:
```c++
// Ports:
// client <-> (63380) nginx <-> (63350) validate_module
//                          <-> (63360) buffer (63370) <-> (ip, port) main-server 
```

### Client
Изменить список репозиториев apt get, запустив скрипт `apt_config.sh`, указав ip адрес и порт, на котором развернут nginx сервер
```bash
./scripts/apt_config.sh <ip> <port>
```

##
**Авторы**: `Дунаев С.` `Мельников К.` `Михаловский М.`

#### ЗДЕСЬ ВСЕ ЕЩЕ МОГЛА БЫ БЫТЬ ВАША РЕКЛАМА!