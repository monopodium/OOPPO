CRT_DIR=$(pwd)
set -e

ASIO_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/asio"
GRPC_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/grpc"
ASIO_DIR=$CRT_DIR"/third_party/asio-1.24.0"
GRPC_DIR=$CRT_DIR"/third_party/grpc"

mkdir -p $ASIO_INSTALL_DIR
mkdir -p $GRPC_INSTALL_DIR

cd $ASIO_INSTALL_DIR
rm * -rf
cd $GRPC_INSTALL_DIR
rm * -rf

cd $CRT_DIR"/third_party"
rm -rf asio-1.24.0
rm -rf grpc
tar -xvzf asio.tar.gz
tar -xvzf grpc.tar.gz



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