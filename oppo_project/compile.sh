CRT_DIR=$(pwd)
set -e

#our_project
cd $CRT_DIR
mkdir -p cmake/build
cd cmake/build
cmake ../..
make -j