FROM debian:trixie-slim
LABEL maintainer="Masaki Waga <masakiwaga@gmail.com>"

# The branch of SyMon to build. This can be overridden at build time
# using `--build-arg SYMON_BRANCH=<branch>`.
ARG SYMON_BRANCH=master

# Install required build dependencies including tree-sitter
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        build-essential \
        libppl-dev \
        libboost-all-dev \
        libgmp-dev \
        cmake \
        git \
        pkg-config \
        libtree-sitter-dev && \
    rm -rf /var/lib/apt/lists/*

# Install tree-sitter-symon
RUN git clone --depth 1 https://github.com/maswag/tree-sitter-symon.git /tmp/tree-sitter-symon && \
    cmake -S /tmp/tree-sitter-symon -B /tmp/tree-sitter-symon/build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /tmp/tree-sitter-symon/build && \
    cmake --install /tmp/tree-sitter-symon/build && \
    rm -rf /tmp/tree-sitter-symon

# Clone and build SyMon
RUN git clone --depth 1 --branch ${SYMON_BRANCH} https://github.com/maswag/symon.git /tmp/symon && \
    cmake -S /tmp/symon -B /tmp/symon/build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /tmp/symon/build && \
    cmake --install /tmp/symon/build && \
    rm -rf /tmp/symon

# Remove build tools to reduce image size
RUN apt-get purge -y \
        build-essential \
        cmake \
        git && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/usr/local/bin/symon"]
