#ifndef PLATFORM_WEB
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
#endif