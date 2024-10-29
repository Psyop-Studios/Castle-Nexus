#!/bin/bash

rm -f *.o
rm -f *.a
if [ "$1" == "all" ]; then
    cd ../../raylib/src
    rm -f *.o
    rm -f *.a
    make clean
    make PLATFORM=PLATFORM_DESKTOP BUILD_MODE=RELEASE RAYLIB_LIBTYPE=SHARED
    cd -
fi

make RAYLIB_LIBTYPE=SHARED BUILD_MODE=DEBUG

./raylib_game