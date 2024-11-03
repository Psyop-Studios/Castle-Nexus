#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <math.h>

static Camera _camera;

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    ClearBackground(DB8_BG_GREY);
    BeginMode3D(_camera);
    _currentCamera = _camera;
    EndMode3D();
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{
    Level_update(Game_getLevel(), dt);
}

static void Step1_UI(Script *script, ScriptAction *action)
{
    // DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams){
    //     .bounds = {10, 300, 200, 50},
    // });
    // DuskGui_label((DuskGuiParams){
    //     .text = "Just a small UI example showing clipping support",
    //     .bounds = {10, 10, 200, 18},
    // });
    // if (DuskGui_button((DuskGuiParams){
    //     .bounds = {10, 30, 300, 18},
    //     .rayCastTarget = 1,
    //     .text = "Click me!",
    // }))
    // {
    //     TraceLog(LOG_INFO, "Button clicked!");
    // }
    // DuskGui_endPanel(panel);
}

static void StepDrawTitle(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    LevelTexture* tex = Level_getLevelTexture(level, "title-part-castle.png");
    LevelTexture* texNexus = Level_getLevelTexture(level, "title-part-nexus.png");
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    if (tex)
    {
        int width = tex->texture.width * 2;
        int height = tex->texture.height * 2;
        int x = (screenWidth - width) / 2;
        int y = (screenHeight - height) / 2 - 50;
        DrawTextureEx(tex->texture, (Vector2){x, y}, 0, 2.0f, WHITE);

        x = x + sinf(level->gameTime * 0.20f) * 10.0f;
        y = y + cosf(level->gameTime * 0.85f) * 20.0f;
        DrawTextureEx(texNexus->texture, (Vector2){x, y}, 0, 2.0f, WHITE);
    }

    float waterHeight = screenHeight * .8f;
    DrawRectangle(0, waterHeight, screenWidth, screenHeight - waterHeight, DB8_BLUE);

    if (script->currentActionId == 0)
    {
        const char *enterToContinue = "Press [color=red_]ENTER[/color] to start your journey...";
        Vector2 txSize = DrawTextRich(_fntMedium, enterToContinue, (Vector2){0, 0}, _fntMedium.baseSize * 2.0f, 0.0f, 1000, DB8_YELLOW, 1);
        DrawTextRich(_fntMedium, enterToContinue, (Vector2){(screenWidth - txSize.x) / 2, screenHeight - txSize.y - 10}, _fntMedium.baseSize * 2.0f, 0.0f, 1000, DB8_YELLOW, 0);

        if (IsKeyPressed(KEY_ENTER))
        {
            // Game_setNextScene(SCENE_ID_START_INTRO);
            script->nextActionId++;
        }
    }
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    if (gameCtx->currentSceneId == SCENE_ID_START)
    {
        TraceLog(LOG_INFO, "SceneInit: %d", SceneConfig->sceneId);
    }

    TraceLog(LOG_INFO, "SceneInit: %d", SceneConfig->sceneId);
    
    _camera = (Camera){0};
    _camera.position = (Vector3){ 10.0f, 1.70f, 10.0f };
    _camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.fovy = 45.0f;
    _camera.projection = CAMERA_PERSPECTIVE;

    TraceLog(LOG_INFO, "SceneInit: %d done", SceneConfig->sceneId);


    int step = 0;
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 10,
        .action = StepDrawTitle,
    });
    step += 1;
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_TOP_DOWN, FADE_TWEEN_TYPE_SIN, 1.0f, 1.0f)});
    step += 1;
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_loadScene,
        .actionInt = SCENE_ID_START_INTRO});

        
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "It was a [color=red_]dark and stormy night[/color] ...", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_jumpStep,
    //     .actionData = ScriptAction_JumpStepData_new(-1, 1, 1)});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = Step1_UI,
    // });
    // step++;

    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("", "the next step!", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_jumpStep,
    //     .actionData = ScriptAction_JumpStepData_new(-1, 1, 1)});
}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
}

SceneConfig _scene_start = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_START,
};