CRT_DIR=$(pwd)
set -e

ASIO_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/asio"
GRPC_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/grpc"
GF_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/gf-complete"
JERASURE_INSTALL_DIR=$CRT_DIR"/oppo_project/third_party/jerasure"

ASIO_DIR=$CRT_DIR"/third_party/asio-1.24.0"
GRPC_DIR=$CRT_DIR"/third_party/grpc"
GF_DIR=$CRT_DIR"/third_party/gf-complete"
JERASURE_DIR=$CRT_DIR"/third_party/jerasure"

mkdir -p $ASIO_INSTALL_DIR
mkdir -p $GRPC_INSTALL_DIR
mkdir -p $GF_INSTALL_DIR
mkdir -p $JERASURE_INSTALL_DIR

cd $ASIO_INSTALL_DIR
rm * -rf
cd $GRPC_INSTALL_DIR
rm * -rf
cd $GF_INSTALL_DIR
rm * -rf
cd $JERASURE_INSTALL_DIR
rm * -rf

cd $CRT_DIR"/third_party"
rm -rf asio-1.24.0
rm -rf grpc
rm -rf gf-complete
rm -rf jerasure
tar -xvzf asio.tar.gz
tar -xvzf grpc.tar.gz
tar -xvzf gf-complete.tar.gz
tar -xvzf jerasure.tar.gz



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