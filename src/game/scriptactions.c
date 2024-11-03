#include "main.h"
#include "scene.h"
#include "util.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "scriptactions.h"

typedef struct ScriptAction_DrawRectData {
    const char *title;
    const char *text;
    Rectangle rect;
} ScriptAction_DrawRectData;

void* ScriptAction_DrawTextRectData_new(const char *title, const char *text, Rectangle rect)
{
    ScriptAction_DrawRectData *data = Scene_alloc(sizeof(ScriptAction_DrawRectData),
        &(ScriptAction_DrawRectData){
            .title = title,
            .text = text,
            .rect = rect,
        });
    return data;
}

void ScriptAction_drawTextRect(Script *script, ScriptAction *action)
{
    ScriptAction_DrawRectData *data = action->actionData;
    DrawRectangleRec(data->rect, DB8_WHITE);
    DrawRectangleLinesEx(data->rect, 1, BLACK);
    DrawRectangleLinesEx((Rectangle){data->rect.x + 1, data->rect.y + 1, data->rect.width - 2, data->rect.height - 2}, 1, DB8_BLACK);
    DrawTextBoxAligned(_fntMedium, data->title, data->rect.x + 6, data->rect.y + 6, data->rect.width - 12, data->rect.height - 12, 0.5f, 0.0f, DB8_WHITE);
    DrawTextBoxAligned(_fntMedium, data->text, data->rect.x + 6, data->rect.y + 32, data->rect.width - 12, data->rect.height - 36, 0.0f, 0.0f, DB8_WHITE);
    // DrawTextEx(fntMedium, data->text, (Vector2){data->rect.x + 4, data->rect.y + 4}, fntMedium.baseSize * 2.0f, -2.0f, DB8_WHITE);
}

typedef struct ScriptAction_DrawMagnifiedTextureData {
    Rectangle srcRect;
    Rectangle dstRect;
    Texture2D *texture;
    Shader shader;
} ScriptAction_DrawMagnifiedTextureData;

void* ScriptAction_DrawMagnifiedTextureData_new(Rectangle srcRect, Rectangle dstRect, Texture2D *texture, Shader shader)
{
    ScriptAction_DrawMagnifiedTextureData *data = Scene_alloc(sizeof(ScriptAction_DrawMagnifiedTextureData),
        &(ScriptAction_DrawMagnifiedTextureData){
            .srcRect = (Rectangle){
                srcRect.x / texture->width, srcRect.y / texture->height,
                srcRect.width / texture->width, srcRect.height / texture->height},
            .dstRect = dstRect,
            .texture = texture,
            .shader = shader,
        });
    return data;
}

