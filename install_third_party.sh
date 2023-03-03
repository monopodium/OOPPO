#如果报错：clients/memflush.cc:51:21: error: ISO C++ forbids comparison between pointer and integer [-fpermissive]
#   51 |     if (opt_servers == false)
#则手动将代码改为 if (opt_servers == NULL)即可
#
#
CRT_DIR=$(pwd)
set -e

ASIO_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/asio"
GRPC_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/grpc"
GF_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/gf-complete"
JERASURE_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/jerasure"
LIBMEMCACHED_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/libmemcached"
MEMCACHED_INSTALL_DIR=$CRT_DIR"/memcached" 

ASIO_DIR=$CRT_DIR"/third_party/asio-1.24.0"
GRPC_DIR=$CRT_DIR"/third_party/grpc"
GF_DIR=$CRT_DIR"/third_party/gf-complete"
JERASURE_DIR=$CRT_DIR"/third_party/jerasure"
LIBMEMCACHED_DIR=$CRT_DIR"/third_party/libmemcached-1.0.18"
MEMCACHED_DIR=$CRT_DIR"/third_party/memcached-1.6.17"

mkdir -p $ASIO_INSTALL_DIR
mkdir -p $GRPC_INSTALL_DIR
mkdir -p $GF_INSTALL_DIR
mkdir -p $JERASURE_INSTALL_DIR
mkdir -p $LIBMEMCACHED_INSTALL_DIR
mkdir -p $MEMCACHED_INSTALL_DIR

cd $ASIO_INSTALL_DIR
rm * -rf
cd $GRPC_INSTALL_DIR
rm * -rf
cd $GF_INSTALL_DIR
rm * -rf
cd $JERASURE_INSTALL_DIR
rm * -rf
cd $LIBMEMCACHED_INSTALL_DIR
rm * -rf
cd $MEMCACHED_INSTALL_DIR
rm * -rf

cd $CRT_DIR"/third_party"
rm -rf asio-1.24.0
rm -rf grpc
rm -rf gf-complete
rm -rf jerasure
rm -rf libmemcached-1.0.18
rm -rf memcached-1.6.17
tar -xvzf asio.tar.gz
tar -xvzf grpc.tar.gz
tar -xvzf gf-complete.tar.gz
tar -xvzf jerasure.tar.gz
if [ ! -f "libmemcached-1.0.18.tar.gz" ]; then
  wget https://launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz
fi
tar -xvzf libmemcached-1.0.18.tar.gz
if [ ! -f "memcached-1.6.17.tar.gz" ]; then
  wget http://www.memcached.org/files/memcached-1.6.17.tar.gz
fi
tar -xvzf memcached-1.6.17.tar.gz

#libmemcached
cd $LIBMEMCACHED_DIR
autoreconf -i
./configure --prefix=$LIBMEMCACHED_INSTALL_DIR CFLAGS="-O0 -g"
sed -i 's/opt_servers == false/opt_servers == NULL/g' ./clients/memflush.cc
make -j6
make install

#asio
cd $ASIO_DIR
./configure --prefix=$ASIO_INSTALL_DIR
make -j6
make install

#grpc
cd $GRPC_DIR
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=$GRPC_INSTALL_DIR \
    ../..
make -j6
make install

#gf-complete
cd $GF_DIR
autoreconf -if
./configure --prefix=$GF_INSTALL_DIR
make -j6
make install

#jerasure
cd $JERASURE_DIR
autoreconf -if
./configure --prefix=$JERASURE_INSTALL_DIR LDFLAGS=-L$GF_INSTALL_DIR/lib CPPFLAGS=-I$GF_INSTALL_DIR/include
make -j6
make install

#memcached
cd $MEMCACHED_DIR
autoreconf -i
./configure --prefix=$MEMCACHED_INSTALL_DIR CFLAGS="-O0 -g"
make -j6
make install