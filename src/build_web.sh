rm -f *.o
rm -f *.a

if [ "$1" == "all" ]; then
    cd ../../raylib/src
    rm -f *.o
    make clean
    make PLATFORM=PLATFORM_WEB BUILD_MODE=RELEASE
    cd -
fi

make PLATFORM=PLATFORM_WEB BUILD_MODE=RELEASE RAYLIB_SRC_PATH=../../raylib/src

emrun --no_browser --port 8080 .