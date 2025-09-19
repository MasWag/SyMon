Installation
============

SyMon is fully implemented in C++. SyMon works on many UNIX-like operating systems. We tested SyMon on macOS Sequoia 15.6, Ubuntu 24.04, and Arch Linux.

Installation with Homebrew
--------------------------

On macOS, it is the simplest to install SyMon using [Homebrew](https://brew.sh/).

```sh
brew install --HEAD maswag/scientific/symon
```

Then, you can just run symon, for example, as follows.

```sh
symon -pnf [foo.symon] < [bar.log] 
```

You can likely "run" the .symon file by appropriately specify the [shebang](https://en.wikipedia.org/wiki/Shebang_(Unix)).

Installation with Docker
------------------------

SyMon can be also used within a docker container.

```sh
docker pull maswag/symon:latest
```

Then, you can run symon. Notice that you have to specify `-v` option to bind the directory containing .symon file.

```sh
docker run -i -v $PWD:/tmp maswag/ symon:latest -pnf /tmp/[foo.symon] < [bar.log]
```

Building from scratch
---------------------

To build SyMon from scratch, you need to install the following dependencies.

- C++ compiler supporting C++17 and the corresponding libraries.
- Boost (>= 1.67.0)
- CMake (>= 3.25)
- Parma Polyhedra Library
- Tree Sitter

On Ubuntu, the above can be installed as follows.

```sh
sudo apt-get install libppl-dev \
    libboost-all-dev \
    cmake \
    libgmp-dev \
    libtree-sitter-dev
```

First, you need to build and install the grammar definition of SyMon.

```sh
git clone https://github.com/maswag/tree-sitter-symon.git /tmp/tree-sitter-symon
cmake -B /tmp/tree-sitter-symon/build -S /tmp/tree-sitter-symon
cmake --build /tmp/tree-sitter-symon/build
sudo cmake --install /tmp/tree-sitter-symon/build
```

Then, you can build and install SyMon.

```sh
git clone https://github.com/maswag/symon.git /tmp/symon
cmake -DCMAKE_BUILD_TYPE=Release -B /tmp/symon/build -S /tmp/symon
cmake --build /tmp/symon/build
sudo cmake --install /tmp/symon/build
```