void ScriptAction_drawMagnifiedTexture(Script *script, ScriptAction *action)
{
    ScriptAction_DrawMagnifiedTextureData *data = action->actionData;
    Rectangle srcRect = data->srcRect;
    Texture2D texture = *data->texture;
    srcRect.x *= texture.width;
    srcRect.y = (1.0f - srcRect.y - srcRect.height) * texture.height;
    Rectangle srcRectScreen = data->srcRect;
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();
    // float scale = screenWidth / texture.width;
    srcRectScreen.x *= screenWidth;
    srcRectScreen.y *= screenHeight;
    srcRectScreen.width *= screenWidth;
    srcRectScreen.height *= screenHeight;
    // TraceLog(LOG_INFO, "srcRectScreen: %f %f %f %f", srcRectScreen.x, srcRectScreen.y, srcRectScreen.width, srcRectScreen.height);
    srcRect.width *= texture.width;
    srcRect.height *= -texture.height;
    // TraceLog(LOG_INFO, "srcRect: %f %f %f %f", srcRect.x, srcRect.y, srcRect.width, srcRect.height);
    DrawRectangleLinesEx((Rectangle){srcRectScreen.x - 1.0f, srcRectScreen.y - 1.0f, srcRectScreen.width + 2.0f, srcRectScreen.height + 2.0f}, 4.0f, DB8_WHITE);
    DrawRectangleLinesEx(srcRectScreen, 2.0f, BLACK);

    for (float lw = 4.0f; lw >= 2.0f; lw -= 2.0f)
    {
        Color color = lw == 2.0f ? BLACK : DB8_WHITE;
        DrawLineEx((Vector2){srcRectScreen.x, srcRectScreen.y}, (Vector2){data->dstRect.x, data->dstRect.y}, lw, color);
        DrawLineEx((Vector2){srcRectScreen.x + srcRectScreen.width, srcRectScreen.y}, (Vector2){data->dstRect.x + data->dstRect.width, data->dstRect.y}, lw, color);
        DrawLineEx((Vector2){srcRectScreen.x, srcRectScreen.y + srcRectScreen.height}, (Vector2){data->dstRect.x, data->dstRect.y + data->dstRect.height}, lw, color);
        DrawLineEx((Vector2){srcRectScreen.x + srcRectScreen.width, srcRectScreen.y + srcRectScreen.height}, (Vector2){data->dstRect.x + data->dstRect.width, data->dstRect.y + data->dstRect.height}, lw, color);
    }

    rlDrawRenderBatchActive();
    rlDisableColorBlend();
    BeginShaderMode(data->shader);
    DrawTexturePro(texture, srcRect, data->dstRect, (Vector2){0, 0}, 0.0f, DB8_WHITE);
    EndShaderMode();

    rlDrawRenderBatchActive();
    rlEnableColorBlend();


    DrawRectangleLinesEx((Rectangle){data->dstRect.x-1.0f, data->dstRect.y-1.0f, data->dstRect.width + 2.0f, data->dstRect.height + 2.0f}, 4.0f, DB8_WHITE);
    DrawRectangleLinesEx(data->dstRect, 2.0f, BLACK);
}

typedef struct ScriptAction_JumpStepData {
    int prevStep;
    int nextStep;
    int isRelative;
} ScriptAction_JumpStepData;

void* ScriptAction_JumpStepData_new(int prevStep, int nextStep, int isRelative)
{
    ScriptAction_JumpStepData *data = Scene_alloc(sizeof(ScriptAction_JumpStepData),
        &(ScriptAction_JumpStepData){
            .prevStep = prevStep,
            .nextStep = nextStep,
            .isRelative = isRelative,
        });
    return data;
}

void ScriptAction_jumpStep(Script *script, ScriptAction *action)
{
    ScriptAction_JumpStepData *data = action->actionData;
    int h = GetScreenHeight();
    int w = GetScreenWidth();
    int x = w - 64, y = h - 64, sx = 40, sy = 40;
    int mx = GetMouseX();
    int my = GetMouseY();

    DrawRectangle(x, y, sx, sy, DB8_WHITE);
    DrawRectangleLinesEx((Rectangle){x,y,sx,sy},2.0f, BLACK);
    DrawTextEx(_fntMedium, ">", (Vector2){x+sx / 2 - 4,y+6}, _fntMedium.baseSize * 2.0f, -2.0f, DB8_WHITE);
    if ((IsMouseButtonReleased(0) && mx >= x && my >= y && mx < x + sx && my < y + sy) || IsKeyReleased(KEY_RIGHT))
    {
        TraceLog(LOG_INFO, "nextStep: %d", data->nextStep);
        if (data->isRelative)
        {
            script->nextActionId = script->currentActionId + data->nextStep;
        }
        else
        {
            script->nextActionId = data->nextStep;
        }
    }

    x-=sx + 4;

    DrawRectangle(x, y, sx, sy, DB8_WHITE);
    DrawRectangleLinesEx((Rectangle){x,y,sx,sy},2.0f, BLACK);
    DrawTextEx(_fntMedium, "<", (Vector2){x+sx / 2 - 4,y+6}, _fntMedium.baseSize * 2.0f, -2.0f, DB8_WHITE);
    if ((IsMouseButtonReleased(0) && mx >= x && my >= y && mx < x + sx && my < y + sy) || IsKeyReleased(KEY_LEFT))
    {
        TraceLog(LOG_INFO, "prevStep: %d", data->prevStep);
        if (data->isRelative)
        {
            script->nextActionId = script->currentActionId + data->prevStep;
        }
        else
        {
            script->nextActionId = data->prevStep;
        }
    }
}

