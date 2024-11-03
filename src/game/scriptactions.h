#ifndef __GAME_SCRIPTACTIONS_H__
#define __GAME_SCRIPTACTIONS_H__

#include "main.h"

#define FADE_TYPE_TOP_DOWN 0
#define FADE_TWEEN_TYPE_LINEAR 0
#define FADE_TWEEN_TYPE_SIN 1


void DrawNarrationBottomBox(const char *narrator, const char *text, const char *proceedText);

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

void ScriptAction_progressNextOnTriggeredOn(Script *script, ScriptAction *action);
void ScriptAction_lookCameraAt(Script *script, ScriptAction *action);
void* ScriptAction_LookCameraAtData_new(FPSCameraZ *camera, float transitionTime, Vector3 position);

void ScriptAction_drawNarrationBottomBox(Script *script, ScriptAction *action);
void* ScriptAction_DrawNarrationBottomBoxData_new(const char *narrator, const char *text, int proceedOnEnter);
void ScriptAction_loadScene(Script *script, ScriptAction *action);

void ScriptAction_fadingCut(Script *script, ScriptAction *action);
void* ScriptAction_FadingCutData_new(float transitionTime, Color color, uint8_t fadeType, uint8_t fadeTweenType, float nextStepDelay, float direction);
#endif