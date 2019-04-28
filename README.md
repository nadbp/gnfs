# A user space file system
 
# Install dependencies
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

# Clone and build 
```bash
git clone git@github.com:ArtoriaRen/gnfs.git
cd gnfs
git checkout write_behind
cd src
make
```

# Run
In window #1, start server:
```bash
mkdir <server_side_mount_folder>
./greeter_server <server_side_mount_folder>
```
For example
```bash
mkdir ~/Download/server_mnt
./greeter_server  ~/Download/server_mnt
```

In window #2, start client:
```bash
mkdir <client_side_mount_folder>
./greeter_client <client_side_mount_folder>
```
For example
```bash
mkdir ~/Download/client_mnt
./greeter_client  ~/Download/client_mnt
```

Try creat some folder and file in the \<client_side_mount_folder\>, and you will see it appears under the \<server_side_mount_folder\>. If there are some files are folders existing in the \<server_side_mount_folder\> before starting the gnfs server and client, they will appear under the \<client_side_mount_folder\> immediately after the server and client are started.

<img src="images/creating_folder.png" width="800">
<img src="images/created.png" width="800">

You can also see some debug info printed out in window #1.

<img src="images/server_debug.png" width="800">

# Clean up
Stop the server by typing `Ctrl-C` in window #1.
Unmount the \<client_side_mount_folder\>:
```bash
fusermount -u <client_side_mount_folder>
```