typedef struct ScriptAction_DrawMeshData {
    Mesh *mesh;
    Shader shader;
    Camera3D *camera;
    Material *material;
    Matrix transform;
} ScriptAction_DrawMeshData;

void* ScriptAction_DrawMeshData_new(Mesh *mesh, Shader shader, Material *material, Matrix transform, Camera3D *camera)
{
    ScriptAction_DrawMeshData *data = Scene_alloc(sizeof(ScriptAction_DrawMeshData),
        &(ScriptAction_DrawMeshData){
            .mesh = mesh,
            .camera = camera,
            .shader = shader,
            .material = material,
            .transform = transform,
        });
    return data;
}

void ScriptAction_drawMesh(Script *script, ScriptAction *action)
{
    ScriptAction_DrawMeshData *data = action->actionData;
    Mesh mesh = *data->mesh;
    Shader shader = data->shader;
    Material material = *data->material;
    Matrix transform = data->transform;
    Camera3D camera = *data->camera;
    BeginMode3D(camera);
    BeginShaderMode(shader);
    DrawMesh(mesh, material, transform);
    EndShaderMode();
    EndMode3D();
}

typedef struct ScriptAction_DrawTextureData {
    Texture2D *texture;
    Rectangle dstRect;
    Rectangle srcRect;
} ScriptAction_DrawTextureData;

void* ScriptAction_DrawTextureData_new(Texture2D *texture, Rectangle dstRect, Rectangle srcRect)
{
    ScriptAction_DrawTextureData *data = Scene_alloc(sizeof(ScriptAction_DrawTextureData),
        &(ScriptAction_DrawTextureData){
            .texture = texture,
            .dstRect = dstRect,
            .srcRect = srcRect,
        });
    return data;
}

void ScriptAction_drawTexture(Script *script, ScriptAction *action)
{
    ScriptAction_DrawTextureData *data = action->actionData;
    DrawTexturePro(*data->texture, data->srcRect, data->dstRect, (Vector2){0, 0}, 0.0f, DB8_WHITE);
}

void ScriptAction_progressNextOnTriggeredOn(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    if (Level_isTriggeredOn(level, (char*) action->actionData))
    {
        script->nextActionId = script->currentActionId + 1;
    }
}

typedef struct LookCameraAtData {
    Vector3 position;
    FPSCameraZ *camera;
    float transitionTime;

    float actionStartTime;
    float targetYaw;
    float targetPitch;
    float startYaw;
    float startPitch;
} LookCameraAtData;


void ScriptAction_lookCameraAt(Script *script, ScriptAction *action)
{
    LookCameraAtData *data = action->actionData;
    FPSCameraZ *camera = data->camera;
    Level *level = Game_getLevel();
    if (data->actionStartTime == 0.0f)
    {
        data->actionStartTime = level->gameTime;
        data->startYaw = camera->rotation.y;
        data->startPitch = camera->rotation.x;
        Vector3 dir = Vector3Subtract(data->position, camera->camera.position);
        data->targetYaw = atan2f(dir.x, dir.z);
        data->targetPitch = atan2f(-dir.y, sqrtf(dir.x * dir.x + dir.z * dir.z));
    }
    float t = (level->gameTime - data->actionStartTime) / data->transitionTime;
    t = EaseInOutSine(t, 0.0f, 1.0f);

    camera->rotation.y = LerpAngle(data->startYaw, data->targetYaw, t);
    camera->rotation.x = LerpAngle(data->startPitch, data->targetPitch, t);

    // printf("dy: %.2f\n", data->position.y - camera->camera.position.y);

    // printf("t: %.2f %.2f %.2f; %.2f %.2f %.2f -> %.2f %.2f %.2f\n", t, camera->rotation.y, camera->rotation.x,
    //     data->camera->camera.position.x, data->camera->camera.position.y, data->camera->camera.position.z,
    //     data->position.x, data->position.y, data->position.z);
    // Vector3 dir = Vector3Subtract(position, camera->camera.position);
    // float angle = atan2f(dir.z, dir.x);
    // camera->rotation.y = angle;
}

