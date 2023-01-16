#这是如何多运行几个memcached的实例
kill -9 $(pidof memcached)
kill -9 $(pidof run_datanode)


./memcached/bin/memcached -m 128 -p 8100 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8101 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8102 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8103 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8104 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8105 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8106 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8107 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8108 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8109 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8110 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8111 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8112 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8113 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8114 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8115 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8116 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8117 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8118 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8119 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8120 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8121 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8122 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8123 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8124 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8125 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8126 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8127 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8128 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8129 --max-item-size=5242880 -vv -d

./memcached/bin/memcached -m 128 -p 8200 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8201 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8202 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8203 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8204 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8205 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8206 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8207 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8208 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8209 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8210 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8211 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8212 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8213 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8214 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8215 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8216 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8217 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8218 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8219 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8220 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8221 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8222 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8223 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8224 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8225 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8226 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8227 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8228 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8229 --max-item-size=5242880 -vv -d

./memcached/bin/memcached -m 128 -p 8300 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8301 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8302 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8303 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8304 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8305 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8306 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8307 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8308 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8309 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8310 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8311 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8312 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8313 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8314 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8315 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8316 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8317 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8318 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8319 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8320 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8321 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8322 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8323 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8324 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8325 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8326 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8327 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8328 --max-item-size=5242880 -vv -d
./memcached/bin/memcached -m 128 -p 8329 --max-item-size=5242880 -vv -d









