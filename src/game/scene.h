#ifndef __SCENE_H__
#define __SCENE_H__

#include "main.h"

typedef struct SceneConfig SceneConfig;

typedef void (*SceneDrawFn)(GameContext *gameCtx, SceneConfig *SceneConfig);
typedef void (*SceneUpdateFn)(GameContext *gameCtx, SceneConfig *SceneConfig, float dt);
typedef void (*SceneInitFn)(GameContext *gameCtx, SceneConfig *SceneConfig);
typedef void (*SceneDeinitFn)(GameContext *gameCtx, SceneConfig *SceneConfig);

typedef struct SceneConfig {
    SceneDrawFn drawLevelFn;
    SceneDrawFn drawUiFn;
    SceneUpdateFn updateFn;
    SceneInitFn initFn;
    SceneDeinitFn deinitFn;
    uint32_t sceneId;
} SceneConfig;

// Allocate memory for the duration of the scene. Any allocation after a scene change
// becomes invalid. Copies *data to the allocated memory (if not NULL) and returns a pointer.
void* Scene_alloc(int size, void *data);
void Scene_init();
SceneConfig* Scene_getConfig(uint32_t sceneId);

#endif