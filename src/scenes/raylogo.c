#include "../scene.h"
#include "../timer.h"
#include <raylib.h>
#include <math.h>
#include <raymath.h>
#include <stdio.h>
#include "../helper_functions.h"

// TODO: these should be static const ints
#define RAYLOGO_WIDTH 290
#define RAYLOGO_HEIGHT 300
#define RAYLOGO_SQUARE_SIZE 15
#define RAYLOGO_COLUMNS (RAYLOGO_WIDTH / RAYLOGO_SQUARE_SIZE)
#define RAYLOGO_ROWS (RAYLOGO_HEIGHT / RAYLOGO_SQUARE_SIZE)

extern const int screenWidth;
extern const int screenHeight;

static Timer raylogoTimer = {0};

#define PSY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

const static float animationDuration = 1.0f;
static float elapsedTime = 0.0f;

void InitRaylogoScene(void)
{
    StartTimer(&raylogoTimer, 3.5f);
}

void UpdateRaylogoScene(void)
{
    if (!raylogoTimer.isRunning)
    {
        ChangeScene(GAME);
    }
    UpdateTimer(&raylogoTimer);
    elapsedTime = GetTime() - animationDuration;
}

void DrawRaylogoScene(void)
{
    ClearBackground(RAYWHITE);

    // Text

    Vector2 textPos = (Vector2){screenWidth / 2 - MeasureText("raylib", 52) / 2 + 35, screenHeight / 2 + 65};

    Vector2 whiteBoxSize = {
        .x = (MeasureText("raylib", 52) + 40) * (1 - elapsedTime),
        .y = 80,
    };

    DrawText("raylib", (int)textPos.x, (int)textPos.y, 52, BLACK);
    DrawRectangleV(textPos, whiteBoxSize, RAYWHITE);

    // Borders

    for (int row = 0; row < RAYLOGO_ROWS; row++)
    {
        for (int column = 0; column < RAYLOGO_COLUMNS; column++)
        {
            const bool is_border_square = row == 0 || column == 0 || row == RAYLOGO_ROWS - 1 || column == RAYLOGO_COLUMNS - 1;
            if (!is_border_square)
                continue;

            float delay = 1 * (fabs(column - row) / (float)((RAYLOGO_ROWS + RAYLOGO_COLUMNS) / 2));
            float amount = PSY_MIN(1, PSY_MAX(0, elapsedTime - delay) * 2);
            float currentSize = Lerp(0, RAYLOGO_SQUARE_SIZE, amount);

            Vector2 pos = {.x = screenWidth / 2 - RAYLOGO_WIDTH / 2 + column * RAYLOGO_SQUARE_SIZE, .y = screenHeight / 2 - RAYLOGO_HEIGHT / 2 + row * RAYLOGO_SQUARE_SIZE};

            DrawRectangleV(pos, (Vector2){currentSize, currentSize}, BLACK);
        }
    }
}