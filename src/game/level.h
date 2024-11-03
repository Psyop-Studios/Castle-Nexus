#ifndef __GAME_LEVEL_H__
#define __GAME_LEVEL_H__

#include "raylib.h"
#include <inttypes.h>
#include "cjson.h"

typedef struct Level Level;
typedef struct LevelCollider LevelCollider;

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
    LevelCollider *colliders;
    int colliderCount;
} LevelMesh;

typedef struct LevelTextureSpriteAnimation
{
    char *name;
    int frameCount;
    float frameRate;
    Vector2 offset;
    Vector2 frameSize;
} LevelTextureSpriteAnimation;

typedef struct LevelTexture
{
    int index;
    Texture2D texture;
    char *filename;
    LevelTextureSpriteAnimation *animations;
    int animationCount;
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
    void (*onEditorInspectFn)(Level* level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *yPos, int isMouseOver);
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
    uint8_t transformIsDirty;
    char *name;
    Vector3 position;
    Vector3 eulerRotationDeg;
    Vector3 scale;
    Matrix toWorldTransform;
} LevelEntity;

#define LEVEL_COLLIDER_TYPE_SPHERE 0
#define LEVEL_COLLIDER_TYPE_AABOX 1

typedef struct LevelCollider {
    uint8_t type;
    uint8_t isTrigger;
    uint16_t id;
    Vector3 position;
    union
    {
        struct
        {
            float radius;
        } sphere;
        struct
        {
            Vector3 size;
        } aabox;
    };
} LevelCollider;

typedef struct Level {
    char *filename;
    char *assetDirectory;

    LevelMesh *meshes;
    int meshCount;
    LevelTexture *textures;
    int textureCount;

    LevelEntity *entities;
    int entityCount;
    LevelEntityComponentClass *entityComponentClasses;
    int entityComponentClassCount;

    LevelCollider *colliders;
    int colliderCount;

    double gameTime;
    double renderTime;
    uint8_t isEditor;

    const char *previousTriggerIds[16];
    int previousTriggerCount;
    const char *activeTriggerIds[16];
    int activeTriggerCount;

    float playerDistanceWalked;
} Level;


typedef struct LevelCollisionResult {
    LevelEntityInstanceId ownerId;
    Vector3 surfaceContact;
    Vector3 direction;
    Vector3 normal;
    float depth;
    int colliderId;
    const char *triggerId;
} LevelCollisionResult;

void Level_init(Level *level);
void Level_reloadAssets(Level *level);
void Level_loadAssets(Level *level, const char *assetsDirectory);
LevelMeshInstance* Level_addInstance(Level *level, const char *meshName, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale);
void Level_clearInstances(Level *level);
void Level_load(Level *level, const char *levelFile);
void Level_save(Level *level, const char *levelFile);
void Level_update(Level *level, float dt);
void Level_draw(Level *level);
void Level_unload(Level *level);
void Level_updateInstanceTransform(LevelMeshInstance *instance);
Texture2D Level_getTexture(Level *level, const char *filename, Texture2D fallback);
LevelTexture* Level_getLevelTexture(Level *level, const char *filename);
LevelMesh *Level_getMesh(Level *level, const char *filename);

int Level_isTriggerActive(Level *level, const char *name);
int Level_isTriggeredOn(Level *level, const char *name);
int Level_isTriggeredOff(Level *level, const char *name);
void Level_addTriggerId(Level *level, const char *triggerId);

void Level_addColliderSphere(Level *level, Vector3 position, float radius, int isTrigger);
void Level_addColliderBox(Level *level, Vector3 position, Vector3 size, int isTrigger);
int Level_findCollisions(Level *level, Vector3 position, float radius, uint8_t matchNormal, uint8_t matchTrigger,
    LevelCollisionResult *results, int maxResults);
int Level_testCollider(LevelCollider *collider, LevelCollisionResult *result, Vector3 position, float r);

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

LevelEntity* Level_instantiatePrefab(Level *level, cJSON *json);
cJSON* Level_serializeEntityAsPrefab(Level *level, LevelEntityInstanceId instanceId);

LevelCollisionResult Level_calcPenetrationDepth(Level *level, Vector3 point, float radius);

#endif