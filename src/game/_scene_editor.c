#include "scene.h"
#include "_scenes.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <math.h>
#include "level.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static Camera _camera;

static Vector3 _worldCursor = {0};
static int _updateCamera;
static char _levelFileNameBuffer[256] = {0};
static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    // ClearBackground(DB8_BG_DEEPPURPLE);
    // TraceLog(LOG_INFO, "SceneDraw: %d", SceneConfig->sceneId);
    BeginMode3D(_camera);

    Level *level = Game_getLevel();
    Level_draw(level);

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

    if (_updateCamera)
    {
        UpdateCamera(&_camera, CAMERA_FIRST_PERSON);
    }
}

static void SceneDrawUi(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    _updateCamera = DuskGui_dragArea((DuskGuiParams) {
        .text = "Camera",
        .bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() },
        .rayCastTarget = 1,
    });
    // DrawRectangle(0, 0, 200, 200, DB8_WHITE);
    DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { -5, -5, GetScreenWidth() + 10, 30 },
        .rayCastTarget = 1,
    });
    
    DuskGui_label((DuskGuiParams) {
        .text = TextFormat("Cursor: %.2f %.2f %.2f", _worldCursor.x, _worldCursor.y, _worldCursor.z),
        .bounds = (Rectangle) { 10, 10, 180, 20 },
    });

    {
        char *resultBuffer = NULL;
        DuskGui_textInputField((DuskGuiParams) {
            .text = _levelFileNameBuffer,
            .isFocusable = 1,
            .rayCastTarget = 1,
            .bounds = (Rectangle) { 300, 10, 180, 20 },
        }, &resultBuffer);
        
        if (resultBuffer)
        {
            strncpy(_levelFileNameBuffer, resultBuffer, 256);
        }


    }
    // if (DuskGui_button((DuskGuiParams) {
    //     .text = "Editor",
    //     .rayCastTarget = 1,
    //     .bounds = (Rectangle) { 10, 10, 100, 20 },
    // })) {
    //     TraceLog(LOG_INFO, "Button clicked");
    // }
    DuskGui_endPanel(panel);

    DuskGuiParamsEntryId objectCreatePanel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { 0, 20, 200, GetScreenHeight() - 20},
        .rayCastTarget = 1,
    });
    Level *level = Game_getLevel();
    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];
        char *lastSlash = strrchr(mesh->filename, '/');
        if (DuskGui_button((DuskGuiParams) {
            .rayCastTarget = 1,
            .text = lastSlash ? lastSlash + 1 : mesh->filename,
            .bounds = (Rectangle) { 10, 10 + i * 20, 180, 20 },
        }))
        {
            Level_addInstance(level, mesh->filename, _worldCursor, (Vector3){0, 0, 0}, (Vector3){1, 1, 1});
        }
    }
    DuskGui_endPanel(objectCreatePanel);
    DuskGuiParamsEntryId objectEditPanel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { GetScreenWidth() - 200, 20, 200, GetScreenHeight() - 20},
        .rayCastTarget = 1,
    });

    float posY = 10.0f;

    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];
        for (int j = 0; j < mesh->instanceCount; j++)
        {
            LevelMeshInstance *instance = &mesh->instances[j];
            Vector3 diff = Vector3Subtract(instance->position, _worldCursor);
            float dx = fabsf(diff.x);
            float dz = fabsf(diff.z);
            if (dx <= 1.0f && dz <= 1.0f)
            {
                DuskGui_horizontalLine((DuskGuiParams) {
                    .text = mesh->filename,
                    .bounds = (Rectangle) { 10, posY, 180, 1 },
                });
                posY += 8.0f;
                char buffer[128];
                sprintf(buffer, "%.3f##X-%d", instance->position.x, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer,
                    .rayCastTarget = 1,
                    .bounds = (Rectangle) { 10, posY, 60, 20 },
                }, &instance->position.x, _worldCursor.x - 1.0f, _worldCursor.x + 1.0f))
                {
                    Level_updateInstanceTransform(instance);
                }
                sprintf(buffer, "%.3f##Y-%d", instance->position.y, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer,
                    .rayCastTarget = 1,
                    .bounds = (Rectangle) { 70, posY, 60, 20 },
                }, &instance->position.y, _worldCursor.y - 2.0f, _worldCursor.y + 4.0f))
                {
                    Level_updateInstanceTransform(instance);
                }
                sprintf(buffer, "%.3f##Z-%d", instance->position.z, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer,
                    .rayCastTarget = 1,
                    .bounds = (Rectangle) { 130, posY, 60, 20 },
                }, &instance->position.z, _worldCursor.z - 1.0f, _worldCursor.z + 1.0f))
                {
                    Level_updateInstanceTransform(instance);
                }

                posY += 20.0f;
                if (DuskGui_button((DuskGuiParams) {
                    .text = "Delete",
                    .rayCastTarget = 1,
                    .bounds = (Rectangle) { 10, posY, 180, 20 },
                }))
                {
                    for (int k = j; k < mesh->instanceCount - 1; k++)
                    {
                        mesh->instances[k] = mesh->instances[k + 1];
                    }
                    mesh->instanceCount--;
                }

                posY += 30.0f;
            }
        }
    }

    DuskGui_endPanel(objectEditPanel);
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