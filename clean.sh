CRT_DIR=$(pwd)

ASIO_INSTALL_DIR=$CRT_DIR"/third_party_install/asio"
GRPC_INSTALL_DIR=$CRT_DIR"/third_party_install/grpc"

cd $ASIO_INSTALL_DIR
rm * -rf
cd $GRPC_INSTALL_DIR
rm * -rf

cd $CRT_DIR"/third_party"
rm -rf asio-1.24.0
rm -rf grpc
tar -xvzf asio.tar.gz
tar -xvzf grpc.tar.gz
