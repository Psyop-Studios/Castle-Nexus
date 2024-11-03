#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <stdio.h>


static FPSCameraZ _camera;
static int _allowCameraMovement = 1;
#define TRIGGER_DOOR_ZONE "TriggerCastleFrontDoor"

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    ClearBackground(DB8_BG_GREY);
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

static void ScriptAction_island_firstStep(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    if (level->playerDistanceWalked > 1.0f)
    {
        script->nextActionId = action->actionIdStart + 1;
        return;
    }
    DrawNarrationBottomBox("Chapter 2:", 
        "After a short ride, the dock workers dropped you off onto the [color=blue]island[/color].\n"
        "Somehow, the dock workers were [color=red_] already gone.[/color]", NULL);
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    Game_setFogGradient(176.0/512.0, 320.0/512.0, 0.175f, 0.4f);
    DisableCursor();
    // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    _camera.camera = (Camera){0};
    _camera.camera.position = (Vector3){ 8.0f, 1.70f, 4.0f };
    _camera.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.rotation.y = 200.0f * DEG2RAD;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 50.0f;

    Level_load(Game_getLevel(), "resources/levels/island-fix.lvl");
    int step = 0;

    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, -1.0f)});
    
    step += 1;
    // message to player to get started
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_island_firstStep });
    step += 1;

    // Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_exitZoneMessage });
    // Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_explorationMessageZone_1 });

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_progressNextOnTriggeredOn, .actionData = (char*)TRIGGER_DOOR_ZONE });
    step += 1;
    
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 0});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("You:",
            "* knocks on door *", 1)});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("You:",
            "Hello, is anyone there?", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("Spooky voice:",
            "Who goes there?", 1)});
    step += 1;

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("Spooky voice:",
            "You believe you have the right to invade my land? ", 1)});
    step += 1;

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("Spooky voice:",
            "You lot may have won last time, but not again!", 1)});
    step += 1;

    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(0.4f, DB8_BLACK, FADE_TYPE_BOTTOM_UP_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, 1.0f)});
    

    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_loadScene, .actionInt = SCENE_ID_PUZZLE_1 });
}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    EnableCursor();
}


SceneConfig _scene_island = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_START_ISLAND,
};