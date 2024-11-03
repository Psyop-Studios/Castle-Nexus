/*******************************************************************************************
 *
 *   raylib gamejam template
 *
 *   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
 *
 *   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS       // Force custom modal dialogs usage
#include <emscripten/emscripten.h> // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>  // Required for: printf()
#include <stdlib.h> // Required for:
#include <string.h> // Required for:

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define HONK printf("HONK");

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum
{
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING
} GameScreen;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

// TODO: Define global variables here, recommended to make them static

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void); // Update and Draw one frame

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    // SetTraceLogLevel(LOG_NONE); // Disable raylib trace log messages
    SetTraceLogLevel(LOG_WARNING); // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_WINDOW_RESIZABLE); // Enable resizable window
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");

    // TODO: Load resources / Initialize variables at this point

    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    // target = LoadRenderTexture(screenWidth, screenHeight);
    // SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60); // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // TODO: Unload all loaded resources at this point

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

#ifdef PLATFORM_DESKTOP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void (*Game_init)(void **);
void (*Game_deinit)();
void (*Game_update)();

static void *contextData = NULL;

void init()
{
    if (Game_init)
    {
        Game_init(&contextData);
    }
}

void deinit()
{
    if (Game_deinit)
    {
        Game_deinit();
    }
}

void update()
{
    if (Game_update)
    {
        Game_update();
    }
}

void unload_game();
void load_game();

static int is_built = 0;
static void build_game()
{
    #ifndef PLATFORM_WEB
    SetTraceLogLevel(LOG_WARNING);
    #endif
    is_built = 1;
    deinit();
    unload_game();
    Game_deinit = NULL;
    Game_init = NULL;
    Game_update = NULL;
    char buildCommand[4096] = {0};
    char cfilelist[2048] = {0};
    long lastModTime = 0;
    FilePathList files = LoadDirectoryFiles("game");
    for (int i = 0; i < files.count; i++)
    {
        const char *file = files.paths[i];
        const char *ext = GetFileExtension(file);
        if (strcmp(ext, ".c") == 0)
        {
            strcat(cfilelist, file);
            strcat(cfilelist, " ");
            long modTime = GetFileModTime(file);
            if (modTime > lastModTime)
            {
                lastModTime = modTime;
            }
        }
    }
    UnloadDirectoryFiles(files);

#if defined(_WIN32) || defined(_WIN64)
    const char *dllName = "game.dll";
#else
    const char *dllName = "game.so";
#endif


    if (lastModTime > GetFileModTime(dllName))
    {
        sprintf(buildCommand, "gcc -g -Wall -I../../raylib/src -L../../raylib/src -o %s -shared -fPIC %s -lraylib -DCJSON_HIDE_SYMBOLS",
                dllName, cfilelist);

        printf("Building game: %s\n", buildCommand);
        int result = system(buildCommand);
        printf("result : %d\n", result);
    }
    load_game();
    init();
    printf("Game built and loaded\n");
}
#else

int isInitialized = 0;
void *contextData = NULL;

void Game_init(void **contextData);
void Game_deinit();
void Game_update();

void init()
{
    Game_init(&contextData);
}

void deinit()
{
    Game_deinit();
}

void update()
{
    if (!isInitialized)
    {
        isInitialized = 1;
        init();
    }
    Game_update();
}
#endif
static int run_foreground = 0;
//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
// Update and draw frame
void UpdateDrawFrame(void)
{
#ifdef PLATFORM_DESKTOP
    if ((IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_CONTROL)) || !is_built)
    {
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            // trigger complete rebuild and reset context data
            contextData = 0;
        }
        #if !defined(_WIN32) && !defined(_WIN64)
        system("reset");
        #endif
        build_game();
    }

    if (IsKeyPressed(KEY_F))
    {


        run_foreground = !run_foreground;


        if (run_foreground)
            SetWindowState(FLAG_WINDOW_TOPMOST);
        else
            ClearWindowState(FLAG_WINDOW_TOPMOST);
    }

    static int isPaused = 0;
    if (IsKeyReleased(KEY_P) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        isPaused = !isPaused;
        printf("Toggle pause: %s\n", isPaused ? "true" : "false");
    }
    if (isPaused)
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("PAUSED", GetScreenWidth() / 2 - MeasureText("PAUSED", 40) / 2, GetScreenHeight() / 2 - 40, 40, GRAY);
        EndDrawing();
        return;
    }
#endif

    update();
}