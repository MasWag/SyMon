FROM debian:bookworm-slim
LABEL maintainer="Masaki Waga <masakiwaga@gmail.com>"

# Install required build dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        build-essential \
        libppl-dev \
        libboost-all-dev \
        cmake \
        libgmp-dev \
        git && \
    rm -rf /var/lib/apt/lists/*

# Clone and build SyMon in one layer
RUN git clone --depth 1 https://github.com/MasWag/symon.git /tmp/symon && \
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
