#include "main.h"
#include <math.h>
#include <stdio.h>
#include <external/glad.h>
#include <memory.h>
#include <raymath.h>
#include "scriptsystem.h"
#include "scriptactions.h"
#include "scene.h"
#include "_scenes.h"
#include "dusk-gui.h"
#include "level.h"
#include "level_components.h"

static GameContext *_contextData;

static Shader _outlineShader;
static Shader _colorReduceShader;
static RenderTexture2D _target = {0};
static RenderTexture2D _finalTarget = {0};
static Level _level = {0};
static Texture2D _fogTex = {0};

// TODO: maybe move this elsewere
Sound walkSfx1;
Sound walkSfx2;
Sound jumpSfx;
Sound waterSfx1;
Sound waterSfx2;
Sound pushSfx;
Sound talkSfxMale1;
Sound talkSfxMale2;
Sound talkSfxMale3;
Sound talkSfxFemale1;
Sound talkSfxFemale2;
Sound talkSfxFemale3;


Shader _modelDitherShader;
Shader _modelTexturedShader;
Font _fntMono = {0};
Font _fntMedium = {0};
Camera _currentCamera;

void UpdateRenderTexture()
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int downScaleFac = 1;
    if ((screenWidth >> downScaleFac) != _target.texture.width || (screenHeight >> downScaleFac) != _target.texture.height)
    {
        UnloadRenderTexture(_target);
        _target = LoadRenderTexture(screenWidth>>downScaleFac, screenHeight>>downScaleFac);
        SetTextureFilter(_target.texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(_target.texture, TEXTURE_WRAP_CLAMP);

        UnloadRenderTexture(_finalTarget);
        _finalTarget = LoadRenderTexture(screenWidth, screenHeight);
        SetTextureFilter(_finalTarget.texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(_finalTarget.texture, TEXTURE_WRAP_CLAMP);
    }
}


void Game_init(void** contextData)
{
    printf("Game_init\n");

    DuskGui_init();
    InitAudioDevice();

    // TODO: move this elsewere
    walkSfx1 = LoadSound("resources/audio/WalkSand1.wav");
    walkSfx2 = LoadSound("resources/audio/WalkSand2.wav");
    jumpSfx = LoadSound("resources/audio/Jump.wav");
    waterSfx1 = LoadSound("resources/audio/WaterSounds.wav");
    waterSfx2 = LoadSound("resources/audio/WaterSounds2.wav");
    pushSfx = LoadSound("resources/audio/CrateSound1.wav");
    talkSfxMale1 = LoadSound("resources/audio/ATalk1.wav");
    talkSfxMale2 = LoadSound("resources/audio/ATalk2.wav");
    talkSfxMale3 = LoadSound("resources/audio/ATalk3.wav");
    talkSfxFemale1 = LoadSound("resources/audio/CTalk1.wav");
    talkSfxFemale2 = LoadSound("resources/audio/CTalk2.wav");
    talkSfxFemale3 = LoadSound("resources/audio/CTalk3.wav");

    if (*contextData == NULL)
    {
        *contextData = MemAlloc(sizeof(GameContext));
        memset(*contextData, 0, sizeof(GameContext));

        _contextData = (GameContext*) *contextData;
        _contextData->currentSceneId = SCENE_ID_INVALID;
        _contextData->nextSceneId = SCENE_ID_START;
    }
    else
    {
        _contextData = (GameContext*) *contextData;
        _contextData->nextSceneId = _contextData->currentSceneId;
        _contextData->currentSceneId = SCENE_ID_INVALID;
    }

    printf("Game_init\n");


    _modelDitherShader = LoadShader("resources/dither.vs", "resources/dither.fs");
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "drawInnerOutlines"), (float[]){1.0f}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "uvDitherBlockPosScale"), (float[]){16.0f}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "uvOverride"), (float[]){0.0f, 0.0f}, SHADER_UNIFORM_VEC2);
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "texSize"), (float[]){128.0f, 128.0f}, SHADER_UNIFORM_VEC2);
    _modelTexturedShader = LoadShader("resources/dither.vs", "resources/dither.fs");
    SetShaderValue(_modelTexturedShader, GetShaderLocation(_modelTexturedShader, "drawInnerOutlines"), (float[]){0.0f}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_modelTexturedShader, GetShaderLocation(_modelTexturedShader, "uvDitherBlockPosScale"), (float[]){16.0f}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_modelTexturedShader, GetShaderLocation(_modelTexturedShader, "uvOverride"), (float[]){0.0f, 0.0f}, SHADER_UNIFORM_VEC2);

    Vector2 fogPoint = {174.0/512.0, 200.0/512.0};
    Game_setFogGradient(fogPoint.x, fogPoint.y, 0.125f, 0.4f);

    _outlineShader = LoadShader(0, "resources/outline.fs");
    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "depthOutlineEnabled"), (float[]){1.0f}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "uvOutlineEnabled"), (float[]){1.0f}, SHADER_UNIFORM_FLOAT);

    _colorReduceShader = LoadShader(0, "resources/colorreduce.fs");

    _fntMedium = LoadFont("resources/fnt_medium.png");
    _fntMono = LoadFont("resources/fnt_mymono.png");

    UpdateRenderTexture();
    SetTextLineSpacingEx(-6);

    Level_init(&_level);
    LevelComponents_register(&_level);

    Level_loadAssets(&_level, "resources/level_assets");

    _fogTex = Level_getTexture(&_level, "db8-dither.png", (Texture2D){0});
    DuskGui_setDefaultFont(_fntMedium, _fntMedium.baseSize, -1);
}

