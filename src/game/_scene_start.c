#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <math.h>

static Camera _camera;
#define START_SCREEN 1
#define FINISH_SCREEN 2

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
    int isStartScreen = action->actionInt == START_SCREEN;

    Level *level = Game_getLevel();
    LevelTexture* tex = Level_getLevelTexture(level, "title-part-castle.png");
    LevelTexture* texNexus = Level_getLevelTexture(level, "title-part-nexus.png");
    LevelTexture* waterGradient = Level_getLevelTexture(level, "water-gradient.png");
    LevelTexture* skyGradient = Level_getLevelTexture(level, "sky-gradient.png");
    LevelTexture* horizont = Level_getLevelTexture(level, "horizont.png");
    LevelTexture* introCastle = Level_getLevelTexture(level, "intro-castle.png");
    LevelTexture* miniGhost = Level_getLevelTexture(level, "mini-ghost-sprite.png");
    LevelTexture* rowboat = Level_getLevelTexture(level, "rowboat-sprite.png");
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float waterHeight = screenHeight * .6f;
    if (tex && texNexus && waterGradient && skyGradient && horizont && introCastle && miniGhost && rowboat)
    {
        int width = tex->texture.width * 2;
        int height = tex->texture.height * 2;
        
        int sx = (screenWidth - width) / 5.0f;
        if (!isStartScreen)
        {
            sx = -40;
        }

        // sky
        for (int x = 0; x < screenWidth; x += waterGradient->texture.width * 2.0f)
        {
            DrawTextureEx(skyGradient->texture, (Vector2){x, 0}, 0, 2.0f, WHITE);
        }
        // horizont (mountains)
        for (int x = 0; x < screenWidth; x += horizont->texture.width * 2.0f)
        {
            DrawTextureEx(horizont->texture, (Vector2){x, waterHeight - horizont->texture.height * 2.0f + 64}, 0, 2.0f, WHITE);
        }
        // credits
        if (!isStartScreen)
        {
            const char *credits[] = {
                "Thank you for playing",
                "",
                "[color=red_]Castle[/color] [color=blue]Nexus[/color]",
                "",
                "A game by",
                "",
                "Cypress",
                "Haikuno",
                "Chao-Etta",
                "Zet",
                "",
                "for the raylib NEXT game jam 2024",
                "",
                "",
                "We hope you enjoyed it!",
                "",
                "The End.",
                0
            };

            float maxHeight = 0;
            for (int i = 0; credits[i]; i++)
            {
                maxHeight += _fntMedium.baseSize * 2.0f + 2.0f;
            }

            for (int i = 0; credits[i]; i++)
            {
                if (TextLength(credits[i]) <= 1)
                {
                    continue;
                }
                float offsetY = level->gameTime * 30.0f;
                offsetY = fmodf(offsetY, maxHeight * 2.0f);
                float txY = i * _fntMedium.baseSize * 2.0f + 2.0f + waterHeight + 80 - offsetY;
                float txX = sx + texNexus->texture.width * 2 + 10;
                Vector2 sz = DrawTextRich(_fntMedium, credits[i], (Vector2){txX, txY},
                    _fntMedium.baseSize * 2.0f, 0.0f, 1000, WHITE, 1);
                DrawTextRich(_fntMedium, credits[i], (Vector2){txX + 100.0f - sz.x * .5f, txY}, _fntMedium.baseSize * 2.0f, 0.0f, 1000, WHITE, 0);
            }
        }
        // water
        for (int x = 0; x < screenWidth; x += waterGradient->texture.width * 2.0f)
        {
            DrawRectangle(x, waterHeight + waterGradient->texture.height, waterGradient->texture.width * 2.0f, screenHeight - waterHeight, DB8_BLUE);
            DrawTextureEx(waterGradient->texture, (Vector2){x, waterHeight}, 0, 2.0f, WHITE);
        }

        float castleX = (screenWidth - introCastle->texture.width * 2) * 4 / 5;
        float castleY = waterHeight - introCastle->texture.height + 32;

        if (!isStartScreen) {
            float boatStartX = castleX + 120;
            float boatY = waterHeight + 80;
            float boatSpeed = 10.0f;
            int boatWidth = 8;
            int boatHeight = 8;
            int frameSelect = (int)(level->gameTime * 3) % 5;
            float boatX = boatStartX - ((int)(level->gameTime * boatSpeed) & ~1);
            DrawTexturePro(rowboat->texture, (Rectangle){frameSelect * 8, 0, 8, 8}, (Rectangle){boatX, boatY, 16, 16}, (Vector2){8, 8}, 0, WHITE);

            if (level->gameTime > 30.0f)
            {
                float ghostX = boatStartX - ((int)((level->gameTime - 30.0f) * boatSpeed * 1.5f) & ~1);
                float ghostY = boatY + sinf(level->gameTime * 4.15f) * 5.0f - 18;

                int frameSelect = (int)(level->gameTime * 4) % 4;
                DrawTexturePro(miniGhost->texture, (Rectangle){frameSelect * 8, 0, -8, 8}, (Rectangle){ghostX, ghostY, 16, 16}, (Vector2){8, 8}, 0, WHITE);
            }
        }

        DrawTextureEx(introCastle->texture, (Vector2){castleX, castleY}, 0, 2.0f, WHITE);

        if (isStartScreen)
        {
            float ghostX = castleX + introCastle->texture.width + 20 + cosf(level->gameTime * 0.25f) * 40.0f;
            float xdir = sinf(level->gameTime * 0.25f);
            float ghostY = castleY + introCastle->texture.height + 20 + sinf(level->gameTime * 0.15f) * 10.0f;

            int frameSelect = (int)(level->gameTime * 4) % 4;
            DrawTexturePro(miniGhost->texture, (Rectangle){frameSelect * 8, 0, xdir < 0.0f ? 8 : -8, 8}, (Rectangle){ghostX, ghostY, 16, 16}, (Vector2){8, 8}, 0, WHITE);
        }
        
        
        int sy = (screenHeight - height) / 2 - 50;
        DrawTextureEx(tex->texture, (Vector2){sx, sy}, 0, 2.0f, WHITE);

        int nsx = sx + sinf(level->gameTime * 0.20f) * 10.0f;
        int nsy = sy + cosf(level->gameTime * 0.85f) * 20.0f;
        DrawTextureEx(texNexus->texture, (Vector2){nsx, nsy}, 0, 2.0f, WHITE);

        // DrawTextureEx(waterGradient->texture, (Vector2){0, waterHeight - waterGradient->texture.height}, 0, 2.0f, WHITE);
    // DrawRectangle(0, waterHeight, screenWidth, screenHeight - waterHeight, DB8_BLUE);
        
    }


    
    if (script->currentActionId == 0)
    {
        const char *enterToContinue = "Press [color=red_]ENTER[/color] to start your journey...";
        if (!isStartScreen)
        {
            enterToContinue = "Press [color=red_]ENTER[/color] to restart the game.";
        }
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

    
    if (gameCtx->currentSceneId == SCENE_ID_START)
    {
        Script_addAction((ScriptAction){
            .actionIdStart = step-1,
            .actionIdEnd = step + 10,
            .action = StepDrawTitle,
            .actionInt = START_SCREEN,
        });
        Script_addAction((ScriptAction){
            .actionIdStart = step,
            .actionIdEnd = step + 2,
            .action = ScriptAction_fadingCut,
            .actionData = ScriptAction_FadingCutData_new(2.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 100000000.0f, -1.0f)});
        
        step += 1;
        Script_addAction((ScriptAction){
            .actionIdStart = step,
            .actionIdEnd = step + 2,
            .action = ScriptAction_fadingCut,
            .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, 1.0f)});
        step += 1;
        Script_addAction((ScriptAction){
            .actionIdStart = step,
            .action = ScriptAction_loadScene,
            .actionInt = SCENE_ID_START_INTRO});
    }
    else
    {
        Script_addAction((ScriptAction){
            .actionIdStart = step,
            .actionIdEnd = step + 10,
            .action = StepDrawTitle,
            .actionInt = FINISH_SCREEN,
        });
        Script_addAction((ScriptAction){
            .actionIdStart = step,
            .actionIdEnd = step + 2,
            .action = ScriptAction_fadingCut,
            .actionData = ScriptAction_FadingCutData_new(2.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 100000000.0f, -1.0f)});


        step += 1;
        Script_addAction((ScriptAction){
            .actionIdStart = step,
            .actionIdEnd = step + 2,
            .action = ScriptAction_fadingCut,
            .actionData = ScriptAction_FadingCutData_new(3.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, 1.0f)});
        step += 1;
        Script_addAction((ScriptAction){
            .actionIdStart = step,
            .action = ScriptAction_loadScene,
            .actionInt = SCENE_ID_START});
    }


        
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
SceneConfig _scene_finish = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_FINISH,
};