#这是如何多运行几个memcached的实例

./memcached/bin/memcached -m 128 -p 8100 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8101 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8102 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8103 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8104 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8105 --max-item-size=5242880 -vv -d