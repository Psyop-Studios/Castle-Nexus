#include "main.h"
#include "scene.h"


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
    DrawRectangleRec(data->rect, WHITE);
    DrawRectangleLinesEx(data->rect, 1, BLACK);
    DrawRectangleLinesEx((Rectangle){data->rect.x + 1, data->rect.y + 1, data->rect.width - 2, data->rect.height - 2}, 1, BLACK);
    DrawTextBoxAligned(_fntMedium, data->title, data->rect.x + 6, data->rect.y + 6, data->rect.width - 12, data->rect.height - 12, 0.5f, 0.0f, WHITE);
    DrawTextBoxAligned(_fntMedium, data->text, data->rect.x + 6, data->rect.y + 32, data->rect.width - 12, data->rect.height - 36, 0.0f, 0.0f, WHITE);
    // DrawTextEx(fntMedium, data->text, (Vector2){data->rect.x + 4, data->rect.y + 4}, fntMedium.baseSize * 2.0f, -2.0f, WHITE);
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
    DrawRectangleLinesEx((Rectangle){srcRectScreen.x - 1.0f, srcRectScreen.y - 1.0f, srcRectScreen.width + 2.0f, srcRectScreen.height + 2.0f}, 4.0f, WHITE);
    DrawRectangleLinesEx(srcRectScreen, 2.0f, BLACK);

    for (float lw = 4.0f; lw >= 2.0f; lw -= 2.0f)
    {
        Color color = lw == 2.0f ? BLACK : WHITE;
        DrawLineEx((Vector2){srcRectScreen.x, srcRectScreen.y}, (Vector2){data->dstRect.x, data->dstRect.y}, lw, color);
        DrawLineEx((Vector2){srcRectScreen.x + srcRectScreen.width, srcRectScreen.y}, (Vector2){data->dstRect.x + data->dstRect.width, data->dstRect.y}, lw, color);
        DrawLineEx((Vector2){srcRectScreen.x, srcRectScreen.y + srcRectScreen.height}, (Vector2){data->dstRect.x, data->dstRect.y + data->dstRect.height}, lw, color);
        DrawLineEx((Vector2){srcRectScreen.x + srcRectScreen.width, srcRectScreen.y + srcRectScreen.height}, (Vector2){data->dstRect.x + data->dstRect.width, data->dstRect.y + data->dstRect.height}, lw, color);
    }

    rlDrawRenderBatchActive();
    rlDisableColorBlend();
    BeginShaderMode(data->shader);
    DrawTexturePro(texture, srcRect, data->dstRect, (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();

    rlDrawRenderBatchActive();
    rlEnableColorBlend();


    DrawRectangleLinesEx((Rectangle){data->dstRect.x-1.0f, data->dstRect.y-1.0f, data->dstRect.width + 2.0f, data->dstRect.height + 2.0f}, 4.0f, WHITE);
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

    DrawRectangle(x, y, sx, sy, WHITE);
    DrawRectangleLinesEx((Rectangle){x,y,sx,sy},2.0f, BLACK);
    DrawTextEx(_fntMedium, ">", (Vector2){x+sx / 2 - 4,y+6}, _fntMedium.baseSize * 2.0f, -2.0f, WHITE);
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
    
    DrawRectangle(x, y, sx, sy, WHITE);
    DrawRectangleLinesEx((Rectangle){x,y,sx,sy},2.0f, BLACK);
    DrawTextEx(_fntMedium, "<", (Vector2){x+sx / 2 - 4,y+6}, _fntMedium.baseSize * 2.0f, -2.0f, WHITE);
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
    DrawTexturePro(*data->texture, data->srcRect, data->dstRect, (Vector2){0, 0}, 0.0f, WHITE);
}