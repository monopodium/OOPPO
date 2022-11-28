CRT_DIR=$(pwd)
set -e

#libmemcached-1.0.18
LIBMEMCACHED_INSTALL_DIR=$CRT_DIR"/third_party/libmemcached"
MEMCACHED_INSTALL_DIR=$CRT_DIR"/../memcached"

LIBMEMCACHED_DIR=$CRT_DIR"/src/libmemcached-1.0.18"
MEMCACHED_DIR=$CRT_DIR"/src/memcached-1.6.17"

mkdir -p $LIBMEMCACHED_INSTALL_DIR
cd $LIBMEMCACHED_DIR
autoreconf -i
./configure --prefix=$LIBMEMCACHED_INSTALL_DIR
make -j6
make install

#memcached
mkdir -p $MEMCACHED_INSTALL_DIR
cd $MEMCACHED_DIR
autoreconf -i
./configure --prefix=$MEMCACHED_INSTALL_DIR
make && make install

#our_project
cd DIR
mkdir -p cmake/build
cd cmake/build
cmake ../..
make -j