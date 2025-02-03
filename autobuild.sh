#!/bin/bash

set -e

rm -rf `pwd`/build `pwd`/lib `pwd`/tests
 
mkdir `pwd`/build &&
    cd `pwd`/build &&
    cmake .. &&
    make

cd ..

# copy headers to /usr/include/moony, so to /usr/lib
if [ ! -d /usr/include/moony ]; then
    mkdir /usr/include/moony
fi

cp -r `pwd`/* /usr/include/moony

cp `pwd`/lib/libleemuduo.so /usr/lib

ldconfig