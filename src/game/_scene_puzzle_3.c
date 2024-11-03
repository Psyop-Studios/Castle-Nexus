#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <stdio.h>


static FPSCameraZ _camera;
static int _allowCameraMovement = 1;
#define TRIGGER_BOXTARGET_LEVEL_3_1 "BoxTarget1"
#define TRIGGER_BOXTARGET_LEVEL_3_2 "BoxTarget2"
#define TRIGGER_BOXTARGET_LEVEL_3_3 "BoxTargetPt1"

#define TRIGGER_MEMORY_3 "Memory3"

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

typedef struct {
    float timeInPlace1;
    float timeInPlace2;

} BoxInPlaceData;

// void ScriptAction_onBoxInPlaceLevel3(Script *script, ScriptAction *action)
// {
//     Level *level = Game_getLevel();
//     BoxInPlaceData *data = (BoxInPlaceData*)action->actionData;

//     if ((Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_3_1)) && (Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_3_2)))
//     {
//         if (data->timeInPlace <= 0.0f)
//         {
//             data->timeInPlace = level->gameTime;
//         }
//     }
//     if (data->timeInPlace > 0.0f && level->gameTime - data->timeInPlace < 4.0f)
//     {
//         DrawNarrationBottomBox("You:", "The box is in place", NULL);
//     }
// }

void ScriptAction_onBoxInPlaceLevel3(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    BoxInPlaceData *data = (BoxInPlaceData*)action->actionData;

    // Check first box

    extern Sound triggerSfx;

    if (Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_3_1))
    {
        PlaySound(triggerSfx);
        if (data->timeInPlace1 <= 0.0f)
        {
            data->timeInPlace1 = level->gameTime;
        }
    }

    // Check second box
    if (Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_3_2))
    {
        PlaySound(triggerSfx);
        if (data->timeInPlace2 <= 0.0f)
        {
            data->timeInPlace2 = level->gameTime;
        }
    }

    // Check third box

    if (Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_3_3))
    {
        PlaySound(triggerSfx);
    }

    // Only show message if both boxes have been in place for less than 4 seconds
    if (data->timeInPlace1 > 0.0f && data->timeInPlace2 > 0.0f &&
        level->gameTime - data->timeInPlace1 < 4.0f &&
        level->gameTime - data->timeInPlace2 < 4.0f)
    {
        DrawNarrationBottomBox("You:", "The boxes are in place", NULL);
    }
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    Game_setFogGradient(174.0/512.0, 104.0/512.0, 0.55f, 0.26f);
    DisableCursor();
    // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    _camera.camera = (Camera){0};
    _camera.camera.position = (Vector3){ -2.0f, 1.70f, -2.0f };
    _camera.camera.target = (Vector3){ 90.0f, 100.0f, 180.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 100.0f;

    Level_load(Game_getLevel(), "resources/levels/Level3.lvl");
     int step = 0;


    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, -1.0f)});
    step += 1;


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

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_onBoxInPlaceLevel3, .actionData = Scene_alloc(sizeof(BoxInPlaceData), &(BoxInPlaceData){.timeInPlace1 = 0.0f, .timeInPlace2 = 0.0f}) });


    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_progressNextOnTriggeredOn, .actionData = (char*)TRIGGER_MEMORY_3 });
    step += 1;

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("Spooky voice:",
            "You lot may have won last time, but not again!", 1)});
    step += 1;



    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, 1.0f)});

    step += 1;

}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    EnableCursor();
}



SceneConfig _scene_puzzle_3 = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_PUZZLE_3,
};