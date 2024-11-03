#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <stdio.h>


static FPSCameraZ _camera;
static int _allowCameraMovement = 1;
#define TRIGGER_BOXTARGET_LEVEL_2 "BoxTarget"
#define TRIGGER_MEMORY_2 "Memory2"

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    BeginMode3D(_camera.camera);
    _currentCamera = _camera.camera;
    
    Level *level = Game_getLevel();
    
    Level_draw(level);
    
    EndMode3D();

    // if (IsMouseButtonDown(0) && _allowCameraMovement)
    // {
        // UpdateCamera(&_camera, CAMERA_FIRST_PERSON);
    // }
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{
    dt = fminf(dt, 0.1f);
    Level *level = Game_getLevel();
    level->isEditor = 0;
    
    Level_update(level, dt);
    FPSCamera_update(&_camera, level, _allowCameraMovement, dt);
}
static void ScriptAction_setCameraMovementEnabled(Script *script, ScriptAction *action)
{
    _allowCameraMovement = action->actionInt;
}

typedef struct BoxInPlaceData
{
    float timeInPlace;
} BoxInPlaceData;

void ScriptAction_onBoxInPlaceLevel2(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    BoxInPlaceData *data = (BoxInPlaceData*)action->actionData;
    if (Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_2))
    {
        if (data->timeInPlace <= 0.0f)
        {
            data->timeInPlace = level->gameTime;
        }
    }
    if (data->timeInPlace > 0.0f && level->gameTime - data->timeInPlace < 4.0f)
    {
        DrawNarrationBottomBox("You:", "The box is in place", NULL);
    }
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    DisableCursor();
    // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    _camera.camera = (Camera){0};
    _camera.camera.position = (Vector3){ 1.5f, 1.70f, 0.0f };
    _camera.camera.target = (Vector3){ 90.0f, 100.0f, 180.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 50.0f;

    Level_load(Game_getLevel(), "resources/levels/Level2.lvl");
    int step = 0;
    


    
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 0});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "AHHHHHH!!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "Sorry, you scared me! I'm not used to having company.\n"
        "You must have fallen for the trap door. Poor thing.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
          "You should get washed up once you get out of here.\n"
        "Oh, speaking of which, no one has ever made it out of here..", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
                      "So unless you figure something out, you'll be stuck in here.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
        "I'd open the door myself from the outside, but I am a ghost..\n"
        "I can't really touch anything.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 1});

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_onBoxInPlaceLevel2, .actionData = Scene_alloc(sizeof(BoxInPlaceData), &(BoxInPlaceData){.timeInPlace = 0.0f}) });

    
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_progressNextOnTriggeredOn, .actionData = (char*)TRIGGER_MEMORY_2 });
    step += 1;

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("Spooky voice:",
            "You lot may have won last time, but not again!", 1)});
    step += 1;



}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    EnableCursor();
}


SceneConfig _scene_puzzle_2 = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_PUZZLE_2,
};