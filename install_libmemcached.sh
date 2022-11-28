CRT_DIR=$(pwd)

LIBMEMCACHED_DIR=$CRT_DIR"/oppo_project/src/libmemcached-1.0.18"

LIBMEMCACHED_INSTALL_DIR=$CRT_DIR"/third_party_install/libmemcached"

cd $LIBMEMCACHED_DIR
./configure --prefix=$LIBMEMCACHED_INSTALL_DIR
make -j6
make install
