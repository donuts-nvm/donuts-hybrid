FROM ubuntu:20.04

LABEL author="kleber.kruger"
LABEL maintainer="kleberkruger@gmail.com"

ARG UID
ARG USER
ARG TZ_ARG=UTC

# Necessary for tzdata
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=${TZ_ARG}

ENV GRAPHITE_ROOT=${GRAPHITE_ROOT}
ENV BENCHMARKS_ROOT=${BENCHMARKS_ROOT}

# Install locales and configure
RUN apt-get update && apt-get install -y locales && \
    locale-gen en_US en_US.UTF-8 es_ES es_ES.UTF-8 pt_BR pt_BR.UTF-8 && \
    update-locale LANG=en_US.UTF-8

# Add i386 support for Pin
RUN dpkg --add-architecture i386
RUN apt-get update && apt-get install -y \
    python \
    screen \
    tmux \
    binutils \
    libc6:i386 \
    libncurses5:i386 \
    libstdc++6:i386 \
    && rm -rf /var/lib/apt/lists/*

# For building Sniper
RUN apt-get update && apt-get install -y \
    automake \
    build-essential \
    cmake \
    curl \
    wget \
    libboost-dev \
    libsqlite3-dev \
    zlib1g-dev \
    libbz2-dev \
    && rm -rf /var/lib/apt/lists/*

# For building RISC-V Tools
RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    autotools-dev \
    bc \
    bison \
    curl \
    device-tree-compiler \
    flex \
    gawk \
    gperf \
    libexpat-dev \
    libgmp-dev \
    libmpc-dev \
    libmpfr-dev \
    libtool \
    libusb-1.0-0-dev \
    patchutils \
    pkg-config \
    texinfo \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Install gcc-13 and g++-13
RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    lsb-release \
    gnupg \
    ca-certificates && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update && \
    apt-get install -y \
    gcc-13 g++-13 && \
    rm -rf /var/lib/apt/lists/* && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 60 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 60

# Helper utilities
RUN apt-get update && apt-get install -y \
    gdb \
    gfortran \
    git \
    vim \
    nano \
    lsof \
    && rm -rf /var/lib/apt/lists/*

# For compile the benchmarks
RUN apt-get update && apt-get install -y \
    xsltproc \
    gettext \
    libx11-dev \
    libxext-dev \
    libxt-dev \
    libxmu-dev \
    libxi-dev \
    task-spooler \
    && rm -rf /var/lib/apt/lists/*

# Upgrade OS
RUN apt-get upgrade -y \
    && rm -rf /var/lib/apt/lists/*

# Install sudo and SSH server
RUN apt-get update && apt-get install -y \
    sudo \
    openssh-server \
    && rm -rf /var/lib/apt/lists/*

# Create user and setup permissions on /etc/sudoers
RUN useradd -m -s /bin/bash -N -u $UID $USER && \
    # usermod -aG sudo $USER && \
    echo "${USER} ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers && \
    chmod 0440 /etc/sudoers && \
    chmod g+w /etc/passwd

# Copy the id_ed25519 key to container using Docker secrets
RUN mkdir -p /home/$USER/.ssh && ln -s /run/secrets/user_ssh_key /home/$USER/.ssh/id_ed25519
RUN chown -R $USER:sudo /home/$USER/.ssh
RUN echo "Host *.trabe.io\n\tStrictHostKeyChecking no\n" >> /home/$USER/.ssh/config

# Allow SSH access
RUN mkdir /var/run/sshd
# Expose the SSH port
EXPOSE 22
