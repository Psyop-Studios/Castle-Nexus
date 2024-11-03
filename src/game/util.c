#include "raylib.h"
#include "rlgl.h"
#include <string.h>
#include "main.h"
#include "dusk-gui.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int textLineSpacing = 0;
void SetTextLineSpacingEx(int spacing)
{
    SetTextLineSpacing(spacing);
    textLineSpacing = spacing;
}

static float CalcNextRichtextWordWidth(Font font, const char *text, float fontSize, float spacing);



static int hexToInt(char chr)
{
    if (chr >= '0' && chr <= '9') return chr - '0';
    if (chr >= 'a' && chr <= 'f') return chr - 'a' + 10;
    if (chr >= 'A' && chr <= 'F') return chr - 'A' + 10;
    return -1;
}

static float CalcNextRichtextWordWidth(Font font, const char *text, float fontSize, float spacing)
{
    if (font.texture.id == 0) font = GetFontDefault();  // Safety check in case of not valid font

    float scaleFactor = fontSize/font.baseSize;
    int size = TextLength(text);
    float textOffsetX = 0.0f;

    for (int i=0; i < size;)
    {
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);

        if (codepoint == '[')
        {
            // check for color tag
            if (strncmp(&text[i], "[color=", 7) == 0 && size - i >= 11 && text[i+11] == ']')
            {
                int r = hexToInt(text[i + 7]);
                int g = hexToInt(text[i + 8]);
                int b = hexToInt(text[i + 9]);
                int a = hexToInt(text[i + 10]);
                char word[5] = {text[i + 7], text[i + 8], text[i + 9], text[i + 10], '\0'};
                if (strcmp(word, "blac") == 0) { i+=12; continue; }
                else if (strcmp(word, "whit") == 0) { i+=12; continue; }
                else if (strcmp(word, "red_") == 0) { i+=12; continue; }
                else if (strcmp(word, "blue") == 0) { i+=12; continue; }
                else if (strcmp(word, "gree") == 0) { i+=12; continue; }
                else if (strcmp(word, "yell") == 0) { i+=12; continue; }
                else if (strcmp(word, "grey") == 0) { i+=12; continue; }
                else if (strcmp(word, "purp") == 0) { i+=12; continue; }
                else if (r >= 0 && g >= 0 && b >= 0 && a >= 0)
                {
                    i+= 12;
                    continue;
                }
            }
            if (strncmp(&text[i], "[/color]", 8) == 0)
            {
                i += 8;
                continue;
            }
        }

        if (codepoint <= ' ')
        {
            return textOffsetX;
        }
        int index = GetGlyphIndex(font, codepoint);
        if (font.glyphs[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
        else textOffsetX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
        i+= codepointByteCount;
    }

    return textOffsetX;
}

// Draw text using Font
// NOTE: chars spacing is NOT proportional to fontSize
Vector2 DrawTextRich(Font font, const char *text, Vector2 position, float fontSize, float spacing, int wrapWidth, Color tint, int noDraw)
{
    if (font.texture.id == 0) font = GetFontDefault();  // Safety check in case of not valid font

    int size = TextLength(text);    // Total size in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;         // Character quad scaling factor
    int alpha = tint.a;
    Color colorStack[16];
    int colorStackIndex = 0;
    Color startColor = tint;

    float maxWidth = 0.0f;

    for (int i = 0; i < size;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == '[')
        {
            // check for color tag
            if (strncmp(&text[i], "[color=", 7) == 0 && size - i >= 11 && text[i+11] == ']')
            {
                // select from db8 color palette
                char word[5] = {text[i + 7], text[i + 8], text[i + 9], text[i + 10], '\0'};
                Color color = {255,0,255,0};
                if (strcmp(word, "blac") == 0) color = DB8_BLACK;
                else if (strcmp(word, "whit") == 0) color = DB8_WHITE;
                else if (strcmp(word, "red_") == 0) color = DB8_RED;
                else if (strcmp(word, "blue") == 0) color = DB8_BLUE;
                else if (strcmp(word, "gree") == 0) color = DB8_GREEN;
                else if (strcmp(word, "yell") == 0) color = DB8_YELLOW;
                else if (strcmp(word, "grey") == 0) color = DB8_GREY;
                else if (strcmp(word, "purp") == 0) color = DB8_DEEPPURPLE;
                else
                {
                    // parse hex color
                    int r = hexToInt(text[i + 7]);
                    int g = hexToInt(text[i + 8]);
                    int b = hexToInt(text[i + 9]);
                    int a = hexToInt(text[i + 10]);
                    if (r >= 0 && g >= 0 && b >= 0 && a >= 0)
                    {
                        // valid color tag
                        color = (Color) {r | r << 4, g | g << 4, b | b << 4, 
                            (a | a << 4) * alpha / 255 };
                    }
                }
                if (color.a != 0)
                {
                    i+= 12;
                    if (colorStackIndex < 16)
                        colorStack[colorStackIndex++] = tint;
                    else TraceLog(LOG_WARNING, "Color stack overflow");
                    tint = color;
                    continue;
                }
            }
            if (strncmp(&text[i], "[/color]", 8) == 0)
            {
                if (colorStackIndex > 0)
                {
                    tint = colorStack[--colorStackIndex];
                }
                else
                {
                    tint = startColor;
                }
                i += 8;
                continue;
            }
        }

        if (codepoint == '\n')
        {
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            maxWidth = fmaxf(maxWidth, textOffsetX);
            textOffsetY += (fontSize + textLineSpacing);
            textOffsetX = 0.0f;
        }
        else
        {
            float posX = position.x + textOffsetX;
            if (font.glyphs[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                if (!noDraw) DrawTextCodepoint(font, codepoint, (Vector2){ posX, position.y + textOffsetY }, fontSize, tint);
            }
            else {
                // check next word, measure width, check if it would fit. If not, start new line.
                float nextWordWidth = CalcNextRichtextWordWidth(font, &text[i + codepointByteCount], fontSize, spacing);
                if (nextWordWidth + textOffsetX > wrapWidth)
                {
                    maxWidth = fmaxf(maxWidth, textOffsetX);
                    textOffsetY += (fontSize + textLineSpacing);
                    textOffsetX = 0.0f;
                    i += codepointByteCount;
                    continue;
                }
            }

        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }

    maxWidth = fmaxf(maxWidth, textOffsetX);
    return (Vector2){ maxWidth, textOffsetY + fontSize };
}

Rectangle DrawTextBoxAligned(Font font, const char *text, int x, int y, int w, int h, float alignX, float alignY, Color color)
{
    float fontSpacing = -2.0f;
    float fontSize = font.baseSize * 2.0f;
    Vector2 textSize = DrawTextRich(font, text, (Vector2){0, 0}, fontSize, fontSpacing, w, color, 1);
    int posX = x + (int)((w - textSize.x) * alignX);
    int posY = y + (int)((h - textSize.y) * alignY);
    DrawTextRich(font, text, (Vector2){posX, posY}, fontSize, fontSpacing, w, color, 0);

    return (Rectangle) {
        .x = posX, .y = posY, .width = textSize.x, .height = textSize.y
    };
}

// Rectangle DrawStyledTextBox(StyledTextBox styledTextBox)
// {
//     char text[1024] = {0};
//     int linesToDisplay = styledTextBox.displayedLineCount;
//     for (int i=0;styledTextBox.text[i];i++)
//     {
//         text[i] = styledTextBox.text[i];
//         if (text[i] == '\n')
//         {
//             linesToDisplay--;
//             if (linesToDisplay == 0)
//             {
//                 text[i] = '\0';
//                 break;
//             }
//         }
//     }

//     Rectangle rect = DrawTextBoxAligned(text, styledTextBox.fontSize,
//         styledTextBox.box.x, styledTextBox.box.y,
//         styledTextBox.box.width, styledTextBox.box.height, 
//         styledTextBox.align.x, styledTextBox.align.y,
//         styledTextBox.color);
//     if (styledTextBox.underScoreSize > 0)
//     {
//         DrawRectangle(rect.x, rect.y + rect.height + styledTextBox.underScoreOffset, 
//             rect.width, styledTextBox.underScoreSize, styledTextBox.color);
//     }
//     return rect;
// }



char* replacePathSeps(char *path)
{
    for (int i = 0; i < strlen(path); i++)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }
    return path;
}

extern Vector3 _worldCursor;

int SceneDrawUi_transformUi(float *posY, const char *uiId, Vector3 *position, Vector3 *euler, Vector3 *scale, Vector3 *cursorAnchor, Vector3 maxMoveDist)
{
    int modified = 0;
    char buffer[256];
    // Position input fields
    if (position)
    {

        sprintf(buffer, "%.3f##X-%s", position->x, uiId);
        
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 10, *posY, 60, 20 },
        }, &position->x, cursorAnchor->x - maxMoveDist.x, cursorAnchor->x + maxMoveDist.x, 0.025f))
        {
            modified = 1;
        }
        
        sprintf(buffer, "%.3f##Y-%s", position->y, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 70, *posY, 60, 20 },
        }, &position->y, cursorAnchor->y - maxMoveDist.y, cursorAnchor->y + maxMoveDist.y, 0.025f))
        {
            modified = 1;
        }
        
        sprintf(buffer, "%.3f##Z-%s", position->z, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 130, *posY, 60, 20 },
        }, &position->z, cursorAnchor->z - maxMoveDist.z, cursorAnchor->z + maxMoveDist.z, 0.025f))
        {
            modified = 1;
        }
        
        *posY += 20.0f;
    }
    // Rotation input fields
    if (euler)
    {

        sprintf(buffer, "%.3f##RX-%s", euler->x, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 10, *posY, 60, 20 },
        }, &euler->x, -3000.0f, 3000.0f, 1.0f))
        {
            modified = 1;
        }
        
        sprintf(buffer, "%.3f##RY-%s", euler->y, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 70, *posY, 60, 20 },
        }, &euler->y, -3000.0f, 3000.0f, 1.0f))
        {
            modified = 1;
        }
        
        sprintf(buffer, "%.3f##RZ-%s", euler->z, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 130, *posY, 60, 20 },
        }, &euler->z, -3000.0f, 3000.0f, 1.0f))
        {
            modified = 1;
        }
        
        *posY += 20.0f;
        if (DuskGui_button((DuskGuiParams) {
            .text = TextFormat("Reset Rotation##reset-rot-%s", uiId), .rayCastTarget = 1, .bounds = (Rectangle) { 10, *posY, 60, 20 }}))
        {
            *euler = (Vector3){0, 0, 0};
            modified = 1;
        }
        

        if (DuskGui_button((DuskGuiParams) {
            .text = TextFormat("<-##<-%s", uiId), .rayCastTarget = 1, .bounds = (Rectangle) { 70, *posY, 60, 20 }}))
        {
            euler->y += -45.0f;
            modified = 1;
        }
        
        if (DuskGui_button((DuskGuiParams) {
            .text = TextFormat("->##->%s", uiId), .rayCastTarget = 1, .bounds = (Rectangle) { 130, *posY, 60, 20 }}))
        {
            euler->y -= -45.0f;
            modified = 1;
        }

        *posY += 20.0f;
    }

    // scale
    if (scale)
    {
        sprintf(buffer, "%.3f##SX-%s", scale->x, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 10, *posY, 60, 20 },
        }, &scale->x, 0.1f, 10.0f, 0.1f))
        {
            modified = 1;
        }

        sprintf(buffer, "%.3f##SY-%s", scale->y, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 70, *posY, 60, 20 },
        }, &scale->y, 0.1f, 10.0f, 0.1f))
        {
            modified = 1;
        }

        sprintf(buffer, "%.3f##SZ-%s", scale->z, uiId);
        if (DuskGui_floatInputField((DuskGuiParams) {
            .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle) { 130, *posY, 60, 20 },
        }, &scale->z, 0.1f, 10.0f, 0.1f))
        {
            modified = 1;
        }

        *posY += 20.0f;

        if (DuskGui_button((DuskGuiParams) {
            .text = TextFormat("Reset Scale##reset-scale-%s", uiId), .rayCastTarget = 1, .bounds = (Rectangle) { 10, *posY, 60, 20 }}))
        {
            *scale = (Vector3){1, 1, 1};
            modified = 1;
        }

        float maxAbsScale = fmaxf(fmaxf(fabsf(scale->x), fabsf(scale->y)), fabsf(scale->z));
        if (maxAbsScale > 0.0f)
        {
            float scaleFac = maxAbsScale;
            sprintf(buffer, "%.3f##scale_xyz-%s", scaleFac, uiId);
            if (DuskGui_floatInputField((DuskGuiParams){
                .text = buffer, .rayCastTarget = 1, .bounds = (Rectangle){70, *posY, 60, 20},
            }, &scaleFac, 0.0f, 50.0f, 0.025f))
            {
                scaleFac /= maxAbsScale;
                scale->x *= scaleFac;
                scale->y *= scaleFac;
                scale->z *= scaleFac;
                modified = 1;
            }
        }

        *posY += 20.0f;
    }
    
    return modified;
}

float EaseInOutSine(float t, float from, float to)
{
    if (t < 0.0f) return from;
    if (t > 1.0f) return to;
    return sinf(t * PI * 0.5f) * (to - from) + from;
}


float LerpAngle(float a, float b, float t)
{
    float delta = b - a;
    if (delta > PI) delta -= 2 * PI;
    if (delta < -PI) delta += 2 * PI;
    return a + delta * t;
}