#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>

static FPSCameraZ _camera;
static int _allowCameraMovement = 1;

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
    Level *level = Game_getLevel();
    level->isEditor = 0;
    FPSCamera_update(&_camera, level, _allowCameraMovement, dt);
    Level_update(level, dt);
}

static void ScriptAction_setCameraMovementEnabled(Script *script, ScriptAction *action)
{
    _allowCameraMovement = action->actionInt;
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    DisableCursor();
    // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    _camera.camera = (Camera){0};
    _camera.camera.position = (Vector3){ 30.0f, 1.70f, -32.0f };
    _camera.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 25.0f;

    Level_load(Game_getLevel(), "resources/levels/island-fix.lvl");
    int step = 0;
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter 2",  "After a short ride, you arrive at the [color=blue]island.[/color]", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter 2",  "You get off the boat.", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter 2",  "You look around to say thank you, but..", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter 2",  "[color=grey] they're already gone.[/color]", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_setCameraMovementEnabled,
        .actionInt = 1});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_jumpStep,
        .actionData = ScriptAction_JumpStepData_new(-1, 1, 1)});
    step++;
    
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_setCameraMovementEnabled,
        .actionInt = 1});
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