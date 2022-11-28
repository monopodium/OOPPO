CRT_DIR=$(pwd)

GRPC_DIR=$CRT_DIR"/third_party/grpc"

GRPC_INSTALL_DIR=$CRT_DIR"/third_party_install/grpc"

cd $GRPC_DIR
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=$GRPC_INSTALL_DIR \
    ../..
make -j6
make install
