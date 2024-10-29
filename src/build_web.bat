del *.o
del *.a
if "%1" == "all" (
    cd ..\..\raylib\src
    del *.o
    make clean
    make PLATFORM=PLATFORM_WEB BUILD_MODE=RELEASE
    cd ..\..\rayjam24\src
)
make PLATFORM=PLATFORM_WEB BUILD_MODE=RELEASE RAYLIB_SRC_PATH=..\..\raylib\src
emrun --no_browser --port 8080 .