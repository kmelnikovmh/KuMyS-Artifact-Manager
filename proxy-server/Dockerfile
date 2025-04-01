FROM ubuntu:24.04

# Обновляем пакеты
RUN apt-get update && \
    # Основная часть
    apt-get install -y --no-install-recommends \
    build-essential \
    sudo \
    nano \
    curl \
    telnet \
    g++-12 \
    cmake \
    git \
    # Скрипт с ip
    net-tools \
    lsof \
    # cpprest
    libboost-all-dev \
    libssl-dev \
    libgoogle-glog-dev \
    xz-utils \
    libcpprest-dev

RUN apt-get install -y --no-install-recommends \
    wget \
    gnupg \
    ca-certificates \
    lsb-release
RUN wget -O - https://openresty.org/package/pubkey.gpg | gpg --dearmor -o /usr/share/keyrings/openresty.gpg && \
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/openresty.gpg] http://openresty.org/package/arm64/ubuntu $(lsb_release -sc) main" | tee /etc/apt/sources.list.d/openresty.list > /dev/null
RUN apt-get update && \
    apt-get -y install openresty

WORKDIR lib
RUN git clone https://github.com/eddic/fastcgipp.git fastcgi++ && \
    mkdir fastcgi++.build && \
    cd fastcgi++.build && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=RELEASE ../fastcgi++ && \
    make && \
    sudo make install

WORKDIR /
RUN git clone proxy-server https://github.com/kmelnikovmh/KuMyS-Artifact-Manager.git kymus
WORKDIR kymus

CMD ["bash"]