void Game_setFogGradient(float x, float y, float fogScale, float fogPower)
{
    Vector2 fogPoint = {x, y};
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "fogPoint"), (float[]){fogPoint.x, fogPoint.y}, SHADER_UNIFORM_VEC2);
    SetShaderValue(_modelTexturedShader, GetShaderLocation(_modelTexturedShader, "fogPoint"), (float[]){fogPoint.x, fogPoint.y}, SHADER_UNIFORM_VEC2);
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "fogScale"), (float[]){fogScale}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_modelTexturedShader, GetShaderLocation(_modelTexturedShader, "fogScale"), (float[]){fogScale}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "fogPower"), (float[]){fogPower}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_modelTexturedShader, GetShaderLocation(_modelTexturedShader, "fogPower"), (float[]){fogPower}, SHADER_UNIFORM_FLOAT);
}

void Game_setFogTextures(Material *mtl)
{
    // printf("Fog texture: %s %d %d\n", fogTex->filename, fogTex->texture.width, fogTex->texture.height);
    // for (int i = 0; i < 4; i++) rlActiveTextureSlot(i);
    mtl->maps[MATERIAL_MAP_METALNESS].texture = _fogTex;
    mtl->maps[MATERIAL_MAP_METALNESS].color = WHITE;
}

Level *Game_getLevel()
{
    return &_level;
}

void Game_deinit()
{
    printf("Game_deinit\n");

    UnloadSound(walkSfx1);
    UnloadSound(walkSfx2);
    UnloadSound(jumpSfx);
    UnloadSound(waterSfx1);
    UnloadSound(waterSfx2);
    UnloadSound(pushSfx);
    UnloadSound(talkSfxMale1);
    UnloadSound(talkSfxMale2);
    UnloadSound(talkSfxMale3);
    UnloadSound(talkSfxFemale1);
    UnloadSound(talkSfxFemale2);
    UnloadSound(talkSfxFemale3);

    SceneConfig *config = Scene_getConfig(_contextData->currentSceneId);
    if (config && config->deinitFn)
    {
        config->deinitFn(_contextData, config);
    }

    CloseAudioDevice();
    UnloadShader(_modelDitherShader);
    UnloadShader(_outlineShader);
    UnloadRenderTexture(_target);
    UnloadRenderTexture(_finalTarget);
    UnloadFont(_fntMedium);
    UnloadFont(_fntMono);
    Level_unload(&_level);
}

void DrawScene()
{
    float clockTime = GetTime();
    SetShaderValue(_modelDitherShader, GetShaderLocation(_modelDitherShader, "time"), &clockTime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "depthOutlineEnabled"), (float[]){1.0f}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "uvOutlineEnabled"), (float[]){1.0f}, SHADER_UNIFORM_FLOAT);

    SceneConfig *config = Scene_getConfig(_contextData->currentSceneId);
    if (config)
    {
        config->updateFn(_contextData, config, GetFrameTime());
        config->drawLevelFn(_contextData, config);
    }

}

void DrawUi()
{
    SceneConfig *config = Scene_getConfig(_contextData->currentSceneId);
    if (config && config->drawUiFn)
    {
        config->drawUiFn(_contextData, config);
    }
}