void* ScriptAction_LookCameraAtData_new(FPSCameraZ *camera, float transitionTime, Vector3 position)
{
    LookCameraAtData *data = Scene_alloc(sizeof(LookCameraAtData),
        &(LookCameraAtData){
            .camera = camera,
            .position = position,
            .transitionTime = transitionTime,
        });
    return data;
}

void DrawNarrationBottomBox(const char *narrator, const char *text, const char *proceedText)
{
    Rectangle rect = {30, GetScreenHeight() - 120, GetScreenWidth() - 60, 78};

    DrawRectangleRec(rect, DB8_WHITE);
    DrawRectangleLinesEx(rect, 2, DB8_BLACK);

    DrawTextBoxAligned(_fntMedium, text,
        rect.x + 12, rect.y + 10, rect.width - 24, rect.height - 20, 0.5f, 0.5f, DB8_WHITE);

    Rectangle narratorBox = {rect.x + 10, rect.y - 24, 160, 32};
    Rectangle shadowBox = {narratorBox.x + 2, narratorBox.y + 2, narratorBox.width, narratorBox.height};
    DrawRectangleRec(shadowBox, (Color){0, 0, 0, 180});
    DrawRectangleRec(narratorBox, DB8_BLUE);
    DrawRectangleLinesEx(narratorBox, 2, DB8_BLACK);
    DrawTextBoxAligned(_fntMedium, narrator, narratorBox.x + 6, narratorBox.y + 6, narratorBox.width - 12, narratorBox.height - 12, 0.0f, 0.5f, DB8_WHITE);

    if (!proceedText) return;
    Rectangle proceedBox = {rect.x + rect.width - 270, rect.y + rect.height - 10, 280, 36};
    shadowBox = (Rectangle){proceedBox.x + 2, proceedBox.y + 2, proceedBox.width, proceedBox.height};
    DrawRectangleRec(shadowBox, (Color){0, 0, 0, 180});
    DrawRectangleRec(proceedBox, DB8_YELLOW);
    DrawRectangleLinesEx(proceedBox, 2, DB8_BLACK);
    DrawTextBoxAligned(_fntMedium, proceedText, proceedBox.x + 6, proceedBox.y + 6, proceedBox.width - 12, proceedBox.height - 12, 0.5f, 0.5f, DB8_WHITE);
}

typedef struct ScriptAction_DrawNarrationBottomBoxData {
    const char *narrator;
    const char *text;
    int proceedOnEnter;
} ScriptAction_DrawNarrationBottomBoxData;

void ScriptAction_drawNarrationBottomBox(Script *script, ScriptAction *action)
{
    extern Sound talkSfxMale1;
    extern Sound talkSfxMale2;
    extern Sound talkSfxMale3;
    extern Sound talkSfxFemale1;
    extern Sound talkSfxFemale2;
    extern Sound talkSfxFemale3;

    ScriptAction_DrawNarrationBottomBoxData *data = action->actionData;

    static bool has_played_sound = false;
    if (!has_played_sound)
    {
        // hacky way to determine if we should play male or female talking sfx
        if (strstr(data->narrator, "Cecilia") == NULL)
        {
            // the narrator is male
            switch(GetRandomValue(0, 2))
            {
                case 0: PlaySound(talkSfxMale1);
                    break;
                case 1: PlaySound(talkSfxMale2);
                    break;
                case 2: PlaySound(talkSfxMale3);
                    break;
            }
        }
        else
        {
            // the narrator is female (Cecilia)
            switch(GetRandomValue(0, 2))
            {
                case 0: PlaySound(talkSfxFemale1);
                    break;
                case 1: PlaySound(talkSfxFemale2);
                    break;
                case 2: PlaySound(talkSfxFemale3);
                    break;
            }
        }
        has_played_sound = true;
    }

    DrawNarrationBottomBox(data->narrator, data->text, data->proceedOnEnter ? "Press [color=red_]ENTER[/color] to continue" : NULL);
    if (data->proceedOnEnter && IsKeyPressed(KEY_ENTER))
    {
        script->nextActionId = script->currentActionId + 1;
        has_played_sound = false;
    }
}

