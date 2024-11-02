#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <stdio.h>

typedef struct FPSCamera {
    Camera camera;
    Vector3 rotation;
    Vector3 velocity;
    float velocityDecayRate;
    float acceleration;
} FPSCamera;

static FPSCamera _camera;
static int _allowCameraMovement = 0;

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
        if (_camera.rotation.x > PI / 2 * .9f)
        {
            _camera.rotation.x = PI / 2 * .9f;
        }
        if (_camera.rotation.x < -PI / 2 * .9f)
        {
            _camera.rotation.x = -PI / 2 * .9f;
        }
        Vector3 rotatedForward = Vector3Transform((Vector3){0,0,1.0f}, MatrixRotateZYX(_camera.rotation));
        _camera.camera.target = Vector3Add(_camera.camera.position, rotatedForward);
    }

    Vector3 moveDelta = Vector3Scale(_camera.velocity, dt);
    _camera.camera.position = Vector3Add(_camera.camera.position, moveDelta);
    _camera.camera.target = Vector3Add(_camera.camera.target, moveDelta);
    float decay = 1.0f - _camera.velocityDecayRate * dt;
    _camera.velocity.x *= decay;
    _camera.velocity.z *= decay;

    LevelCollisionResult results[4] = {0};
    int resultCount = Level_findCollisions(level, 
        Vector3Add(_camera.camera.position, (Vector3){0,-1.25f,0}), 0.5f, 1, 0, results, 4);
    
    // gravity
    _camera.velocity = Vector3Add(_camera.velocity, (Vector3) {0, -24.0f * dt, 0});
    
    Vector3 totalShift = {0};
    for (int i = 0; i < resultCount; i++)
    {
        Vector3 normal = results[i].normal;

        if (normal.y > 0.25f)
        {
            // lets assume upward facing normals are flat floors to avoid glitches
            normal = (Vector3){0,1.0f,0};
        }
        Vector3 shift = Vector3Scale(normal, results[i].depth);
        if (fabsf(shift.y) > fabsf(totalShift.y))
        {
            totalShift.y = shift.y;
        }
        if (fabsf(shift.x) > fabsf(totalShift.x))
        {
            totalShift.x = shift.x;
        }
        if (fabsf(shift.z) > fabsf(totalShift.z))
        {
            totalShift.z = shift.z;
        }
        // cancel velocity in direction of normal
        // printf("velocity: %f %f %f -> ", _camera.velocity.x, _camera.velocity.y, _camera.velocity.z);
        if (normal.y > 0.25f)
        {
            _camera.velocity.y = 0;
        }
        // _camera.velocity = Vector3Subtract(_camera.velocity, Vector3Scale(normal, Vector3DotProduct(_camera.velocity, results[i].normal)));
        // printf(" %f %f %f\n", _camera.velocity.x, _camera.velocity.y, _camera.velocity.z);
    }

    _camera.camera.position = Vector3Add(_camera.camera.position, totalShift);
    _camera.camera.target = Vector3Add(_camera.camera.target, totalShift);
        


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
    _camera.camera.position = (Vector3){ 3.0f, 1.70f, 4.0f };
    _camera.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 25.0f;

    Level_load(Game_getLevel(), "resources/levels/docks.lvl");
    int step = 0;
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "It was a [color=red_]dark and stormy night[/color] ...", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "You are an investigative journalist tasked", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "with chasing some [color=grey]rumors[/color].", (Rectangle){10, 10, 200, 100})});

    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "You wanna write some stories, huh?", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "I've got a scoop for ya.", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "I happen to know a local island is haunted by ghosts.", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 2",  "Don't bother listening to him.", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 2",  "He just likes telling stories. ", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "Nonesense, I've seen em myself!", (Rectangle){10, 10, 200, 100})});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_drawTextRect,
        .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "As a matter of fact, I'll take you for free.", (Rectangle){10, 10, 200, 100})});
        
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

SceneConfig _scene_intro_scene = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_START_INTRO,
};