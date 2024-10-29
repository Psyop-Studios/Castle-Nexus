#include "level.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "cjson.h"
#include <raymath.h>
#include "main.h"

void Level_init(Level *level)
{
    level->meshCount = 0;
    level->meshes = NULL;
    level->textureCount = 0;
    level->textures = NULL;
}

static int Level_getTextureIndexByName(Level *level, const char *name)
{
    TraceLog(LOG_INFO, "Looking for texture: %s / %d", name, level->textureCount);
    for (int i = 0; i < level->textureCount; i++)
    {
        const char *lastSlash = strrchr(level->textures[i].filename, '/');
        if (!lastSlash)
        {
            lastSlash = level->textures[i].filename;
        }
        else
        {
            lastSlash++;
        }
        TraceLog(LOG_INFO, "Comparing to: %s", lastSlash);
        if (strcmp(lastSlash, name) == 0)
        {
            TraceLog(LOG_INFO, "Texture found in level assets: %s", name);
            return i;
        }
    }

    TraceLog(LOG_ERROR, "Texture not found in level assets: %s", name);
    return -1;
}

char* replacePathSeps(char *path)
{
    for (int i = 0; i < strlen(path); i++)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }
    return path;
}

void Level_loadAssets(Level *level, const char *assetDirectory)
{
    TraceLog(LOG_INFO, "Loading level assets from: %s", assetDirectory);
    FilePathList files = LoadDirectoryFiles(assetDirectory);
    level->textureCount = 0;
    level->meshCount = 0;

    for (int i = 0; i < files.count; i++)
    {
        const char *file = files.paths[i];
        const char *ext = GetFileExtension(file);
        if (strcmp(ext, ".glb") == 0)
        {
            level->meshCount++;
        }
        if (strcmp(ext, ".png") == 0)
        {
            TraceLog(LOG_INFO, "Texture found: %s[%d]", file, level->textureCount);
            level->textureCount++;
        }
    }
    level->meshes = (LevelMesh*)malloc(level->meshCount * sizeof(LevelMesh));
    level->textures = (LevelTexture*)malloc(level->textureCount * sizeof(LevelTexture));
    memset(level->meshes, 0, level->meshCount * sizeof(LevelMesh));
    memset(level->textures, 0, level->textureCount * sizeof(LevelTexture));

    int meshIndex = 0;
    int textureCount = 0;
    for (int i = 0; i < files.count; i++)
    {
        const char *file = files.paths[i];
        const char *ext = GetFileExtension(file);
        if (strcmp(ext, ".png") == 0)
        {
            level->textures[textureCount].texture = LoadTexture(file);
            level->textures[textureCount].filename = replacePathSeps(strdup(file));
            textureCount++;
        }
    }
    
    for (int i = 0; i < files.count; i++)
    {
        const char *file = files.paths[i];
        const char *ext = GetFileExtension(file);
        if (strcmp(ext, ".glb") == 0)
        {
            char metaFileName[1024];
            strcpy(metaFileName, file);
            strcat(metaFileName, ".meta");
            if (FileExists(metaFileName))
            {
                TraceLog(LOG_INFO, "Loading meta file: %s", metaFileName);
                char *data = LoadFileText(metaFileName);
                cJSON *root = cJSON_Parse(data);
                cJSON *textureName = cJSON_GetObjectItem(root, "texture");
                cJSON *dithered = cJSON_GetObjectItem(root, "dithered");
                if (textureName && textureName->type == cJSON_String)
                {
                    level->meshes[meshIndex].textureIndex = Level_getTextureIndexByName(level, textureName->valuestring);
                }
                if (dithered && dithered->type == cJSON_True)
                {
                    level->meshes[meshIndex].isDithered = 1;
                }
                cJSON_Delete(root);
                UnloadFileText(data);
            } else {
                TraceLog(LOG_INFO, "No meta file found for: %s", file);
            }

            TraceLog(LOG_INFO, "Loading mesh: %s", file);
            level->meshes[meshIndex].model = LoadModel(file);
            level->meshes[meshIndex].filename = replacePathSeps(strdup(file));
            level->meshes[meshIndex].instances = NULL;
            level->meshes[meshIndex].instanceCount = 0;
            meshIndex++;
        }
    }
}

void Level_clearInstances(Level *level)
{
    for (int i = 0; i < level->meshCount; i++)
    {
        free(level->meshes[i].instances);
        level->meshes[i].instances = NULL;
        level->meshes[i].instanceCount = 0;
    }
}

void Level_updateInstanceTransform(LevelMeshInstance *instance)
{
    Vector3 position = instance->position;
    Vector3 eulerRotationDeg = instance->eulerRotationDeg;
    Vector3 scale = instance->scale;
    instance->toWorldTransform = MatrixRotateXYZ((Vector3){DEG2RAD * eulerRotationDeg.x, DEG2RAD * eulerRotationDeg.y, DEG2RAD * eulerRotationDeg.z});
    instance->toWorldTransform = MatrixMultiply(instance->toWorldTransform, MatrixScale(scale.x, scale.y, scale.z));
    instance->toWorldTransform = MatrixMultiply(instance->toWorldTransform, MatrixTranslate(position.x, position.y, position.z));
    
}