void* ScriptAction_DrawNarrationBottomBoxData_new(const char *narrator, const char *text, int proceedOnEnter)
{
    ScriptAction_DrawNarrationBottomBoxData *data = Scene_alloc(sizeof(ScriptAction_DrawNarrationBottomBoxData),
        &(ScriptAction_DrawNarrationBottomBoxData){
            .narrator = narrator,
            .text = text,
            .proceedOnEnter = proceedOnEnter,
        });
    return data;
}

void ScriptAction_loadScene(Script *script, ScriptAction *action)
{
    Game_setNextScene(action->actionInt);
}

typedef struct ScriptAction_FadingCutData {
    float transitionTime;
    float actionStartTime;
    float nextStepDelay;
    Color color;
    uint8_t fadeType;
    uint8_t fadeTweenType;
    float animationDirection;
} ScriptAction_FadingCutData;

void ScriptAction_fadingCut(Script *script, ScriptAction *action)
{
    ScriptAction_FadingCutData *data = action->actionData;
    Level *level = Game_getLevel();
    if (data->actionStartTime == 0.0f)
    {
        data->actionStartTime = level->gameTime;
    }
    float t = (level->gameTime - data->actionStartTime) / data->transitionTime;
    if (data->animationDirection < 0.0f)
    {
        t = 1.0f - t;
    }
    if (data->fadeTweenType == FADE_TWEEN_TYPE_SIN)
    {
        t = EaseInOutSine(t, 0.0f, 1.0f);
    }
    if (data->fadeType == FADE_TYPE_VERTICAL_CLOSE)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight() * t * .5f, data->color);
        DrawRectangle(0, GetScreenHeight() * (1.0f - t * .5f), GetScreenWidth(), GetScreenHeight(), data->color);
    }
    else if (data->fadeType == FADE_TYPE_TOP_DOWN_CLOSE)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight() * t, data->color);
    }
    else if (data->fadeType == FADE_TYPE_BOTTOM_UP_CLOSE)
    {
        DrawRectangle(0, GetScreenHeight() * (1.0f - t), GetScreenWidth(), GetScreenHeight(), data->color);
    }
    else
    {
        DrawRectangle(0, 0, GetScreenWidth() * t * .5f, GetScreenHeight(), data->color);
        DrawRectangle(GetScreenWidth() * (1.0f - t) * .5f, 0, GetScreenWidth() * t, GetScreenHeight(), data->color);
    }
    if (level->gameTime - data->actionStartTime - data->transitionTime > data->nextStepDelay &&
        action->actionIdStart == script->currentActionId)
    {
        script->nextActionId = script->currentActionId + 1;
    }
}

void* ScriptAction_FadingCutData_new(float transitionTime, Color color, uint8_t fadeType, uint8_t fadeTweenType, float nextStepDelay, float direction)
{
    ScriptAction_FadingCutData *data = Scene_alloc(sizeof(ScriptAction_FadingCutData),
        &(ScriptAction_FadingCutData){
            .transitionTime = transitionTime,
            .color = color,
            .fadeType = fadeType,
            .fadeTweenType = fadeTweenType,
            .nextStepDelay = nextStepDelay,
            .animationDirection = direction,
        });
    return data;
}