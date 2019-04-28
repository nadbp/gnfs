# gnfs

## Install fuse
Download `fuse-3.0.0.tar.gz` from [Github](https://github.com/libfuse/libfuse/releases?after=fuse-3.2.3), extract it and install
```bash
./configure
make -j8
make install
sudo ldconfig # update shared library cache.
```
Test fuse installation:
```bash
python3 -m pytest test/
```

Refer to [fuse_3_0_bugfix](https://github.com/libfuse/libfuse/tree/fuse_3_0_bugfix) for more installation info.
## Install grpc
### Install dependencies
```bash
sudo apt-get install build-essential autoconf libtool pkg-config
sudo apt-get install libgflags-dev libgtest-dev
sudo apt-get install clang libc++-dev
sudo apt-get install libc-ares-dev
```
### Clone the grpc directory
Go into `submodule/third_party/protobuf` to do some configuration. 
```bash
git clone https://github.com/grpc/grpc.git
cd grpc
git submodule update --init

cd third_party/protobuf
sudo ./autogen.sh

cd ../..
make
make install
```

Refer to [C++ Quick Start](https://grpc.io/docs/quickstart/cpp/), [gRPC C++ - Building from source](https://github.com/grpc/grpc/blob/master/BUILDING.md),  for further installation help.
