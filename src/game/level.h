#ifndef __GAME_LEVEL_H__
#define __GAME_LEVEL_H__

#include "raylib.h"
#include <inttypes.h>
#include "cjson.h"

typedef struct Level Level;

typedef struct LevelMeshInstance
{
    Vector3 position;
    Vector3 eulerRotationDeg;
    Vector3 scale;
    Matrix toWorldTransform;
    int textureIndex;
} LevelMeshInstance;

typedef struct LevelMesh
{
    Model model;
    int textureIndex;
    int isDithered;
    char *filename;
    LevelMeshInstance *instances;
    int instanceCount;
} LevelMesh;

typedef struct LevelTexture
{
    Texture2D texture;
    char *filename;
} LevelTexture;

// a reference to a component instance. The id is the index of the component instance.
// When the component's entity id can't be resolved, the component instance is invalid, same as
// when the generation is different.
typedef struct LevelEntityComponentInstanceId
{
    uint32_t id;
    uint32_t generation;
    uint32_t componentId;
} LevelEntityComponentInstanceId;

typedef struct LevelEntityInstanceId
{
    uint32_t id;
    uint32_t generation;
} LevelEntityInstanceId;

typedef struct LevelEntityComponentClassMethods
{
    void (*updateFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData, float dt);
    void (*drawFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData);
    void (*onEnableFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData);
    void (*onDisableFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData);
    void (*onDestroyFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData);
    void (*onInitFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData);
    void (*onSerializeFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json);
    void (*onDeserializeFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json);
    void (*onEditorInspectFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *yPos);
    void (*onEditorMenuFn)(Level* level);
} LevelEntityComponentClassMethods;

typedef struct LevelEntityComponentClass
{
    uint32_t componentId;
    void *componentInstanceData;
    int componentInstanceDataSize;
    uint32_t *generations;
    LevelEntityInstanceId *ownerIds;
    char *name;
    int instanceCount;
    LevelEntityComponentClassMethods methods;
} LevelEntityComponentClass;

typedef struct LevelEntity
{
    uint32_t id;
    uint32_t generation;
    char *name;
    Vector3 position;
    Vector3 eulerRotationDeg;
    Vector3 scale;
    Matrix toWorldTransform;
} LevelEntity;

typedef struct Level {
    LevelMesh *meshes;
    int meshCount;
    LevelTexture *textures;
    int textureCount;

    LevelEntity *entities;
    int entityCount;
    LevelEntityComponentClass *entityComponentClasses;
    int entityComponentClassCount;
} Level;

void Level_init(Level *level);
void Level_loadAssets(Level *level, const char *assetsDirectory);
LevelMeshInstance* Level_addInstance(Level *level, const char *meshName, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale);
void Level_clearInstances(Level *level);
void Level_load(Level *level, const char *levelFile);
void Level_save(Level *level, const char *levelFile);
void Level_update(Level *level, float dt);
void Level_draw(Level *level);
void Level_unload(Level *level);
void Level_updateInstanceTransform(LevelMeshInstance *instance);

// registering a new entity component class. Use a unique componentId for each component, starting with 0.
// minimum data size is 1 byte.
void Level_registerEntityComponentClass(Level *level, uint32_t componentId, const char *name, LevelEntityComponentClassMethods methods, int componentInstanceDataSize);
// resolve a component data pointer by its id. Returns NULL if the id is invalid. Returns a pointer to the component class otherwise.
void* Level_resolveComponent(Level *level, LevelEntityComponentInstanceId id);
// resolves an entity by its id. Returns NULL if the id is invalid. Returns a pointer to the entity otherwise.
LevelEntity* Level_resolveEntity(Level *level, LevelEntityInstanceId id);
// get a component class by its id. Returns NULL if the id is invalid. Returns a pointer to the component class otherwise.
LevelEntityComponentClass* Level_getComponentClassById(Level *level, uint32_t componentId);

void Level_updateEntityTransform(LevelEntity *entity);
LevelEntity* Level_addEntity(Level *level, const char *name, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale);
LevelEntityComponentInstanceId Level_addEntityComponent(Level *level, LevelEntity *entity, uint32_t componentId,
    void **componentInstanceData);
LevelEntityComponentInstanceId Level_addEntityComponentAtIndex(Level *level, int index, LevelEntity *entity, uint32_t componentId,
    void **componentInstanceData);
void Level_deleteEntity(Level *level, LevelEntity *entity);
LevelEntity* Level_addEntityAtIndex(Level *level, int index, const char *name, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale);
#endif