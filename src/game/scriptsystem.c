#include "scriptsystem.h"

Script _script;

void Script_init(GameContext *gameCtx)
{
    _script.actionCount = 0;
    _script.currentActionId = 0;
}

void Script_draw(GameContext *gameCtx)
{

}

void Script_update(GameContext *gameCtx, float dt)
{
    // _script.nextActionId = _script.currentActionId;
    // for (int i = 0; i < _script.actionCount; i++) 
    // {
    //     ScriptAction *action = &_script.actions[i];
    //     if (_script.currentActionId >= action->actionIdStart && _script.currentActionId <= action->actionIdEnd)
    //     {
    //         action->action(&_script, action);
    //     }
    // }
    // _script.currentActionId = _script.nextActionId;
    // _contextData->step = _script.currentActionId;
}

void Script_addAction(ScriptAction action)
{
    if (SCRIPT_MAX_ACTIONS == _script.actionCount)
    {
        TraceLog(LOG_ERROR, "Script_addAction: script action count exceeded");
        return;
    }

    // default init, if no end, assume length 1
    if (action.actionIdEnd < action.actionIdStart)
    {
        action.actionIdEnd = action.actionIdStart;
    }

    _script.actions[_script.actionCount++] = action;
}