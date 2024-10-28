#include "scene.h"
#include "_scenes.h"

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

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
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

    TraceLog(LOG_INFO, "SceneInit: %d done", SceneConfig->sceneId);
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