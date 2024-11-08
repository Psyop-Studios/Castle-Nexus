#ifndef __GAME_LEVEL_COMPONENTS_H__

#include "raylib.h"
#include "level.h"
#include <inttypes.h>

#define COMPONENT_TYPE_CAMERAFACING 0
#define COMPONENT_TYPE_WOBBLER 1
#define COMPONENT_TYPE_MESHRENDERER 2
#define COMPONENT_TYPE_SPRITERENDERER 3
#define COMPONENT_TYPE_COLLIDER_BOX 4
#define COMPONENT_TYPE_RIGID_SPHERE 5
#define COMPONENT_TYPE_COLLISION_DETECTOR 6
#define COMPONENT_TYPE_DOOR_STATE 7
#define COMPONENT_TYPE_SWITCH 8

typedef struct ColliderBoxComponent
{
    Vector3 size;
    Vector3 offset;
    uint8_t isTrigger;
} ColliderBoxComponent;

void LevelComponents_register(Level *level);

#endif