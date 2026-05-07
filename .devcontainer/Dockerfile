FROM debian:trixie

ARG USER=user
ENV DEBIAN_FRONTEND=noninteractive

# general stuff plus mingw cross compiler
RUN apt update -y && apt install -y \
    git \
    build-essential \
    make \
    pkg-config \
    cmake \
    ninja-build \
    clangd \
    clang-format \
    sudo \
    locales \
    gcc-mingw-w64-x86-64 \
    g++-mingw-w64-x86-64 \
    doxygen \
    ccache \
    jq \
    yq \
    python3-pip \
    curl
    
# SDL3 reqs
RUN apt update -y && apt install -y \
    gnome-desktop-testing \
    libasound2-dev \
    libpulse-dev \
    libaudio-dev \
    libjack-dev \
    libsndio-dev \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxfixes-dev \
    libxi-dev \
    libxss-dev \
    libxtst-dev \
    libxkbcommon-dev \
    libdrm-dev \
    libgbm-dev \
    libgl1-mesa-dev \
    libgles2-mesa-dev \
    libegl1-mesa-dev \
    libdbus-1-dev \
    libibus-1.0-dev \
    libudev-dev

# SFML3 reqs
RUN apt update -y && apt install -y \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev \
    libfreetype-dev

RUN curl -L --output /tmp/glslang.tar.gz https://github.com/KhronosGroup/glslang/archive/refs/tags/16.3.0.tar.gz && \
    tar -xzf /tmp/glslang.tar.gz -C /tmp && \
    cd /tmp/glslang-16.3.0 && python3 update_glslang_sources.py && \
    cmake -S /tmp/glslang-16.3.0 -B /tmp/glslang-build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DENABLE_GLSLANG_BINARIES=ON \
        -DENABLE_HLSL=ON \
        -GNinja && \
    ninja -C /tmp/glslang-build install && \
    rm -rf /tmp/glslang*

RUN locale-gen en_GB.UTF-8 && update-locale

RUN useradd -m $USER && echo "$USER:$USER" | chpasswd && adduser $USER sudo
RUN echo "user            ALL = (ALL) NOPASSWD: ALL" >> /etc/sudoers

RUN printf "alias ll='ls -la'" >> /home/$USER/.bashrc

CMD /bin/bash