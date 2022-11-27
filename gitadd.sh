CRT_DIR=$(pwd)

ASIO_DIR=$CRT_DIR"/third_party/asio-1.24.0"
LIBMEMCACHED_DIR=$CRT_DIR"/third_party/libmemcached-1.0.18"
GRPC_DIR=$CRT_DIR"/third_party/grpc"

ASIO_INSTALL_DIR=$CRT_DIR"/third_party_install/asio"
LIBMEMCACHED_INSTALL_DIR=$CRT_DIR"/third_party_install/libmemcached"
GRPC_INSTALL_DIR=$CRT_DIR"/third_party_install/grpc"

cd oppo_project
rm build -rf
git add .
cd $LIBMEMCACHED_DIR
git add .
