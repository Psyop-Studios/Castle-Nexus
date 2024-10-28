#include "scene.h"
#include "_scenes.h"
#include "dusk-gui.h"

static Camera _camera;

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    BeginMode3D(_camera);
    // DrawModel(_model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    EndMode3D();

    if (IsMouseButtonDown(0))
    {
        UpdateCamera(&_camera, CAMERA_FIRST_PERSON);
    }
}

static void SceneDrawUi(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    // DrawRectangle(0, 0, 200, 200, DB8_WHITE);
    DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { 10, 10, 200, 200 },
    });
    
    if (DuskGui_button((DuskGuiParams) {
        .text = "Hello",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 10, 10, 100, 20 },
    })) {
        TraceLog(LOG_INFO, "Button clicked");
    }
    DuskGui_endPanel(panel);
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{

}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    TraceLog(LOG_INFO, "Editor : %d", SceneConfig->sceneId);
    // _model = LoadModel("resources/level-blocks.glb");
    // _model.materials[0].shader = _modelDitherShader;
    // _model.materials[1].shader = _modelDitherShader;
    // _model.materials[2].shader = _modelTexturedShader;

    _camera = (Camera){0};
    _camera.position = (Vector3){ 10.0f, 1.70f, 10.0f };
    _camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.fovy = 45.0f;
    _camera.projection = CAMERA_PERSPECTIVE;

    TraceLog(LOG_INFO, "SceneInit: %d done", SceneConfig->sceneId);
}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    // UnloadModel(_model);
}

SceneConfig _scene_editor = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .drawUiFn = SceneDrawUi,
    .sceneId = SCENE_ID_EDITOR,
};