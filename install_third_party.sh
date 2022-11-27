CRT_DIR=$(pwd)

ASIO_DIR=$CRT_DIR"/third_party/asio-1.24.0"
LIBMEMCACHED_DIR=$CRT_DIR"/third_party/libmemcached-1.0.18"
GRPC_DIR=$CRT_DIR"/third_party/grpc"

ASIO_INSTALL_DIR=$CRT_DIR"/third_party_install/asio"
LIBMEMCACHED_INSTALL_DIR=$CRT_DIR"/third_party_install/libmemcached"
GRPC_INSTALL_DIR=$CRT_DIR"/third_party_install/grpc"

cd $ASIO_DIR
./configure --prefix=$ASIO_INSTALL_DIR
make -j6
make install

cd $LIBMEMCACHED_DIR
./configure --prefix=$LIBMEMCACHED_INSTALL_DIR
make -j6
make install

cd $GRPC_DIR
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=$GRPC_INSTALL_DIR \
    ../..
make -j6
make install
