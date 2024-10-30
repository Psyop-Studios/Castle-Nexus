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
            .bounds = (Rectangle) { 300, 5, 180, 20 },
        }, &resultBuffer);
        
        
        if (resultBuffer)
        {
            
            strncpy(_levelFileNameBuffer, resultBuffer, 256);
        }
        
        if (DuskGui_button((DuskGuiParams) {
            .text = "Save",
            .rayCastTarget = 1,
            .bounds = (Rectangle) { 500, 5, 100, 20 },
        }))
        
        {
            Level_save(Game_getLevel(), _levelFileNameBuffer);
        }
        

        if (DuskGui_button((DuskGuiParams) {
            .text = "Load",
            .rayCastTarget = 1,
            .bounds = (Rectangle) { 600, 5, 100, 20 },
        }))
        {
            DuskGui_openMenu("LoadMenu");
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
                
                
                // Position input fields
                sprintf(buffer, "%.3f##X-%d:%d", instance->position.x, i, j);
                
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 10, posY, 60, 20 },
                }, &instance->position.x, _worldCursor.x - 1.0f, _worldCursor.x + 1.0f, 0.025f))
                {
                    Level_updateInstanceTransform(instance);
                }
                
                sprintf(buffer, "%.3f##Y-%d:%d", instance->position.y, i, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 70, posY, 60, 20 },
                }, &instance->position.y, _worldCursor.y - 2.0f, _worldCursor.y + 4.0f, 0.025f))
                {
                    Level_updateInstanceTransform(instance);
                }
                
                sprintf(buffer, "%.3f##Z-%d:%d", instance->position.z, i, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 130, posY, 60, 20 },
                }, &instance->position.z, _worldCursor.z - 1.0f, _worldCursor.z + 1.0f, 0.025f))
                {
                    Level_updateInstanceTransform(instance);
                }
                
                posY += 20.0f;
                
                // Rotation input fields
                sprintf(buffer, "%.3f##RX-%d:%d", instance->eulerRotationDeg.x, i, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 10, posY, 60, 20 },
                }, &instance->eulerRotationDeg.x, -3000.0f, 3000.0f, 1.0f))
                {
                    Level_updateInstanceTransform(instance);
                }
                
                sprintf(buffer, "%.3f##RY-%d:%d", instance->eulerRotationDeg.y, i, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 70, posY, 60, 20 },
                }, &instance->eulerRotationDeg.y, -3000.0f, 3000.0f, 1.0f))
                {
                    Level_updateInstanceTransform(instance);
                }
                
                sprintf(buffer, "%.3f##RZ-%d:%d", instance->eulerRotationDeg.z, i, j);
                if (DuskGui_floatInputField((DuskGuiParams) {
                    .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 130, posY, 60, 20 },
                }, &instance->eulerRotationDeg.z, -3000.0f, 3000.0f, 1.0f))
                {
                    Level_updateInstanceTransform(instance);
                }
                
                posY += 20.0f;
                if (DuskGui_button((DuskGuiParams) {
                    .text = "Reset Rotation", .rayCastTarget = 1, .bounds = (Rectangle) { 10, posY, 60, 20 }}))
                {
                    instance->eulerRotationDeg = (Vector3){0, 0, 0};
                    Level_updateInstanceTransform(instance);
                }
                

                if (DuskGui_button((DuskGuiParams) {
                    .text = "<-", .rayCastTarget = 1, .bounds = (Rectangle) { 70, posY, 60, 20 }}))
                {
                    instance->eulerRotationDeg.y += -45.0f;
                    Level_updateInstanceTransform(instance);
                }
                
                if (DuskGui_button((DuskGuiParams) {
                    .text = "->", .rayCastTarget = 1, .bounds = (Rectangle) { 130, posY, 60, 20 }}))
                {
                    instance->eulerRotationDeg.y -= -45.0f;
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

    
    DuskGuiParamsEntry* menu;
    if ((menu = DuskGui_beginMenu((DuskGuiParams) {
        .text = "LoadMenu",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 600, 25, 110, 60 },
    })))
    {
        
        FilePathList levelFiles = LoadDirectoryFilesEx("resources/levels", ".lvl", 0);
        
        for (int i = 0; i < levelFiles.count; i++)
        {
            
            char *filename = levelFiles.paths[i];
            char *lastSlash = strrchr(filename, '/');
            if (lastSlash)
            {
                filename = lastSlash + 1;
            } else {
                lastSlash = filename;
            }
            

            if (DuskGui_menuItem(0, (DuskGuiParams) {
                .text = lastSlash,
                .rayCastTarget = 1,
                .bounds = (Rectangle) { 5, 5 + i * 20, 100, 20 },
            }))
            {
                Level_load(Game_getLevel(), levelFiles.paths[i]);
                char *filename = GetFileNameWithoutExt(levelFiles.paths[i]);
                strncpy(_levelFileNameBuffer, filename, 256);
                DuskGui_closeMenu("LoadMenu");
            }
            
        }
        menu->params.bounds.height = 10 + levelFiles.count * 20;
        
        UnloadDirectoryFiles(levelFiles);
        
        DuskGui_endMenu();
    }
    else
    {
        DuskGui_closeMenu("LoadMenu");
    }
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