./oppo_project/cmake/build/run_datanode 0.0.0.0:9100
./oppo_project/cmake/build/run_datanode 0.0.0.0:9101
./oppo_project/cmake/build/run_datanode 0.0.0.0:9102
./oppo_project/cmake/build/run_datanode 0.0.0.0:9103
./oppo_project/cmake/build/run_datanode 0.0.0.0:9104
./oppo_project/cmake/build/run_datanode 0.0.0.0:9105
./oppo_project/cmake/build/run_datanode 0.0.0.0:9106
./oppo_project/cmake/build/run_datanode 0.0.0.0:9107
./oppo_project/cmake/build/run_datanode 0.0.0.0:9108
./oppo_project/cmake/build/run_datanode 0.0.0.0:9109
./oppo_project/cmake/build/run_datanode 0.0.0.0:9110
./oppo_project/cmake/build/run_datanode 0.0.0.0:9111
./oppo_project/cmake/build/run_datanode 0.0.0.0:9112
./oppo_project/cmake/build/run_datanode 0.0.0.0:9113
./oppo_project/cmake/build/run_datanode 0.0.0.0:9114
./oppo_project/cmake/build/run_datanode 0.0.0.0:9115
./oppo_project/cmake/build/run_datanode 0.0.0.0:9116
./oppo_project/cmake/build/run_datanode 0.0.0.0:9117
./oppo_project/cmake/build/run_datanode 0.0.0.0:9118
./oppo_project/cmake/build/run_datanode 0.0.0.0:9119
./oppo_project/cmake/build/run_datanode 0.0.0.0:9120
./oppo_project/cmake/build/run_datanode 0.0.0.0:9121
./oppo_project/cmake/build/run_datanode 0.0.0.0:9122
./oppo_project/cmake/build/run_datanode 0.0.0.0:9123
./oppo_project/cmake/build/run_datanode 0.0.0.0:9124
./oppo_project/cmake/build/run_datanode 0.0.0.0:9125
./oppo_project/cmake/build/run_datanode 0.0.0.0:9126
./oppo_project/cmake/build/run_datanode 0.0.0.0:9127
./oppo_project/cmake/build/run_datanode 0.0.0.0:9128
./oppo_project/cmake/build/run_datanode 0.0.0.0:9129
./oppo_project/cmake/build/run_datanode 0.0.0.0:9200
./oppo_project/cmake/build/run_datanode 0.0.0.0:9201
./oppo_project/cmake/build/run_datanode 0.0.0.0:9202
./oppo_project/cmake/build/run_datanode 0.0.0.0:9203
./oppo_project/cmake/build/run_datanode 0.0.0.0:9204
./oppo_project/cmake/build/run_datanode 0.0.0.0:9205
./oppo_project/cmake/build/run_datanode 0.0.0.0:9206
./oppo_project/cmake/build/run_datanode 0.0.0.0:9207
./oppo_project/cmake/build/run_datanode 0.0.0.0:9208
./oppo_project/cmake/build/run_datanode 0.0.0.0:9209
./oppo_project/cmake/build/run_datanode 0.0.0.0:9210
./oppo_project/cmake/build/run_datanode 0.0.0.0:9211
./oppo_project/cmake/build/run_datanode 0.0.0.0:9212
./oppo_project/cmake/build/run_datanode 0.0.0.0:9213
./oppo_project/cmake/build/run_datanode 0.0.0.0:9214
./oppo_project/cmake/build/run_datanode 0.0.0.0:9215
./oppo_project/cmake/build/run_datanode 0.0.0.0:9216
./oppo_project/cmake/build/run_datanode 0.0.0.0:9217
./oppo_project/cmake/build/run_datanode 0.0.0.0:9218
./oppo_project/cmake/build/run_datanode 0.0.0.0:9219
./oppo_project/cmake/build/run_datanode 0.0.0.0:9220
./oppo_project/cmake/build/run_datanode 0.0.0.0:9221
./oppo_project/cmake/build/run_datanode 0.0.0.0:9222
./oppo_project/cmake/build/run_datanode 0.0.0.0:9223
./oppo_project/cmake/build/run_datanode 0.0.0.0:9224
./oppo_project/cmake/build/run_datanode 0.0.0.0:9225
./oppo_project/cmake/build/run_datanode 0.0.0.0:9226
./oppo_project/cmake/build/run_datanode 0.0.0.0:9227
./oppo_project/cmake/build/run_datanode 0.0.0.0:9228
./oppo_project/cmake/build/run_datanode 0.0.0.0:9229
./oppo_project/cmake/build/run_datanode 0.0.0.0:9300
./oppo_project/cmake/build/run_datanode 0.0.0.0:9301
./oppo_project/cmake/build/run_datanode 0.0.0.0:9302
./oppo_project/cmake/build/run_datanode 0.0.0.0:9303
./oppo_project/cmake/build/run_datanode 0.0.0.0:9304
./oppo_project/cmake/build/run_datanode 0.0.0.0:9305
./oppo_project/cmake/build/run_datanode 0.0.0.0:9306
./oppo_project/cmake/build/run_datanode 0.0.0.0:9307
./oppo_project/cmake/build/run_datanode 0.0.0.0:9308
./oppo_project/cmake/build/run_datanode 0.0.0.0:9309
./oppo_project/cmake/build/run_datanode 0.0.0.0:9310
./oppo_project/cmake/build/run_datanode 0.0.0.0:9311
./oppo_project/cmake/build/run_datanode 0.0.0.0:9312
./oppo_project/cmake/build/run_datanode 0.0.0.0:9313
./oppo_project/cmake/build/run_datanode 0.0.0.0:9314
./oppo_project/cmake/build/run_datanode 0.0.0.0:9315
./oppo_project/cmake/build/run_datanode 0.0.0.0:9316
./oppo_project/cmake/build/run_datanode 0.0.0.0:9317
./oppo_project/cmake/build/run_datanode 0.0.0.0:9318
./oppo_project/cmake/build/run_datanode 0.0.0.0:9319
./oppo_project/cmake/build/run_datanode 0.0.0.0:9320
./oppo_project/cmake/build/run_datanode 0.0.0.0:9321
./oppo_project/cmake/build/run_datanode 0.0.0.0:9322
./oppo_project/cmake/build/run_datanode 0.0.0.0:9323
./oppo_project/cmake/build/run_datanode 0.0.0.0:9324
./oppo_project/cmake/build/run_datanode 0.0.0.0:9325
./oppo_project/cmake/build/run_datanode 0.0.0.0:9326
./oppo_project/cmake/build/run_datanode 0.0.0.0:9327
./oppo_project/cmake/build/run_datanode 0.0.0.0:9328
./oppo_project/cmake/build/run_datanode 0.0.0.0:9329