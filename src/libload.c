#ifndef PLATFORM_WEB

#if defined(_WIN32) || defined(_WIN64)

#include "windows.h"

extern void (*Game_init)();
extern void (*Game_deinit)();
extern void (*Game_update)();

static HMODULE game;

void unload_game()
{
    if (game)
    {
        FreeLibrary(game);
        game = NULL;
    }
}

void load_game()
{
    unload_game();

    game = LoadLibraryA("game.dll");
    if (game)
    {
        Game_init = (void (*)())GetProcAddress(game, "Game_init");
        Game_deinit = (void (*)())GetProcAddress(game, "Game_deinit");
        Game_update = (void (*)())GetProcAddress(game, "Game_update");
    }
}

#else

#include <dlfcn.h>
#include <stddef.h>

extern void (*Game_init)();
extern void (*Game_deinit)();
extern void (*Game_update)();

static void *game;

void unload_game()
{
    if (game)
    {
        dlclose(game);
        game = NULL;
    }
}

void load_game()
{
    unload_game();

    game = dlopen("./game.so", RTLD_LAZY);
    if (game)
    {
        Game_init = (void (*)())dlsym(game, "Game_init");
        Game_deinit = (void (*)())dlsym(game, "Game_deinit");
        Game_update = (void (*)())dlsym(game, "Game_update");
    }
}


#endif
#endif