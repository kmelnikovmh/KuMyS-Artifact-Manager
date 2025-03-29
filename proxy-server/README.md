## Документация proxy-server

### Docker 
1) Официальный образ Ubuntu 24.04

    #### man-server'/
2) ...

    #### proxy-server
2) apt update, install: git, nginx+lua-module(openresty вместо nginx, по инструкции, потом локалы: apt-get install -y locales, locale-gen en_US.UTF-8), g++, cmake (clang?, nano?, curl?, netcat?, strace?, telnet?)

colima start --memory 4
(spprestsdk, folly)
apt-get install -y libboost-all-dev
apt-get install -y libssl-dev (openssl)
apt-get install sudo
apt-get install -y xz-utils
apt-get install -y libgoogle-glog-dev

#define _TURN_OFF_PLATFORM_STRING 

apt-get install -y net-tools (скрипт с ip частью)

3) Установить библиотеку FastCGI++ by Eddie Carle из исходного кода

