#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>

typedef struct FPSCamera {
    Camera camera;
    Vector3 rotation;
    Vector3 velocity;
    float velocityDecayRate;
    float acceleration;
} FPSCamera;

static FPSCamera _camera;
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
    if (_allowCameraMovement)
    {
        Vector3 move = {0};
        if (IsKeyDown(KEY_W))
        {
            move.z = 1;
        }
        if (IsKeyDown(KEY_S))
        {
            move.z = -1;
        }
        if (IsKeyDown(KEY_A))
        {
            move.x = -1;
        }
        if (IsKeyDown(KEY_D))
        {
            move.x = 1;
        }

        Vector3 forwardXZ = Vector3Normalize((Vector3){_camera.camera.target.x - _camera.camera.position.x, 0, _camera.camera.target.z - _camera.camera.position.z});
        Vector3 right = Vector3CrossProduct(forwardXZ, (Vector3){0, 1, 0});
        if (Vector3Length(move) > 0.0f)
        {
            move = Vector3Normalize(move);
            move = Vector3Add(Vector3Scale(right, move.x), Vector3Scale(forwardXZ, move.z));

            _camera.velocity = Vector3Add(_camera.velocity, Vector3Scale(move, _camera.acceleration * dt));
        }

        Vector2 mouseDelta = GetMouseDelta();
        // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
        _camera.rotation.y -= mouseDelta.x * 0.002f;
        _camera.rotation.x += mouseDelta.y * 0.002f;
        // Matrix yaw = MatrixRotateY(-mouseDelta.x * 0.002f);
        // Matrix pitch = MatrixRotateX(-mouseDelta.y * 0.002f);
        // Vector3 forward = Vector3Normalize(Vector3Subtract(_camera.camera.target, _camera.camera.position));
        // Vector3 rotatedForward = Vector3Transform(forward, MatrixMultiply(yaw, pitch));
        Vector3 rotatedForward = Vector3Transform((Vector3){0,0,1.0f}, MatrixRotateZYX(_camera.rotation));
        _camera.camera.target = Vector3Add(_camera.camera.position, rotatedForward);
    }

    Vector3 moveDelta = Vector3Scale(_camera.velocity, dt);
    _camera.camera.position = Vector3Add(_camera.camera.position, moveDelta);
    _camera.camera.target = Vector3Add(_camera.camera.target, moveDelta);
    _camera.velocity = Vector3Scale(_camera.velocity, 1.0f - _camera.velocityDecayRate * dt);

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