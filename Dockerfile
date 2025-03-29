# Используем официальный образ Ubuntu LTS
FROM ubuntu:24.04

# Обновляем пакеты и устанавливаем зависимости
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    g++-12 \
    libboost-all-dev \
    libdouble-conversion-dev \
    libevent-dev \
    libgtest-dev \
    libssl-dev \
    libfmt-dev \
    zlib1g-dev \
    libcpprest-dev \
    gzip \
    gdb \
    valgrind \
    sudo

RUN  sudo apt-get install -y netcat-traditional

RUN apt-get install libgoogle-glog-dev -y

WORKDIR /tmp
RUN git config --global http.sslVerify false

RUN git clone https://github.com/fastfloat/fast_float && \
    cd fast_float && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DFASTFLOAT_INSTALL=ON .. && \
    make -j$(nproc) && \
    sudo make install

RUN git clone --depth 1 --branch v2025.03.17.00 https://github.com/facebook/folly.git && \
    cd folly && \
    ./build/fbcode_builder/getdeps.py install-system-deps --recursive && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++-12 \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && \
    make -j$(nproc) && \
    sudo make install

RUN sudo apt install -y libmongoc-1.0-0 libbson-1.0-0 libmongoc-dev libbson-dev wget gzip

RUN wget https://github.com/mongodb/mongo-c-driver/releases/download/1.30.2/mongo-c-driver-1.30.2.tar.gz && \
    tar -xzf mongo-c-driver-1.30.2.tar.gz && \
    cd mongo-c-driver-1.30.2 && \
    cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DCMAKE_BUILD_TYPE=Release \
        -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF && \
    make -j$(nproc) && \
    sudo make install

RUN wget https://github.com/mongodb/mongo-cxx-driver/archive/r4.0.0.tar.gz && \
    tar -xzf r4.0.0.tar.gz && \
    cd mongo-cxx-driver-r4.0.0 && \
    cd build && \
    cmake .. -DBUILD_VERSION=4.0.0 \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF && \
    make -j$(nproc) && \
    sudo make install

WORKDIR /app/build
CMD ["bash"]
