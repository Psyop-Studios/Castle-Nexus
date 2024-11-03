#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <stdio.h>


static FPSCameraZ _camera;
static int _allowCameraMovement = 1;

#define TRIGGER_BOXTARGET_LEVEL_1 "BoxTarget"
#define TRIGGER_MEMORY_1 "Memory1"


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


void ScriptAction_onBoxInPlaceLevel1(Script *script, ScriptAction *action)
{
    extern Sound triggerSfx;
    extern Sound gateOpeningSfx;
    Level *level = Game_getLevel();
    if (Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_1))
    {
        PlaySound(triggerSfx);
        PlaySound(gateOpeningSfx);
    }
}


extern Sound landingSfx;
static void ScriptAction_level1_transition_landing(Script *script, ScriptAction *action)
{
    static bool done = false;
    if (!done)
    {
        if (!IsSoundPlaying(landingSfx))
            PlaySound(landingSfx);
            done = true;
    }
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    Game_setFogGradient(174.0/512.0, 104.0/512.0, 0.55f, 0.26f);
    DisableCursor();
    // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    _camera.camera = (Camera){0};
    _camera.camera.position = (Vector3){ -9.0f, 1.70f, -4.0f };
    _camera.camera.target = (Vector3){ 90.0f, 100.0f, 180.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.rotation.y = 200.0f * DEG2RAD;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 50.0f;

    Level_load(Game_getLevel(), "resources/levels/test1.lvl");
    int step = 0;
    _allowCameraMovement = 0;


    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 1,
        .action = ScriptAction_level1_transition_landing,
        .actionInt = 1
    });

    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 1,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(0.7f, DB8_BLACK, FADE_TYPE_TOP_DOWN_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, -1.0f)});
    step += 1;

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 0});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_lookCameraAt,
        .actionData = ScriptAction_LookCameraAtData_new(
            &_camera, 2.5f, (Vector3){-9.0f, 1.4f, -2.0})});

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

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_onBoxInPlaceLevel1, .actionData = Scene_alloc(sizeof(BoxInPlaceData), &(BoxInPlaceData){.timeInPlace = 0.0f}) });


    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_progressNextOnTriggeredOn, .actionData = (char*)TRIGGER_MEMORY_1 });
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 0});

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "WOW! I can't believe you figured it out!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "Well.. Now that I think about it..\n"
            "I'm not sure why the others didn't figure it out.", 1)});
    step += 1;

    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_lookCameraAt,
        .actionData = ScriptAction_LookCameraAtData_new(
            &_camera, 2.5f, (Vector3){3.0f, 2.2f, 15.0})});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "By the way.\n"
            "I wasn't always a ghost, you know!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("You:",
            "(Isn't that true for all ghosts?)", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "A long time ago, [color=grey] under a full moon[/color], our island was attacked.\n"
            "August did what he could to protect me, but, the invading force was too quick.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "Our children managed to take a boat to the mainland with the servants\n"
            "But we weren't able to make it to a boat in time.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "It was a tragic night, but I am at peace.\n"
            "I can not say the same for August.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "Sorry about that.\n"
            "All of that aside, let's continue and try to get you out of here!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "This castle can be confusing, so I'll show you the way to the next area.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_loadScene, .actionInt = SCENE_ID_PUZZLE_2 });





}



static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    EnableCursor();
}


SceneConfig _scene_puzzle_1 = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_PUZZLE_1,
};