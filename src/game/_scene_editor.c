#include "scene.h"
#include "_scenes.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <math.h>

static Camera _camera;

static Vector3 _worldCursor = {0};

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    ClearBackground(DB8_BG_DEEPPURPLE);
    // TraceLog(LOG_INFO, "SceneDraw: %d", SceneConfig->sceneId);
    BeginMode3D(_camera);
    // DrawModel(_model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    BeginShaderMode(_modelTexturedShader);
    // draw a corner grid around a point
    const int gridCount = 4;
    for (int x = -gridCount; x < gridCount; x++)
    {
        for (int z = -gridCount; z < gridCount; z++)
        {
            Vector3 pos = {x + .5f, 0.0f, z + .5f};
            pos = Vector3Add(pos, _worldCursor);

            DrawLine3D((Vector3){pos.x - .1f, pos.y, pos.z}, (Vector3){pos.x + .1f, pos.y, pos.z}, DB8_GREY);
            DrawLine3D((Vector3){pos.x, pos.y, pos.z - .1f}, (Vector3){pos.x, pos.y, pos.z + .1f}, DB8_GREY);
        }
    }
    DrawCubeWires((Vector3){_worldCursor.x, _worldCursor.y + .5f, _worldCursor.z}, 1.0f, 1.0f, 1.0f, DB8_RED);
    EndShaderMode();

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
        .bounds = (Rectangle) { -5, -5, GetScreenWidth() + 10, 30 },
    });
    
    DuskGui_label((DuskGuiParams) {
        .text = TextFormat("Cursor: %.2f %.2f %.2f", _worldCursor.x, _worldCursor.y, _worldCursor.z),
        .bounds = (Rectangle) { 10, 10, 180, 20 },
    });
    // if (DuskGui_button((DuskGuiParams) {
    //     .text = "Editor",
    //     .rayCastTarget = 1,
    //     .bounds = (Rectangle) { 10, 10, 100, 20 },
    // })) {
    //     TraceLog(LOG_INFO, "Button clicked");
    // }
    DuskGui_endPanel(panel);
}

static float fsign(float x)
{
    return x > 0 ? 1.0f : -1.0f;
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{
    Vector3 camForward = Vector3Normalize((Vector3){_camera.target.x - _camera.position.x, _camera.target.y - _camera.position.y, _camera.target.z - _camera.position.z});
    Vector3 forward = fabsf(camForward.x) < fabsf(camForward.z) ? (Vector3){0, 0, fsign(camForward.z)} : (Vector3){fsign(camForward.x), 0, 0};
    Vector3 right = Vector3CrossProduct(forward, (Vector3){0, 1, 0});

    if (IsKeyReleased(KEY_UP))
    {
        _worldCursor = Vector3Add(_worldCursor, forward);
    }
    if (IsKeyReleased(KEY_DOWN))
    {
        _worldCursor = Vector3Subtract(_worldCursor, forward);
    }
    if (IsKeyReleased(KEY_LEFT))
    {
        _worldCursor = Vector3Subtract(_worldCursor, right);
    }
    if (IsKeyReleased(KEY_RIGHT))
    {
        _worldCursor = Vector3Add(_worldCursor, right);
    }
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    TraceLog(LOG_INFO, "Editor : %d", SceneConfig->sceneId);
    // _model = LoadModel("resources/level-blocks.glb");
    // _model.materials[0].shader = _modelDitherShader;
    // _model.materials[1].shader = _modelDitherShader;
    // _model.materials[2].shader = _modelTexturedShader;

    _camera = (Camera){0};
    _camera.position = (Vector3){ 5.0f, 3.70f, 2.0f };
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