void Level_addInstance(Level *level, const char *meshName, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale)
{
    for (int i = 0; i < level->meshCount; i++)
    {
        if (strcmp(level->meshes[i].filename, meshName) == 0)
        {
            LevelMesh *mesh = &level->meshes[i];
            mesh->instanceCount++;
            mesh->instances = (LevelMeshInstance*)realloc(mesh->instances, mesh->instanceCount * sizeof(LevelMeshInstance));
            LevelMeshInstance *instance = &mesh->instances[mesh->instanceCount - 1];
            instance->position = position;
            instance->eulerRotationDeg = eulerRotationDeg;
            instance->scale = scale;
            Level_updateInstanceTransform(instance);
            Matrix m = instance->toWorldTransform;
            TraceLog(LOG_INFO, "pos: %.2f %.2f %.2f", position.x, position.y, position.z);
            TraceLog(LOG_INFO, "Matrix: \n"
                "%.2f %.2f %.2f %.2f\n"
                "%.2f %.2f %.2f %.2f\n"
                "%.2f %.2f %.2f %.2f\n"
                "%.2f %.2f %.2f %.2f\n"
                , m.m0, m.m4, m.m8, m.m12,
                m.m1, m.m5, m.m9, m.m13,
                m.m2, m.m6, m.m10, m.m14,
                m.m3, m.m7, m.m11, m.m15
            );
            return;
        }
    }
}

void Level_load(Level *level, const char *levelFile)
{
    if (strlen(levelFile) == 0)
    {
        TraceLog(LOG_ERROR, "Level file name is empty");
        return;
    }
    Level_clearInstances(level);
    TraceLog(LOG_INFO, "Loading level from: %s", levelFile);
    char *data = LoadFileText(levelFile);
    if (!data)
    {
        TraceLog(LOG_ERROR, "Failed to load level file: %s", levelFile);
        return;
    }
    cJSON *root = cJSON_Parse(data);
    if (!root)
    {
        TraceLog(LOG_ERROR, "Failed to parse level file: %s", levelFile);
        UnloadFileText(data);
        return;
    }

    cJSON *meshes = cJSON_GetObjectItem(root, "meshes");

    if (!meshes)
    {
        TraceLog(LOG_ERROR, "No meshes found in level file: %s", levelFile);
        goto cleanup;
    }

    for (int i = 0; i < cJSON_GetArraySize(meshes); i++)
    {
        cJSON *meshObj = cJSON_GetArrayItem(meshes, i);
        cJSON *filename = cJSON_GetObjectItem(meshObj, "filename");
        cJSON *instancesPX = cJSON_GetObjectItem(meshObj, "instancesPX");
        cJSON *instancesPY = cJSON_GetObjectItem(meshObj, "instancesPY");
        cJSON *instancesPZ = cJSON_GetObjectItem(meshObj, "instancesPZ");
        cJSON *instancesRX = cJSON_GetObjectItem(meshObj, "instancesRX");
        cJSON *instancesRY = cJSON_GetObjectItem(meshObj, "instancesRY");
        cJSON *instancesRZ = cJSON_GetObjectItem(meshObj, "instancesRZ");
        cJSON *instancesSX = cJSON_GetObjectItem(meshObj, "instancesSX");
        cJSON *instancesSY = cJSON_GetObjectItem(meshObj, "instancesSY");
        cJSON *instancesSZ = cJSON_GetObjectItem(meshObj, "instancesSZ");

        if (!filename || !instancesPX || !instancesPY || !instancesPZ || !instancesRX || !instancesRY || !instancesRZ || !instancesSX || !instancesSY || !instancesSZ)
        {
            TraceLog(LOG_ERROR, "Invalid mesh object in level file: %s", levelFile);
            continue;
        }

        const char *meshName = filename->valuestring;

        for (int j = 0; j < cJSON_GetArraySize(instancesPX); j++)
        {
            Vector3 position = (Vector3){cJSON_GetArrayItem(instancesPX, j)->valuedouble, cJSON_GetArrayItem(instancesPY, j)->valuedouble, cJSON_GetArrayItem(instancesPZ, j)->valuedouble};
            Vector3 eulerRotationDeg = (Vector3){cJSON_GetArrayItem(instancesRX, j)->valuedouble, cJSON_GetArrayItem(instancesRY, j)->valuedouble, cJSON_GetArrayItem(instancesRZ, j)->valuedouble};
            Vector3 scale = (Vector3){cJSON_GetArrayItem(instancesSX, j)->valuedouble, cJSON_GetArrayItem(instancesSY, j)->valuedouble, cJSON_GetArrayItem(instancesSZ, j)->valuedouble};
            Level_addInstance(level, meshName, position, eulerRotationDeg, scale);
        }
    }

    cleanup:
    cJSON_Delete(root);
    UnloadFileText(data);
    return;
}

