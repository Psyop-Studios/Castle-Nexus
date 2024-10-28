#ifndef __GAME_SCRIPTACTIONS_H__
#define __GAME_SCRIPTACTIONS_H__

#include "main.h"

void* ScriptAction_DrawTextRectData_new(const char *title, const char *text, Rectangle rect);
void ScriptAction_drawTextRect(Script *script, ScriptAction *action);
void* ScriptAction_DrawMagnifiedTextureData_new(Rectangle srcRect, Rectangle dstRect, Texture2D *texture, Shader shader);
void ScriptAction_drawMagnifiedTexture(Script *script, ScriptAction *action);
void* ScriptAction_JumpStepData_new(int prevStep, int nextStep, int isRelative);
void ScriptAction_jumpStep(Script *script, ScriptAction *action);
void* ScriptAction_DrawMeshData_new(Mesh *mesh, Shader shader, Material *material, Matrix transform, Camera3D *camera);
void ScriptAction_drawMesh(Script *script, ScriptAction *action);
void* ScriptAction_SetDrawSceneData_new(void (*drawFn)(void*), void* drawData);
void ScriptAction_setDrawScene(Script *script, ScriptAction *action);
void* ScriptAction_DrawTextureData_new(Texture2D *texture, Rectangle dstRect, Rectangle srcRect);
void ScriptAction_drawTexture(Script *script, ScriptAction *action);

#endif