void Game_update()
{
    _contextData->currentGameTime += GetFrameTime();

    if (_contextData->nextSceneId != SCENE_ID_INVALID)
    {
        SceneConfig *prevConfig = Scene_getConfig(_contextData->currentSceneId);
        SceneConfig *config = Scene_getConfig(_contextData->nextSceneId);
        if (config)
        {
            if (prevConfig && prevConfig->deinitFn)
            {
                prevConfig->deinitFn(_contextData, prevConfig);
            }

            Script_init(_contextData);
            Scene_init();
            _contextData->currentSceneId = _contextData->nextSceneId;
            config->initFn(_contextData, config);
        }
        else
        {
            TraceLog(LOG_ERROR, "Scene not found: %d", _contextData->nextSceneId);
        }

        _contextData->nextSceneId = SCENE_ID_INVALID;
    }

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    UpdateRenderTexture();

    rlDisableColorBlend();

    BeginTextureMode(_target);
    ClearBackground(DB8_BG_DEEPPURPLE);
    DrawScene();

    EndTextureMode();

    BeginTextureMode(_finalTarget);
    // post processing: draw render texture to screen with outlines and dithering
    rlEnableColorBlend();
    // BeginDrawing();
    BeginShaderMode(_outlineShader);

    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "resolution"), (float[2]){(float)_target.texture.width, (float)_target.texture.height}, SHADER_UNIFORM_VEC2);
    DrawTexturePro(_target.texture,
        (Rectangle){0.0f, 0.0f, (float)_target.texture.width, (float)-_target.texture.height},
        (Rectangle){0.0f, 0.0f, (float)screenWidth, (float)screenHeight},
        (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    EndShaderMode();

    rlEnableColorBlend();

    DrawUi();
    float dt = GetFrameTime();
    // cap dt to 100ms to prevent physics explosions
    dt = fminf(dt, 0.1f);

    Script_update(_contextData, dt);
    Script_draw(_contextData);
    DuskGui_finalize();

    // EndDrawing();
    EndTextureMode();

    BeginDrawing();
    BeginShaderMode(_colorReduceShader);
    DrawTexturePro(_finalTarget.texture,
        (Rectangle){0.0f, 0.0f, (float)_finalTarget.texture.width, (float)-_finalTarget.texture.height},
        (Rectangle){0.0f, 0.0f, (float)screenWidth, (float)screenHeight},
        (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    EndShaderMode();
    EndDrawing();


    if (IsKeyReleased(KEY_Q) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        _contextData->nextSceneId = _contextData->currentSceneId - 1;
    }
    if (IsKeyReleased(KEY_E) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        _contextData->nextSceneId = _contextData->currentSceneId + 1;
    }
    if (IsKeyReleased(KEY_T) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        static uint32_t prevScene;
        if (_contextData->currentSceneId != SCENE_ID_EDITOR)
        {
            prevScene = _contextData->currentSceneId;
            _contextData->nextSceneId = SCENE_ID_EDITOR;
        }
        else
        {
            _contextData->nextSceneId = prevScene;
        }
    }
    if (IsKeyReleased(KEY_U) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        Level_reloadAssets(&_level);
    }
}

void Game_setNextScene(int sceneId)
{
    _contextData->nextSceneId = sceneId;
}

// handles collision detection and movement for a first person camera
void FPSCamera_update(FPSCameraZ *camera, Level *level, int allowCameraMovement, float dt)
{

    static float distanceTraveled = 0.0f;

    if (allowCameraMovement)
    {
        // needed for browser; if we don't do that, the mouse will eventually be released and
        // then the game becomes unplayable
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            DisableCursor();
        }
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

        distanceTraveled += Vector3Length(move) * dt;

        if (distanceTraveled > 0.35f && camera->hasGroundContact)
        {
            float pitch = (float)(GetRandomValue(7, 11)) / 10.0f;
            SetSoundPitch(walkSfx1, pitch);
            SetSoundPitch(walkSfx2, pitch);

            GetRandomValue(0, 1) == 0 ? PlaySound(walkSfx1) : PlaySound(walkSfx2);
            distanceTraveled = 0.0f;
        }

        if (IsKeyDown(KEY_SPACE) && camera->hasGroundContact)
        {
            camera->velocity.y = 8.0f;
            PlaySound(jumpSfx);
        }

        Vector3 forwardXZ = Vector3Normalize((Vector3){camera->camera.target.x - camera->camera.position.x, 0, camera->camera.target.z - camera->camera.position.z});
        Vector3 right = Vector3CrossProduct(forwardXZ, (Vector3){0, 1, 0});
        if (Vector3Length(move) > 0.0f)
        {
            move = Vector3Normalize(move);
            move = Vector3Add(Vector3Scale(right, move.x), Vector3Scale(forwardXZ, move.z));

            camera->velocity = Vector3Add(camera->velocity, Vector3Scale(move, camera->acceleration * dt));
        }

        Vector2 mouseDelta = GetMouseDelta();
        // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
        camera->rotation.y -= mouseDelta.x * 0.002f;
        camera->rotation.x += mouseDelta.y * 0.002f;
        if (camera->rotation.x > PI / 2 * .9f)
        {
            camera->rotation.x = PI / 2 * .9f;
        }
        if (camera->rotation.x < -PI / 2 * .9f)
        {
            camera->rotation.x = -PI / 2 * .9f;
        }

    }

    Vector3 rotatedForward = Vector3Transform((Vector3){0,0,1.0f}, MatrixRotateZYX(camera->rotation));
    camera->camera.target = Vector3Add(camera->camera.position, rotatedForward);


    Vector3 moveDelta = Vector3Scale(camera->velocity, dt);
    level->playerDistanceWalked += Vector3Length((Vector3){moveDelta.x, 0, moveDelta.z});
    camera->camera.position = Vector3Add(camera->camera.position, moveDelta);
    camera->camera.target = Vector3Add(camera->camera.target, moveDelta);
    float decay = 1.0f - camera->velocityDecayRate * dt;
    camera->velocity.x *= decay;
    camera->velocity.z *= decay;

#define COLLIDE_RESULT_COUNT 16
    LevelCollisionResult results[COLLIDE_RESULT_COUNT] = {0};
    int resultCount = Level_findCollisions(level,
        Vector3Add(camera->camera.position, (Vector3){0,-1.25f,0}), 0.5f, 1, 1, results, COLLIDE_RESULT_COUNT);

    // gravity
    camera->velocity = Vector3Add(camera->velocity, (Vector3) {0, -24.0f * dt, 0});

    Vector3 totalShift = {0};
    camera->hasGroundContact = 0;


    for (int i = 0; i < resultCount; i++)
    {
        if (results[i].triggerId)
        {
            Level_addTriggerId(level, results[i].triggerId);
            continue;
        }
        Vector3 normal = results[i].normal;

        if (normal.y > 0.25f)
        {
            // lets assume upward facing normals are flat floors to avoid glitches
            normal = (Vector3){0,1.0f,0};
            camera->hasGroundContact = 1;
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
        // printf("velocity: %f %f %f -> ", camera->velocity.x, camera->velocity.y, camera->velocity.z);
        if (normal.y > 0.25f)
        {
            camera->velocity.y = 0;
        }

        if ((results[i].ownerId.id || results[i].ownerId.generation) && fabsf(normal.y) < 0.25f && Vector3DotProduct(camera->velocity, normal) < 0)
        {
            LevelEntityComponentClass *rigidSphereClass = &level->entityComponentClasses[COMPONENT_TYPE_RIGID_SPHERE];
            for (int j = 0; j < rigidSphereClass->instanceCount; j++)
            {
                if (rigidSphereClass->generations[j] == 0 || rigidSphereClass->ownerIds[j].id != results[i].ownerId.id)
                {
                    continue;
                }
                LevelEntity *entity = Level_resolveEntity(level, rigidSphereClass->ownerIds[j]);
                if (!entity)
                {
                    continue;
                }

                float camVel = Vector3Length(camera->velocity);
                Vector3 normal = results[i].normal;
                normal.y = 0;
                entity->position = Vector3Add(entity->position, Vector3Scale(normal, -camVel * 0.5f * results[i].depth));
                if (!IsSoundPlaying(pushSfx)) {
                    PlaySound(pushSfx);
                }
                break;

            }
        }
        // camera->velocity = Vector3Subtract(camera->velocity, Vector3Scale(normal, Vector3DotProduct(camera->velocity, results[i].normal)));
        // printf(" %f %f %f\n", camera->velocity.x, camera->velocity.y, camera->velocity.z);
    }

    camera->camera.position = Vector3Add(camera->camera.position, totalShift);
    camera->camera.target = Vector3Add(camera->camera.target, totalShift);

}