#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"

static Model _model;
static Camera _camera;

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    BeginMode3D(_camera);
    DrawModel(_model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    EndMode3D();

    if (IsMouseButtonDown(0))
    {
        UpdateCamera(&_camera, CAMERA_FIRST_PERSON);
    }
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{

}

static void Step1_UI(Script *script, ScriptAction *action)
{
    DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams){
        .bounds = {10, 300, 200, 50},
    });
    DuskGui_label((DuskGuiParams){
        .text = "Just a small UI example showing clipping support",
        .bounds = {10, 10, 200, 18},
    });
    if (DuskGui_button((DuskGuiParams){
        .bounds = {10, 30, 300, 18},
        .rayCastTarget = 1,
        .text = "Click me!",
    }))
    {
        TraceLog(LOG_INFO, "Button clicked!");
    }
    DuskGui_endPanel(panel);
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    if (gameCtx->currentSceneId == SCENE_ID_START)
    {
        TraceLog(LOG_INFO, "SceneInit: %d", SceneConfig->sceneId);
    }

    TraceLog(LOG_INFO, "SceneInit: %d", SceneConfig->sceneId);
    _model = LoadModel("resources/level-blocks.glb");
    _model.materials[0].shader = _modelDitherShader;
    _model.materials[1].shader = _modelDitherShader;
    _model.materials[2].shader = _modelTexturedShader;

    _camera = (Camera){0};
    _camera.position = (Vector3){ 10.0f, 1.70f, 10.0f };
    _camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.fovy = 45.0f;
    _camera.projection = CAMERA_PERSPECTIVE;

    TraceLog(LOG_INFO, "_modelMeshCount: %d", _model.meshCount);

    TraceLog(LOG_INFO, "SceneInit: %d done", SceneConfig->sceneId);


    int step = 0;
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "It was a [color=red_]dark and stormy night[/color] ...", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_jumpStep,
        .actionData = ScriptAction_JumpStepData_new(-1, 1, 1)});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = Step1_UI,
    });
    step++;

    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("", "the next step!", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_jumpStep,
        .actionData = ScriptAction_JumpStepData_new(-1, 1, 1)});
}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    UnloadModel(_model);
}

SceneConfig _scene_start = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_START,
};