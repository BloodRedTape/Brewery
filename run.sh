#!/bin/sh

OUT_NAME="Brewery"

if ! [ -f build ]; then
    mkdir build
fi

cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

if ! make -j$(nproc); then
    echo "Build failed"
    exit -1
fi

cd ../

if [ -f build/$OUT_NAME ]; then
    ./build/$OUT_NAME
else
    echo "Can't find executable"
fi