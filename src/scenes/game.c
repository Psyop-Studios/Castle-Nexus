#include "game.h"
#include <raylib.h>

Camera3D playerCamera;

void InitGameScene(void)
{
    playerCamera = (Camera3D){0};
    playerCamera.position = (Vector3){0.0f, 5.0f, 5.0f};
    playerCamera.target = (Vector3){0.0f, 0.0f, 0.0f};
    playerCamera.up = (Vector3){0.0f, 1.0f, 0.0f};
    playerCamera.fovy = 60.0f;               // Wider field of view
    playerCamera.projection = CAMERA_CUSTOM; // Changed from FIRST_PERSON
}

void UpdateGameScene(void)
{

    UpdateCamera(&playerCamera, CAMERA_CUSTOM);
}

void DrawGameScene(void)
{
    ClearBackground(BLUE);
    BeginMode3D(playerCamera);
    DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){20.0f, 20.0f}, LIGHTGRAY);
    DrawCube((Vector3){0.0f, 1.0f, 0.0f}, 2.0f, 2.0f, 2.0f, RED);
    DrawCubeWires((Vector3){0.0f, 1.0f, 0.0f}, 2.0f, 2.0f, 2.0f, MAROON);
    DrawSphere((Vector3){-4.0f, 1.0f, 2.0f}, 1.0f, BLACK);
    DrawCylinder((Vector3){4.0f, 0.0f, -2.0f}, 1.0f, 1.0f, 3.0f, 8, GREEN);
    DrawGrid(10, 1.0f);
    EndMode3D();

    DrawText(TextFormat("Camera Pos: %.2f, %.2f, %.2f",
                        playerCamera.position.x,
                        playerCamera.position.y,
                        playerCamera.position.z),
             10, 40, 20, WHITE);
}