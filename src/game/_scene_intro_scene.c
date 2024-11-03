#include "scene.h"
#include "_scenes.h"
#include "scriptsystem.h"
#include "scriptactions.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <stdio.h>


static FPSCameraZ _camera;
static int _allowCameraMovement = 1;

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

#define TRIGGER_EXIT_ZONE "TriggerExitZone"
#define TRIGGER_EXPLORATION_MESSAGE_ZONE_1 "TriggerExplorationMessageZone_1"
#define TRIGGER_FISHERMEN_ZONE "TriggerFishermenZone"
#define TRIGGER_BOXTARGET "BoxTarget"

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        DisableCursor();
    }

    dt = fminf(dt, 0.1f);
    Level *level = Game_getLevel();
    level->isEditor = 0;
    
    Level_update(level, dt);
    FPSCamera_update(&_camera, level, _allowCameraMovement, dt);
}

void ScriptAction_setCameraMovementEnabled(Script *script, ScriptAction *action)
{
    _allowCameraMovement = action->actionInt;
}



static void ScriptAction_exitZoneMessage(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    if (Level_isTriggerActive(level, TRIGGER_EXIT_ZONE))
    {
        DrawNarrationBottomBox("You:", 
            "My boss will fire me, if I don't get him the story about the haunted castle...", "Talk to the fishermen.");
    }
}

static void ScriptAction_explorationMessageZone_1(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    
    if (Level_isTriggerActive(level, TRIGGER_EXPLORATION_MESSAGE_ZONE_1))
    {
        DrawNarrationBottomBox("You:", "This place is giving me a chill... as if I am watched.", NULL);
    }
}

static void ScriptAction_intro_firstStep(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    if (level->playerDistanceWalked > 1.0f)
    {
        script->nextActionId = action->actionIdStart + 1;
        return;
    }

    DrawNarrationBottomBox("You:", 
        "The people said, if I wanted to visit the castle, I should ask the [color=red_]Fisher men[/color].\n"
        "(Use [color=red_]WASD/SPACE[/color] to move & jump, and the [color=red_]mouse[/color] to look around.)", NULL);
}

typedef struct BoxInPlaceData
{
    float timeInPlace;
} BoxInPlaceData;


void ScriptAction_onBoxInPlace(Script *script, ScriptAction *action)
{
    Level *level = Game_getLevel();
    BoxInPlaceData *data = (BoxInPlaceData*)action->actionData;
    if (Level_isTriggeredOn(level, TRIGGER_BOXTARGET))
    {
        if (data->timeInPlace <= 0.0f)
        {
            data->timeInPlace = level->gameTime;
        }
    }
    if (data->timeInPlace > 0.0f && level->gameTime - data->timeInPlace < 4.0f)
    {
        DrawNarrationBottomBox("You:", "The box is in place", NULL);
    }
}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    Game_setFogGradient(174.0/512.0, 200.0/512.0, 0.125f, 0.4f);
    DisableCursor();
    // SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    _camera.camera = (Camera){0};
    _camera.camera.position = (Vector3){ 10.0f, 1.70f, 10.0f };
    _camera.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.camera.fovy = 45.0f;
    _camera.camera.projection = CAMERA_PERSPECTIVE;
    _camera.rotation.y = 200.0f * DEG2RAD;
    _camera.velocityDecayRate = 14.0f;
    _camera.acceleration = 50.0f;

    _allowCameraMovement = 0;

    Level_load(Game_getLevel(), "resources/levels/docks.lvl");
    int step = 0;
    
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .actionIdEnd = step + 2,
        .action = ScriptAction_fadingCut,
        .actionData = ScriptAction_FadingCutData_new(1.0f, DB8_BLACK, FADE_TYPE_TOP_DOWN, FADE_TWEEN_TYPE_SIN, 1.0f, -1.0f)});
    step += 1;
    // message to player to get started
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 1});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_intro_firstStep });
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_exitZoneMessage });
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_explorationMessageZone_1 });
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_onBoxInPlace, .actionData = Scene_alloc(sizeof(BoxInPlaceData), &(BoxInPlaceData){.timeInPlace = 0.0f}) });
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_progressNextOnTriggeredOn, .actionData = (char*)TRIGGER_FISHERMEN_ZONE });
    step += 1;
    
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_setCameraMovementEnabled, .actionInt = 0});
    Script_addAction((ScriptAction){
        .actionIdStart = step,
        .action = ScriptAction_lookCameraAt,
        .actionData = ScriptAction_LookCameraAtData_new(
            &_camera, 2.5f, (Vector3){2.0f, 1.5f, 0})});
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("Fisherman 1:",
            "Oi, what are ye sneaking around like a ghost for?", 1)});
    step += 1;
    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_drawNarrationBottomBox,
        .actionData = ScriptAction_DrawNarrationBottomBoxData_new("You:",
            "Eh, funny you saying that... I was looking for someone to ferry me to the castle.", 1)});
    step += 1;

    Script_addAction((ScriptAction){ .actionIdStart = step, .action = ScriptAction_loadScene, .actionInt = SCENE_ID_START_ISLAND });

    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "You are an investigative journalist tasked", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Chapter I",  "with chasing some [color=grey]rumors[/color].", (Rectangle){10, 10, 200, 100})});

    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "You wanna write some stories, huh?", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "I've got a scoop for ya.", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "I happen to know a local island is haunted by ghosts.", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 2",  "Don't bother listening to him.", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 2",  "He just likes telling stories. ", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "Nonesense, I've seen em myself!", (Rectangle){10, 10, 200, 100})});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_drawTextRect,
    //     .actionData = ScriptAction_DrawTextRectData_new("Dock Worker 1",  "As a matter of fact, I'll take you for free.", (Rectangle){10, 10, 200, 100})});
        
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_setCameraMovementEnabled,
    //     .actionInt = 1});
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_jumpStep,
    //     .actionData = ScriptAction_JumpStepData_new(-1, 1, 1)});
    // step++;
    
    // Script_addAction((ScriptAction){
    //     .actionIdStart = step,
    //     .action = ScriptAction_setCameraMovementEnabled,
    //     .actionInt = 1});
}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
}

SceneConfig _scene_intro_scene = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .sceneId = SCENE_ID_START_INTRO,
};