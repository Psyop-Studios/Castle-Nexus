del *.o
del *.a
if "%1" == "all" (
    cd ..\..\raylib\src
    del *.o
    del *.a
    make clean
    make PLATFORM=PLATFORM_DESKTOP BUILD_MODE=RELEASE RAYLIB_LIBTYPE=SHARED
    cd ..\..\raylib-NEXT-gamejam\src
)

make RAYLIB_LIBTYPE=SHARED BUILD_MODE=DEBUG

raylib_game.exe 