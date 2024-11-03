#include "scene.h"
#include "_scenes.h"
#include <string.h>
#include <memory.h>

static int _allocationPoolIndex;
static char _allocationPool[2048];


SceneConfig* Scene_getConfig(uint32_t sceneId)
{
    SceneConfig *_scenes[] = {
        &_scene_editor,
        &_scene_start,
        &_scene_intro_scene,
        &_scene_island, 
        &_scene_puzzle_1,
        &_scene_puzzle_2,
        &_scene_puzzle_3, 
        &_scene_finish,
    };

    for (int i = 0; i < sizeof(_scenes) / sizeof(SceneConfig*); i++)
    {
        if (_scenes[i]->sceneId == sceneId)
        {
            return _scenes[i];
        }
    }

    return NULL;
}

void* Scene_alloc(int size, void *data)
{
    void *ptr = &_allocationPool[_allocationPoolIndex];
    //ensure alignment
    size = (size + 0xf) & ~0xf;
    _allocationPoolIndex += size;
    if (data) memcpy(ptr, data, size);
    return ptr;
}

void Scene_init()
{
    _allocationPoolIndex = 0;
}