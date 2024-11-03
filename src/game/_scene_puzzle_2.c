#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <stdio.h>


static FPSCameraZ _camera;
static int _allowCameraMovement = 1;
#define TRIGGER_BOXTARGET_LEVEL_2 "BoxTarget"
#define TRIGGER_MEMORY_2 "Memory2"

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    BeginMode3D(_camera.camera);
    _currentCamera = _camera.camera;
    Level *level = Game_getLevel();
    Level_draw(level);
    EndMode3D();
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{
    dt = fminf(dt, 0.1f);
    Level *level = Game_getLevel();
    level->isEditor = 0;

    Level_update(level, dt);
    FPSCamera_update(&_camera, level, _allowCameraMovement, dt);
}
static void ScriptAction_setCameraMovementEnabled(Script *script, ScriptAction *action)
{
    _allowCameraMovement = action->actionInt;
}

typedef struct BoxInPlaceData
{
    float timeInPlace;
} BoxInPlaceData;

void ScriptAction_onBoxInPlaceLevel2(Script *script, ScriptAction *action)
{
    extern Sound triggerSfx;
    extern Sound gateOpeningSfx;
    Level *level = Game_getLevel();
    if (Level_isTriggeredOn(level, TRIGGER_BOXTARGET_LEVEL_2))
    {
        PlaySound(triggerSfx);
        PlaySound(gateOpeningSfx);
    }
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    Game_setFogGradient(174.0/512.0, 104.0/512.0, 0.55f, 0.26f);
    DisableCursor();
    // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    _camera.camera = (Camera){0};
    _camera.camera.position = (Vector3){ 1.5f, 1.70f, 0.0f };
    _camera.camera.target = (Vector3){ 90.0f, 100.0f, 180.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 50.0f;

    Level_load(Game_getLevel(), "resources/levels/Level2.lvl");
    int step = 0;




    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, -1.0f)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 0});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "Hmm. Seems I've gotten a bit mixed up, and we're in another locked area.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
            "Oh, I know, you could try finding a way to open the door again!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=red_] August[/color]:",
          "Who are you talking to, [color=blue] Cecilia?[/color]", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
        "No one!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=red_] August[/color]:",
          "Don't tell me that man escaped the trap dungeon! \n"
          "...It's really hard to set up once it's been used.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=red_] August[/color]:",
          "I don't have any hands!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("You:",
        "I've gotta get out of here quick.\n"
        "My boss is never gonna believe this scoop!", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 1});

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_onBoxInPlaceLevel2, .actionData = Scene_alloc(sizeof(BoxInPlaceData), &(BoxInPlaceData){.timeInPlace = 0.0f}) });



    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_progressNextOnTriggeredOn, .actionData = (char*)TRIGGER_MEMORY_2 });
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 0});
        Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_lookCameraAt,
        .actionData = ScriptAction_LookCameraAtData_new(
            &_camera, 2.5f, (Vector3){2.0f, 1.4f, 20.0})});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
        "I hate to interrupt, \n"
        "But I think it's important for you to know something.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
        "August was a kind man when he was alive. This island was built just\n"
        "so we could send word to shore if there are signs of an impending invasion.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]",
        "This is how our children survived -- they were quick enough to get on the\n"
        "Coastal lookout's boat. The Coastal lookout lived here with their families.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("You:",
        "I've gotta write this down..", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
        "I think I can connect with him again and stop his anguish.\n"
        "He was a benevolent man in life, and could become a benevolent ghost in the afterlife.", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("[color=blue] Cecilia[/color]:",
        "I'm pretty sure the entrance is this way --\n"
        "Follow me!", 1)});
    step += 1;


    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_VERTICAL_CLOSE, FADE_TWEEN_TYPE_SIN, 1.0f, 1.0f)});

    step += 1;

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_loadScene, .actionInt = SCENE_ID_PUZZLE_3 });



}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    EnableCursor();
}


SceneConfig _scene_puzzle_2 = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_PUZZLE_2,
};