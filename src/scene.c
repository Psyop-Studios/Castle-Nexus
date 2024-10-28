#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "scene.h"

#include "scenes/mainmenu.h"
#include "scenes/raylogo.h"
#include "scenes/game.h"

scene_t currentScene = RAYLOGO;

void ChangeScene(scene_t scene)
{
    static void (*initSceneFunctions[])(void) = {[RAYLOGO] = InitRaylogoScene, [MAINMENU] = InitMainmenuScene, [GAME] = InitGameScene};

    switch (scene)
    {
    case GAME:

        break;
    case MAINMENU:

    default:
        break;
    }
    initSceneFunctions[scene]();
    currentScene = scene;
}

void UpdateCurrentScene(void)
{
    static void (*updateSceneFunctions[])(void) = {[RAYLOGO] = UpdateRaylogoScene, [MAINMENU] = UpdateMainmenuScene, [GAME] = UpdateGameScene};
    updateSceneFunctions[currentScene]();
}

void DrawCurrentScene(void)
{
    static void (*drawSceneFunctions[])(void) = {[RAYLOGO] = DrawRaylogoScene, [MAINMENU] = DrawMainmenuScene, [GAME] = DrawGameScene};
    drawSceneFunctions[currentScene]();
}