#ifndef ___SCENES_H__
#define ___SCENES_H__

#include "main.h"
#include "scene.h"

#define SCENE_ID_INVALID -1
#define SCENE_ID_EDITOR 0
#define SCENE_ID_START 1
#define SCENE_ID_START_INTRO 2
#define SCENE_ID_START_ISLAND 3

extern SceneConfig _scene_editor;
extern SceneConfig _scene_start;
extern SceneConfig _scene_intro_scene;
extern SceneConfig _scene_island;

#endif