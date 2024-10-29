#ifndef __GAME_LEVEL_H__
#define __GAME_LEVEL_H__

#include "raylib.h"

typedef struct LevelMeshInstance
{
    Vector3 position;
    Vector3 eulerRotationDeg;
    Vector3 scale;
    Matrix toWorldTransform;
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

typedef struct Level {
    LevelMesh *meshes;
    int meshCount;
    LevelTexture *textures;
    int textureCount;
} Level;

void Level_init(Level *level);
void Level_loadAssets(Level *level, const char *assetsDirectory);
void Level_addInstance(Level *level, const char *meshName, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale);
void Level_clearInstances(Level *level);
void Level_load(Level *level, const char *levelFile);
void Level_save(Level *level, const char *levelFile);
void Level_update(Level *level, float dt);
void Level_draw(Level *level);
void Level_unload(Level *level);
void Level_updateInstanceTransform(LevelMeshInstance *instance);

#endif