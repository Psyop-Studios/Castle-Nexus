#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"

static Camera _camera;
static int _allowCameraMovement = 1;

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    BeginMode3D(_camera);
    _currentCamera = _camera;
    
    Level *level = Game_getLevel();
    
    Level_draw(level);
    
    EndMode3D();

    if (IsMouseButtonDown(0) && _allowCameraMovement)
    {
        UpdateCamera(&_camera, CAMERA_FIRST_PERSON);
    }
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{
    
}

static void ScriptAction_setCameraMovementEnabled(Script *script, ScriptAction *action)
{
    _allowCameraMovement = action->actionInt;
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    _camera = (Camera){0};
    _camera.position = (Vector3){ 3.0f, 1.70f, 4.0f };
    _camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.fovy = 45.0f;
    _camera.projection = CAMERA_PERSPECTIVE;

    Level_load(Game_getLevel(), "resources/levels/docks.lvl");
    int step = 0;
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "It was a [color=red_]dark and stormy night[/color] ...", (Rectangle){10, 10, 200, 100})});
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
}

SceneConfig _scene_intro_scene = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_START_INTRO,
};