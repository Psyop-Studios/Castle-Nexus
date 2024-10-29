#ifndef __GAME_MAIN_H__
#define __GAME_MAIN_H__

#include "raylib.h"
#include "rlgl.h"
#include "util.h"
#include <inttypes.h>

#define DB8_WHITE (Color){220, 245, 255, 255}
#define DB8_DEEPPURPLE (Color){85, 65, 95, 255}
#define DB8_GREY (Color){100, 105, 100, 255}
#define DB8_RED (Color){215, 115, 85, 255}
#define DB8_BLUE (Color){80, 140, 215, 255}
#define DB8_GREEN (Color){100, 185, 100, 255}
#define DB8_YELLOW (Color){230, 200, 110, 255}
#define DB8_BLACK (Color){0, 0, 0, 255}

typedef struct Script Script;
typedef struct ScriptAction ScriptAction;

typedef struct ScriptAction {
    int actionIdStart, actionIdEnd;
    void (*action)(Script *script, ScriptAction *action);
    union {
        int actionInt;
        void *actionData;
    };
} ScriptAction;

#define SCRIPT_MAX_ACTIONS 128

typedef struct Script {
    int actionCount;
    ScriptAction actions[SCRIPT_MAX_ACTIONS];
    int currentActionId;
    int nextActionId;
} Script;


// The game context is our most fundamental structure. It is keeps track of the player's 
// progress in the game and which scene is currently active. It is kept alive throughout the
// game's lifecycle and potentially stored in a save file. Inventory and quest state variables
// should go here. But it also keeps track of the current game time and which scene should be
// loaded next.
typedef struct GameContext {
    // currently active scene - modified by main game loop. Don't write to this directly.
    uint32_t currentSceneId;
    // at the end of the game loop, this will be copied to currentSceneId and the new scene will be initialized
    uint32_t nextSceneId;
    // the current game time in seconds
    float currentGameTime;
} GameContext;

extern Script _script;
extern Font _fntMono;
extern Font _fntMedium;
extern Shader _modelDitherShader;
extern Shader _modelTexturedShader;

#endif