void Level_save(Level *level, const char *levelFile)
{
    if (strlen(levelFile) == 0)
    {
        TraceLog(LOG_ERROR, "Level file name is empty");
        return;
    }

    TraceLog(LOG_INFO, "Saving level to: %s", levelFile);
    cJSON *root = cJSON_CreateObject();
    cJSON *meshes = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "meshes", meshes);
    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];
        cJSON *meshObj = cJSON_CreateObject();
        cJSON_AddItemToArray(meshes, meshObj);
        cJSON_AddStringToObject(meshObj, "filename", mesh->filename);
        cJSON *instancesPX = cJSON_CreateArray();
        cJSON *instancesPY = cJSON_CreateArray();
        cJSON *instancesPZ = cJSON_CreateArray();
        cJSON *instancesRX = cJSON_CreateArray();
        cJSON *instancesRY = cJSON_CreateArray();
        cJSON *instancesRZ = cJSON_CreateArray();
        cJSON *instancesSX = cJSON_CreateArray();
        cJSON *instancesSY = cJSON_CreateArray();
        cJSON *instancesSZ = cJSON_CreateArray();
        cJSON_AddItemToObject(meshObj, "instancesPX", instancesPX);
        cJSON_AddItemToObject(meshObj, "instancesPY", instancesPY);
        cJSON_AddItemToObject(meshObj, "instancesPZ", instancesPZ);
        cJSON_AddItemToObject(meshObj, "instancesRX", instancesRX);
        cJSON_AddItemToObject(meshObj, "instancesRY", instancesRY);
        cJSON_AddItemToObject(meshObj, "instancesRZ", instancesRZ);
        cJSON_AddItemToObject(meshObj, "instancesSX", instancesSX);
        cJSON_AddItemToObject(meshObj, "instancesSY", instancesSY);
        cJSON_AddItemToObject(meshObj, "instancesSZ", instancesSZ);

        for (int j = 0; j < mesh->instanceCount; j++)
        {
            LevelMeshInstance *instance = &mesh->instances[j];
            cJSON_AddItemToArray(instancesPX, cJSON_CreateNumber(instance->position.x));
            cJSON_AddItemToArray(instancesPY, cJSON_CreateNumber(instance->position.y));
            cJSON_AddItemToArray(instancesPZ, cJSON_CreateNumber(instance->position.z));
            cJSON_AddItemToArray(instancesRX, cJSON_CreateNumber(instance->eulerRotationDeg.x));
            cJSON_AddItemToArray(instancesRY, cJSON_CreateNumber(instance->eulerRotationDeg.y));
            cJSON_AddItemToArray(instancesRZ, cJSON_CreateNumber(instance->eulerRotationDeg.z));
            cJSON_AddItemToArray(instancesSX, cJSON_CreateNumber(instance->scale.x));
            cJSON_AddItemToArray(instancesSY, cJSON_CreateNumber(instance->scale.y));
            cJSON_AddItemToArray(instancesSZ, cJSON_CreateNumber(instance->scale.z));
        }
    }

    char *data = cJSON_Print(root);
    cJSON_Delete(root);
    if (!DirectoryExists("resources/levels"))
        MakeDirectory("resources/levels");
    SaveFileData(TextFormat("resources/levels/%s.lvl", levelFile), data, strlen(data));
    free(data);

}


void Level_update(Level *level, float dt)
{

}

void Level_draw(Level *level)
{
    
    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];
        Material material = {0};
        material.shader = mesh->isDithered ? _modelDitherShader : _modelTexturedShader;
        MaterialMap maps[16];
        maps[MATERIAL_MAP_ALBEDO].texture = 
            mesh->textureIndex >= 0 ? level->textures[mesh->textureIndex].texture : (Texture2D) {0};
        material.maps = maps;
        
        for (int j = 0; j < mesh->instanceCount; j++)
        {
            // TraceLog(LOG_INFO, "Drawing mesh: %s", mesh->filename);
            LevelMeshInstance *instance = &mesh->instances[j];
            DrawMesh(mesh->model.meshes[0], material, instance->toWorldTransform);
        }
    }
}

void Level_unload(Level *level)
{

    for (int i = 0; i < level->textureCount; i++)
    {
        UnloadTexture(level->textures[i].texture);
        free(level->textures[i].filename);
    }
    free(level->textures);
    level->textures = NULL;
    level->textureCount = 0;

    for (int i = 0; i < level->meshCount; i++)
    {
        UnloadModel(level->meshes[i].model);
        free(level->meshes[i].filename);
        free(level->meshes[i].instances);
    }
    free(level->meshes);

    level->meshes = NULL;
    level->meshCount = 0;
}
