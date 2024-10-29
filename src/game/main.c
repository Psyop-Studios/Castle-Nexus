#include "main.h"
#include <math.h>
#include <stdio.h>
#include <external/glad.h>
#include <memory.h>
#include <raymath.h>
#include "scriptactions.h"
#include "scene.h"
#include "_scenes.h"
#include "dusk-gui.h"


static GameContext *_contextData;

static Shader _outlineShader;
static RenderTexture2D _target = {0};

Shader _modelDitherShader;
Shader _modelTexturedShader;
Font _fntMono = {0};
Font _fntMedium = {0};

void UpdateRenderTexture()
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    if ((screenWidth >> 1) != _target.texture.width || (screenHeight >> 1) != _target.texture.height)
    {
        UnloadRenderTexture(_target);
        _target = LoadRenderTexture(screenWidth>>1, screenHeight>>1);
        SetTextureFilter(_target.texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(_target.texture, TEXTURE_WRAP_CLAMP);
    }
}


void Game_init(void** contextData)
{
    DuskGui_init();
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
    _modelTexturedShader = LoadShader("resources/dither.vs", "resources/dither.fs");
    SetShaderValue(_modelTexturedShader, GetShaderLocation(_modelDitherShader, "drawInnerOutlines"), (float[]){0.0f}, SHADER_UNIFORM_FLOAT);

    _outlineShader = LoadShader(0, "resources/outline.fs");
    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "depthOutlineEnabled"), (float[]){1.0f}, SHADER_UNIFORM_FLOAT);
    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "uvOutlineEnabled"), (float[]){1.0f}, SHADER_UNIFORM_FLOAT);
    

    _fntMedium = LoadFont("resources/fnt_medium.png");
    _fntMono = LoadFont("resources/fnt_mymono.png");

    UpdateRenderTexture();
    SetTextLineSpacingEx(-6);
}

void Game_deinit()
{
    printf("Game_deinit\n");

    SceneConfig *config = Scene_getConfig(_contextData->currentSceneId);
    if (config && config->deinitFn)
    {
        config->deinitFn(_contextData, config);
    }

    UnloadShader(_modelDitherShader);
    UnloadShader(_outlineShader);
    UnloadRenderTexture(_target);
    UnloadFont(_fntMedium);
    UnloadFont(_fntMono);
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

    static int mode = 0;
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    UpdateRenderTexture();

    rlDisableColorBlend();

    BeginTextureMode(_target);
    ClearBackground(BLACK);
    DrawScene();
    
    EndTextureMode();

    // post processing: draw render texture to screen with outlines and dithering
    rlEnableColorBlend();
    BeginDrawing();
    BeginShaderMode(_outlineShader);
   
    SetShaderValue(_outlineShader, GetShaderLocation(_outlineShader, "resolution"), (float[2]){(float)_target.texture.width, (float)_target.texture.height}, SHADER_UNIFORM_VEC2);
    DrawTexturePro(_target.texture, 
        (Rectangle){0.0f, 0.0f, (float)_target.texture.width, (float)-_target.texture.height}, 
        (Rectangle){0.0f, 0.0f, (float)screenWidth, (float)screenHeight}, 
        (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    EndShaderMode();

    rlEnableColorBlend();

    DrawUi();
    
    DuskGui_finalize();

    EndDrawing();


    if (IsKeyReleased(KEY_Q))
    {
        _contextData->nextSceneId = _contextData->currentSceneId - 1;
    }
    if (IsKeyReleased(KEY_E))
    {
        _contextData->nextSceneId = _contextData->currentSceneId + 1;
    }
}