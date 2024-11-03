#ifndef ___SCENES_H__
#define ___SCENES_H__

#include "main.h"
#include "scene.h"

#define SCENE_ID_INVALID -1
#define SCENE_ID_EDITOR 0
#define SCENE_ID_START 1
#define SCENE_ID_START_INTRO 2
#define SCENE_ID_START_ISLAND 3
#define SCENE_ID_PUZZLE_1 4
#define SCENE_ID_PUZZLE_2 5
#define SCENE_ID_PUZZLE_3 6
#define SCENE_ID_FINISH 7

extern SceneConfig _scene_editor;
extern SceneConfig _scene_start;
extern SceneConfig _scene_finish;
extern SceneConfig _scene_intro_scene;
extern SceneConfig _scene_island;
extern SceneConfig _scene_puzzle_1;
extern SceneConfig _scene_puzzle_2;
extern SceneConfig _scene_puzzle_3;

#endif