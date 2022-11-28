CRT_DIR=$(pwd)

ASIO_DIR=$CRT_DIR"/third_party/asio-1.24.0"

ASIO_INSTALL_DIR=$CRT_DIR"/third_party_install/asio"

cd $ASIO_DIR
./configure --prefix=$ASIO_INSTALL_DIR
make -j6
make install
