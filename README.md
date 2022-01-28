# SSE Schemes

This repository contains implementation of some SSE schemes such as QOS, SDD, SDA, Mitra, and Horus. The base of Mitra and Horus is derived from the code implemented by Javad Ghareh Chamani (https://github.com/jgharehchamani/SSE). 

# Pre-requisites

Schemes' implementation need a compiler supporting C++14 (although the core codebase doesn't). They have been successfully built and tested on Ubuntu 16.04 LTS.
They use Google's [gRPC](http://grpc.io) as their RPC machinery and Facebook's [RocksDB](http://rocksdb.org) as their storage engine.
The linux build commands are as follows.

## Linux

```sh
$ sudo apt-get update
$ sudo apt-get install build-essential autoconf libtool yasm openssl scons pkg-config libgflags-dev libgtest-dev clang libc++-dev git unzip libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev  libssl-dev libcrypto++-dev libssl1.0.0 libgmp-dev curl make automake
$ sudo add-apt-repository ppa:ubuntu-toolchain-r/test
$ sudo apt-get update
$ sudo apt-get install gcc-5 g++-5
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 1
$ sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 1
```

### Installing CMake
```sh
$ wget https://cmake.org/files/v3.11/cmake-3.11.3.tar.gz
$ tar -xzvf cmake-3.11.3.tar.gz
$ cd cmake-3.11.3/
$ ./configure
$ make
$ sudo make install
$ cd ..
```

### Installing GRPC
```sh
$ git clone -b v1.26.0  https://github.com/grpc/grpc
$ cd grpc/
$ git submodule update --init
add -Wno-unused-variable at the end of line 356 in Makefile
$ make
$ sudo make install
$ cd ..
```

### Installing RocksDB
```sh
$ wget https://github.com/facebook/rocksdb/archive/rocksdb-5.7.5.zip
$ unzip rocksdb-5.7.5.zip
$ cd rocksdb-rocksdb-5.7.5/
$ make all
$ sudo make install   
$ cd ..
```

### Installing Boost
```sh
$ wget https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.gz
$ tar -xzvf boost_1_64_0.tar.gz
$ cd boost_1_64_0/
$ ./bootstrap.sh --prefix=/usr/local
$ ./b2
$ sudo ./b2 install
$ cd ..
```

### Installing Relic
```sh
$ git clone https://github.com/relic-toolkit/relic.git
$ cd relic/
$ git checkout 94154a8b77e13d7fd24c9081e7fc6cd95956473d
$ mkdir build
$ cd build/
$ cmake ..
$ make
$ sudo make install
$ cd ../..
```

### Installing Protobuf
```sh
$ wget https://github.com/google/protobuf/releases/download/v3.5.1/protobuf-cpp-3.5.1.zip
$ unzip protobuf-cpp-3.5.1.zip
$ cd protobuf-3.5.1/
$ ./configure
$ make
$ sudo make install
$ cd ..
```

### Reloading Modules
```sh
$ sudo ldconfig
```

## Getting the code
The code is available *via* git:

```sh
 $ git clone https://github.com/jgharehchamani/Small-Client-SSE.git
```

# Building

Building is done through [SConstruct](http://www.scons.org). 

To build the submodules, you can either run

```sh
 $ scons deps
```
Then, to build the code itself, just enter in your terminal

```sh
 $ scons 
```

# Usage

This repository provides implementations of SSE as a proof of concept, and cannot be used for real sensitive applications. In particular, the cryptographic toolkit most probably has many implementation flaws.

After compiling the project, one binary file will be generated for each scheme X which is designed to be run on a single system. It executes client and server operation as a local command and no communication cost is being considered.

# Contributors

Except the cryptographic third party codes which has been written by [Raphael Bost](http://people.irisa.fr/Raphael.Bost/), all other parts have been implemented by [Javad Ghareh Chamani](http://home.cse.ust.hk/~jgc/)

# Licensing

SSE Schemes is licensed under the [GNU Affero General Public License v3](http://www.gnu.org/licenses/agpl.html).

![AGPL](http://www.gnu.org/graphics/agplv3-88x31.png)
