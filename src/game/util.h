#ifndef __UTIL_H__
#define __UTIL_H__

#include "raylib.h"
#include <inttypes.h>

void SetTextLineSpacingEx(int spacing);
void DrawTextRich(Font font, const char *text, Vector2 position, float fontSize, float spacing, int wrapWidth, Color tint);
// void DrawTextRich(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint);
Rectangle DrawTextBoxAligned(Font font, const char *text, int x, int y, int w, int h, float alignX, float alignY, Color color);
char* replacePathSeps(char *path);

int SceneDrawUi_transformUi(float *posY, const char *uiId, Vector3 *position, Vector3 *euler, Vector3 *scale, Vector3 *cursorAnchor);
#endif