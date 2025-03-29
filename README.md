# KuMyS-Artifact-Manager

#### Многофункциональный менеджер артефактов
Описание ...


## Руководство по эксплуатации
### Необходимые библиотеки: 
* main-server: x
* proxy-server: nginx/1.24.0, FastCGI++ by Eddie Carle, CppRestSDK, ...

Протестировано на Docker Ubuntu 24.04

### Docker
... Ждем кубернет

### Main-server
...

### Proxy-server

#### Инструкция по запуску
**Важно**: на данном этапе разработки скрипты зависимы от папки, в которой они лежат
1) Собрать проект:
    ```
    ./scripts/proxy-server/build_modules.sh
    ```
2) Запустить nginx. Остановить его можно дополнительным аргументом stop\
    **Важно**: перед повторным запуском необходимо остановить уже запущенный
    ```
    ./scripts/proxy-server/start_nginx.sh
    ```
    ```
    ./scripts/proxy-server/start_nginx.sh stop
    ```
3) В отдельном терминале запустить модуль валидации запросов:
    ```
    ./scripts/proxy-server/start_validate_module.sh
    ```
4) В отдельном терминале запустить буффер между nginx и main-server'ом, указав в аргументах ip и port main-server'а:
    ```
    ./scripts/proxy-server/start_buffer.sh ... ...
    ```

В данной инструкции не отражена возможность изменить порты модулей, как и возможность запустить модули на разных машинах. Todo

Краткая схема того, как связаны модули на дефолтных портах:
```c++
// Ports:
// client <-> (8000) nginx <-> (5000) validate_module
//                         <-> (6000) buffer (7000) <-> (ip, port) main-server 
```

### Client
Изменить список репозиториев apt get, запустив скрипт `apt_config.sh`, указав ip адрес и порт, на котором развернут nginx сервер
```bash
./scripts/apt_config.sh ip port
```

##
**Авторы**: `Дунаев С.` `Мельников К.` `Михаловский М.`

#### ЗДЕСЬ ВСЕ ЕЩЕ МОГЛА БЫ БЫТЬ ВАША РЕКЛАМА!