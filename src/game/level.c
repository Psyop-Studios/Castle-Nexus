#include "level.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "cjson.h"
#include <raymath.h>
#include "main.h"
#include <stdio.h>
#include "level_components.h"

void Level_init(Level *level)
{
    level->meshCount = 0;
    level->meshes = NULL;
    level->textureCount = 0;
    level->textures = NULL;
    level->entityCount = 0;
    level->entities = NULL;
    level->entityComponentClassCount = 0;
    level->entityComponentClasses = NULL;
    level->colliderCount = 0;
    level->colliders = NULL;
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
            SetTextureWrap(level->textures[textureCount].texture, TEXTURE_WRAP_REPEAT);
            SetTextureFilter(level->textures[textureCount].texture, TEXTURE_FILTER_POINT);
            level->textures[textureCount].filename = replacePathSeps(strdup(file));
            level->textures[textureCount].index = textureCount;

            // load meta file
            char metaFileName[1024];
            strcpy(metaFileName, file);
            strcat(metaFileName, ".meta");
            if (FileExists(metaFileName))
            {
                TraceLog(LOG_INFO, "Loading meta file: %s", metaFileName);
                char *data = LoadFileText(metaFileName);
                cJSON *root = cJSON_Parse(data);



                cJSON *animations = cJSON_GetObjectItem(root, "animations");
                if (cJSON_IsArray(animations))
                {
                    LevelTexture* texture = &level->textures[textureCount];
                    texture->animationCount = cJSON_GetArraySize(animations);
                    texture->animations = (LevelTextureSpriteAnimation*)malloc(texture->animationCount * sizeof(LevelTextureSpriteAnimation));
                    for (int i = 0; i < texture->animationCount; i++)
                    {
                        cJSON *anim = cJSON_GetArrayItem(animations, i);
                        cJSON *name = cJSON_GetObjectItem(anim, "name");
                        cJSON *frameCount = cJSON_GetObjectItem(anim, "frameCount");
                        cJSON *frameRate = cJSON_GetObjectItem(anim, "frameRate");
                        cJSON *offsetX = cJSON_GetObjectItem(anim, "offsetX");
                        cJSON *offsetY = cJSON_GetObjectItem(anim, "offsetY");
                        cJSON *frameWidth = cJSON_GetObjectItem(anim, "frameWidth");
                        cJSON *frameHeight = cJSON_GetObjectItem(anim, "frameHeight");

                        if (name && name->type == cJSON_String &&
                            frameCount && frameCount->type == cJSON_Number &&
                            frameRate && frameRate->type == cJSON_Number &&
                            offsetX && offsetX->type == cJSON_Number &&
                            offsetY && offsetY->type == cJSON_Number &&
                            frameWidth && frameWidth->type == cJSON_Number &&
                            frameHeight && frameHeight->type == cJSON_Number)
                        {
                            TraceLog(LOG_INFO, "Animation: %s", name->valuestring);
                            texture->animations[i].name = strdup(name->valuestring);
                            texture->animations[i].frameCount = frameCount->valueint;
                            texture->animations[i].frameRate = frameRate->valuedouble;
                            texture->animations[i].offset = (Vector2){offsetX->valuedouble, offsetY->valuedouble};
                            texture->animations[i].frameSize = (Vector2){frameWidth->valuedouble, frameHeight->valuedouble};
                        }
                        else
                        {
                            TraceLog(LOG_WARNING, "Invalid animation data in meta file: %s", metaFileName);
                            texture->animations[i].name = "<INVALID>";
                            texture->animations[i].frameCount = 1;
                            texture->animations[i].frameRate = 1;
                            texture->animations[i].offset = (Vector2){0, 0};
                            texture->animations[i].frameSize = (Vector2){256, 256};
                        }
                    }
                }

                cJSON_Delete(root);
                UnloadFileText(data);
            } else {
                TraceLog(LOG_INFO, "No meta file found for: %s", file);
            }

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
            LevelMesh *mesh = &level->meshes[meshIndex++];
            mesh->colliderCount = 0;
            mesh->colliders = NULL;

            if (FileExists(metaFileName))
            {
                TraceLog(LOG_INFO, "Loading meta file: %s", metaFileName);
                char *data = LoadFileText(metaFileName);
                cJSON *root = cJSON_Parse(data);
                cJSON *textureName = cJSON_GetObjectItem(root, "texture");
                cJSON *dithered = cJSON_GetObjectItem(root, "dithered");
                if (textureName && textureName->type == cJSON_String)
                {
                    mesh->textureIndex = Level_getTextureIndexByName(level, textureName->valuestring);
                }
                if (dithered && dithered->type == cJSON_True)
                {
                    mesh->isDithered = 1;
                }

                cJSON *colliders = cJSON_GetObjectItem(root, "colliders");
                if (colliders && cJSON_IsArray(colliders))
                {
                    mesh->colliderCount = cJSON_GetArraySize(colliders);
                    mesh->colliders = (LevelCollider*)malloc(mesh->colliderCount * sizeof(LevelCollider));
                    for (int i = 0; i < mesh->colliderCount; i++)
                    {
                        cJSON *collider = cJSON_GetArrayItem(colliders, i);
                        cJSON *position = cJSON_GetObjectItem(collider, "position");
                        if (position && position->type == cJSON_Array &&
                            cJSON_GetArraySize(position) == 3)
                        {
                            mesh->colliders[i].position = (Vector3){
                                cJSON_GetArrayItem(position, 0)->valuedouble,
                                cJSON_GetArrayItem(position, 1)->valuedouble,
                                cJSON_GetArrayItem(position, 2)->valuedouble,
                            };
                        }
                        else
                        {
                            TraceLog(LOG_WARNING, "Invalid collider position data in meta file: %s", metaFileName);
                            mesh->colliders[i] = (LevelCollider){ 0 };
                            continue;
                        }
                        cJSON *isTrigger = cJSON_GetObjectItem(collider, "isTrigger");
                        mesh->colliders[i].isTrigger = isTrigger && isTrigger->type == cJSON_True;
                        cJSON *type = cJSON_GetObjectItem(collider, "type");
                        if (!cJSON_IsString(type))
                        {
                            TraceLog(LOG_WARNING, "Invalid collider type data in meta file: %s", metaFileName);
                            mesh->colliders[i] = (LevelCollider){ 0 };
                            continue;
                        }

                        if (strcmp(type->valuestring, "sphere") == 0)
                        {
                            mesh->colliders[i].type = LEVEL_COLLIDER_TYPE_SPHERE;
                            cJSON *radius = cJSON_GetObjectItem(collider, "radius");
                            if (radius && radius->type == cJSON_Number)
                            {
                                mesh->colliders[i].sphere.radius = radius->valuedouble;
                            }
                            else
                            {
                                TraceLog(LOG_WARNING, "Invalid collider radius data in meta file: %s", metaFileName);
                                mesh->colliders[i].sphere.radius = 1;
                            }
                        }
                        else if (strcmp(type->valuestring, "aabox") == 0)
                        {
                            mesh->colliders[i].type = LEVEL_COLLIDER_TYPE_AABOX;
                            cJSON *size = cJSON_GetObjectItem(collider, "size");
                            if (size && size->type == cJSON_Array &&
                                cJSON_GetArraySize(size) == 3)
                            {
                                mesh->colliders[i].aabox.size = (Vector3){
                                    cJSON_GetArrayItem(size, 0)->valuedouble,
                                    cJSON_GetArrayItem(size, 1)->valuedouble,
                                    cJSON_GetArrayItem(size, 2)->valuedouble,
                                };
                            }
                            else
                            {
                                TraceLog(LOG_WARNING, "Invalid collider size data in meta file: %s", metaFileName);
                                mesh->colliders[i].aabox.size = (Vector3){1, 1, 1};
                            }
                        }
                        else
                        {
                            TraceLog(LOG_WARNING, "Invalid collider type (%s) in meta file: %s", type->valuestring, metaFileName);
                            mesh->colliders[i].type = LEVEL_COLLIDER_TYPE_SPHERE;
                            mesh->colliders[i].position = (Vector3){0};
                            mesh->colliders[i].isTrigger = 0;
                        }
                    }
                }

                cJSON_Delete(root);
                UnloadFileText(data);
            } else {
                TraceLog(LOG_INFO, "No meta file found for: %s", file);
                mesh->textureIndex = -1;
            }

            TraceLog(LOG_INFO, "Loading mesh: %s", file);
            mesh->model = LoadModel(file);
            mesh->filename = replacePathSeps(strdup(file));
            mesh->instances = NULL;
            mesh->instanceCount = 0;
        }
    }
}

void Level_clearInstances(Level *level)
{
    if (level->filename)
    {
        free(level->filename);
        level->filename = NULL;
    }
    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        LevelEntityComponentClass *componentClass = &level->entityComponentClasses[i];
        if (componentClass->methods.onDestroyFn)
        {
            for (int j = 0; j < componentClass->instanceCount; j++)
            {
                LevelEntityInstanceId ownerId = componentClass->ownerIds[j];
                char *componentInstanceData = (char*) componentClass->componentInstanceData + j * componentClass->componentInstanceDataSize;
                componentClass->methods.onDestroyFn(level, ownerId, componentInstanceData);
                componentClass->generations[j] = 0;
            }
        }
        else
        {
            for (int j = 0; j < componentClass->instanceCount; j++)
            {
                componentClass->generations[j] = 0;
            }
        }
    }

    for (int i = 0; i < level->entityCount; i++)
    {
        free(level->entities[i].name);
        level->entities[i].name = NULL;
    }
    level->entityCount = 0;

    for (int i = 0; i < level->meshCount; i++)
    {
        free(level->meshes[i].instances);
        level->meshes[i].instances = NULL;
        level->meshes[i].instanceCount = 0;
    }
    
    level->colliderCount = 0;
}

LevelCollider* Level_addCollider(Level *level, uint8_t type, Vector3 position, int isTrigger)
{
    level->colliderCount++;
    level->colliders = (LevelCollider*)realloc(level->colliders, level->colliderCount * sizeof(LevelCollider));
    LevelCollider *collider = &level->colliders[level->colliderCount - 1];
    *collider = (LevelCollider){
        .type = type,
        .position = position,
        .isTrigger = isTrigger,
        .id = level->colliderCount,
    };
    return collider;
}

void Level_addColliderSphere(Level *level, Vector3 position, float radius, int isTrigger)
{
    LevelCollider *collider = Level_addCollider(level, LEVEL_COLLIDER_TYPE_SPHERE, position, isTrigger);
    collider->sphere.radius = radius;
}

void Level_addColliderBox(Level *level, Vector3 position, Vector3 size, int isTrigger)
{
    LevelCollider *collider = Level_addCollider(level, LEVEL_COLLIDER_TYPE_AABOX, position, isTrigger);
    collider->aabox.size = size;
}

Vector3 AABB_closestPoint(Vector3 min, Vector3 max, Vector3 point, Vector3 *normal)
{
    Vector3 result = point;
    int matchedXYZ = 0;
    if (point.x < min.x)
    {
        matchedXYZ |= 1;
        result.x = min.x;
    }
    if (point.y < min.y)
    {
        matchedXYZ |= 2;
        result.y = min.y;
    }
    if (point.z < min.z)
    {
        matchedXYZ |= 4;
        result.z = min.z;
    }
    if (point.x > max.x)
    {
        matchedXYZ |= 1;
        result.x = max.x;
    }
    if (point.y > max.y)
    {
        matchedXYZ |= 2;
        result.y = max.y;
    }
    if (point.z > max.z)
    {
        matchedXYZ |= 4;
        result.z = max.z;
    }
    if (normal)
    {
        *normal = (Vector3){0};
        int n = 0;
        if (matchedXYZ & 1)
        {
            normal->x = point.x < min.x ? -1 : 1;
            n++;
        }
        if (matchedXYZ & 2)
        {
            normal->y = point.y < min.y ? -1 : 1;
            n++;
        }
        if (matchedXYZ & 4)
        {
            normal->z = point.z < min.z ? -1 : 1;
            n++;
        }
        if (n > 1)
        {
            *normal = Vector3Normalize(*normal);
        }
    }
    return result;
}

int Level_testCollider(LevelCollider *collider, LevelCollisionResult *result, Vector3 position, float r)
{
    if (collider->type == LEVEL_COLLIDER_TYPE_SPHERE)
    {
        float dist = Vector3DistanceSqr(position, collider->position);
        float maxR2 = r + collider->sphere.radius;
        maxR2 *= maxR2;
        if (dist < maxR2)
        {
            Vector3 normal = Vector3Normalize(Vector3Subtract(position, collider->position));
            *result = (LevelCollisionResult){
                .colliderId = collider->id,
                .surfaceContact = Vector3Add(collider->position, Vector3Scale(normal, collider->sphere.radius)),
                .depth = collider->sphere.radius - sqrtf(dist),
                .normal = normal,
                .direction = Vector3Scale(normal, -1),
            };
            return 1;
        }
        return 0;
    }
    if (collider->type == LEVEL_COLLIDER_TYPE_AABOX)
    {
        Vector3 min = Vector3Subtract(collider->position, Vector3Scale(collider->aabox.size, 0.5f));
        Vector3 max = Vector3Add(collider->position, Vector3Scale(collider->aabox.size, 0.5f));
        Vector3 normal;
        Vector3 closest = AABB_closestPoint(min, max, position, &normal);
        // DrawSphereWires(position, r, 5, 5, DB8_RED);
        // DrawLine3D(position, closest, DB8_RED);
        // DrawCubeWires(collider->position, collider->aabox.size.x, collider->aabox.size.y, collider->aabox.size.z, DB8_RED);
        float dist = Vector3DistanceSqr(position, closest);
        if (dist < r * r)
        {
            Vector3 direction = Vector3Normalize(Vector3Subtract(position, closest));
            *result = (LevelCollisionResult){
                .colliderId = collider->id,
                .depth = r - sqrtf(dist),
                .surfaceContact = closest,
                .normal = normal,
                .direction = Vector3Scale(direction, -1),
            };
            return 1;
        }
        return 0;
    }
    return 0;
}

int Level_findCollisions(Level *level, Vector3 position, float radius, uint8_t matchNormal, uint8_t matchTrigger,
    LevelCollisionResult *results, int maxResults)
{
    int resultIndex = 0;
    for (int i = 0; i < level->colliderCount && resultIndex < maxResults; i++)
    {
        LevelCollider *collider = &level->colliders[i];
        if ((!matchNormal && collider->isTrigger) || (!matchTrigger && !collider->isTrigger))
        {
            continue;
        }

        LevelCollisionResult result;
        if (Level_testCollider(collider, &result, position, radius))
        {
            results[resultIndex++] = result;
        }
    }
    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];
        for (int j = 0; j < mesh->instanceCount && resultIndex < maxResults; j++)
        {
            LevelMeshInstance *instance = &mesh->instances[j];

            for (int k = 0; k < mesh->colliderCount && resultIndex < maxResults; k++)
            {
                LevelCollider *collider = &mesh->colliders[k];
                if ((matchNormal && collider->isTrigger) || (matchTrigger && !collider->isTrigger))
                {
                    continue;
                }
                Matrix invTransform = MatrixInvert(instance->toWorldTransform);
                Vector3 localPosition = Vector3Transform(position, invTransform);
                LevelCollisionResult result;
                if (Level_testCollider(collider, &result, localPosition, radius))
                {
                    Matrix rotTransform = instance->toWorldTransform;
                    rotTransform.m12 = 0;
                    rotTransform.m13 = 0;
                    rotTransform.m14 = 0;
                    result.surfaceContact = Vector3Transform(result.surfaceContact, instance->toWorldTransform);
                    result.direction = Vector3Transform(result.direction, rotTransform);
                    result.normal = Vector3Transform(result.normal, rotTransform);
                    results[resultIndex++] = result;
                }
            }
        }
    }

    LevelEntityComponentClass* boxColliderClass = &level->entityComponentClasses[COMPONENT_TYPE_COLLIDER_BOX];
    for (int i = 0; i < boxColliderClass->instanceCount && resultIndex < maxResults; i++)
    {
        if (boxColliderClass->generations[i] == 0)
        {
            continue;
        }
        LevelEntity *entity = Level_resolveEntity(level, boxColliderClass->ownerIds[i]);
        if (!entity)
        {
            continue;
        }

        ColliderBoxComponent *boxCollider = &((ColliderBoxComponent*)boxColliderClass->componentInstanceData)[i];
        Matrix inv = MatrixInvert(entity->toWorldTransform);
        Vector3 localPosition = Vector3Transform(position, inv);

        LevelCollisionResult result;
        if (Level_testCollider(&(LevelCollider){
            .type = LEVEL_COLLIDER_TYPE_AABOX,
            .position = boxCollider->offset,
            .aabox.size = boxCollider->size,
            .isTrigger = boxCollider->isTrigger,
        }, &result, localPosition, radius))
        {
            results[resultIndex++] = result;
        }
    }
        
    return resultIndex;
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


Texture2D Level_getTexture(Level *level, const char *filename, Texture2D fallback)
{
    for (int i = 0; i < level->textureCount; i++)
    {
        char *lastSlash = strrchr(level->textures[i].filename, '/');
        if (strcmp(level->textures[i].filename, filename) == 0 || (lastSlash && strcmp(lastSlash + 1, filename) == 0))
        {
            return level->textures[i].texture;
        }
    }
    return fallback;
}

LevelTexture* Level_getLevelTexture(Level *level, const char *filename)
{
    for (int i = 0; i < level->textureCount; i++)
    {
        char *lastSlash = strrchr(level->textures[i].filename, '/');
        if (strcmp(level->textures[i].filename, filename) == 0 || (lastSlash && strcmp(lastSlash + 1, filename) == 0))
        {
            return &level->textures[i];
        }
    }
    TraceLog(LOG_WARNING, "Texture not found: %s", filename);
    return NULL;
}

LevelMesh *Level_getMesh(Level *level, const char *filename)
{
    for (int i = 0; i < level->meshCount; i++)
    {
        char *lastSlash = strrchr(level->meshes[i].filename, '/');
        if (strcmp(level->meshes[i].filename, filename) == 0 || (lastSlash && strcmp(lastSlash + 1, filename) == 0))
        {
            return &level->meshes[i];
        }
    }
    return NULL;
}

LevelMeshInstance* Level_addInstance(Level *level, const char *meshName, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale)
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
            instance->textureIndex = -1;
            Level_updateInstanceTransform(instance);
            return instance;
        }
    }
    return NULL;
}

LevelEntityComponentClass* Level_getComponentClassByName(Level *level, const char *name)
{
    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        if (strcmp(level->entityComponentClasses[i].name, name) == 0)
        {
            return &level->entityComponentClasses[i];
        }
    }
    return NULL;
}

void Level_load(Level *level, const char *levelFile)
{
    if (strlen(levelFile) == 0)
    {
        TraceLog(LOG_ERROR, "Level file name is empty");
        return;
    }
    Level_clearInstances(level);
    level->filename = strdup(levelFile);
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
    int *entityMap = NULL;

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

        cJSON *instancesTexId = cJSON_GetObjectItem(meshObj, "instancesTexId");

        const char *meshName = filename->valuestring;

        for (int j = 0; j < cJSON_GetArraySize(instancesPX); j++)
        {
            Vector3 position = (Vector3){cJSON_GetArrayItem(instancesPX, j)->valuedouble, cJSON_GetArrayItem(instancesPY, j)->valuedouble, cJSON_GetArrayItem(instancesPZ, j)->valuedouble};
            Vector3 eulerRotationDeg = (Vector3){cJSON_GetArrayItem(instancesRX, j)->valuedouble, cJSON_GetArrayItem(instancesRY, j)->valuedouble, cJSON_GetArrayItem(instancesRZ, j)->valuedouble};
            Vector3 scale = (Vector3){cJSON_GetArrayItem(instancesSX, j)->valuedouble, cJSON_GetArrayItem(instancesSY, j)->valuedouble, cJSON_GetArrayItem(instancesSZ, j)->valuedouble};
            LevelMeshInstance *instance = Level_addInstance(level, meshName, position, eulerRotationDeg, scale);
            if (instance && instancesTexId)
            {
                instance->textureIndex = cJSON_GetArrayItem(instancesTexId, j)->valueint;
            }
        }
    }

    cJSON *entityNames = cJSON_GetObjectItem(root, "entityNames");
    cJSON *entityTRS = cJSON_GetObjectItem(root, "entityTRS");
    cJSON *entityIds = cJSON_GetObjectItem(root, "entityIds");
    cJSON *components = cJSON_GetObjectItem(root, "components");

    if (!entityNames || !entityTRS || !entityIds || !components)
    {
        goto cleanup;
    }

    int entityCount = cJSON_GetArraySize(entityNames);
    if (cJSON_GetArraySize(entityTRS) != entityCount * 9 || cJSON_GetArraySize(entityIds) != entityCount)
    {
        TraceLog(LOG_ERROR, "Entity data mismatch; not importing entity data: %s", levelFile);
        goto cleanup;
    }

    for (int i = 0; i < entityCount; i++)
    {
        cJSON *entityNameJSON = cJSON_GetArrayItem(entityNames, i);
        if (entityNameJSON->type != cJSON_String)
        {
            continue;
        }
        char *entityName = entityNameJSON->valuestring;
        cJSON *entityId = cJSON_GetArrayItem(entityIds, i);
        Vector3 position = (Vector3){
            (float)cJSON_GetArrayItem(entityTRS, i * 9)->valuedouble,
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 1)->valuedouble,
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 2)->valuedouble};
        Vector3 eulerRotationDeg = (Vector3){
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 3)->valuedouble,
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 4)->valuedouble,
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 5)->valuedouble};
        Vector3 scale = (Vector3){
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 6)->valuedouble,
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 7)->valuedouble,
            (float)cJSON_GetArrayItem(entityTRS, i * 9 + 8)->valuedouble};
        LevelEntity *entity = Level_addEntityAtIndex(level, entityId->valueint, entityName, position, eulerRotationDeg, scale);
        if (!entity)
        {
            TraceLog(LOG_ERROR, "Failed to add entity at %d: %s", entityId->valueint, entityName);
            continue;
        }
        if (entity->id != entityId->valueint)
        {
            TraceLog(LOG_ERROR, "Entity id mismatch at %d (%d vs %d); not importing entity data: %s",
                i, entity->id, entityId->valueint, levelFile);
            goto cleanup;
        }
    }

    int componentCount = cJSON_GetArraySize(components);
    for (int i = 0; i < componentCount; i+=1)
    {
        cJSON *componentClass = cJSON_GetArrayItem(components, i);
        cJSON *componentName = cJSON_GetObjectItem(componentClass, "name");
        cJSON *componentId = cJSON_GetObjectItem(componentClass, "componentId");
        cJSON *ownerEntityIds = cJSON_GetObjectItem(componentClass, "ownerEntityIds");
        cJSON *entityData = cJSON_GetObjectItem(componentClass, "entityData");
        if (!componentName || !componentId || !ownerEntityIds || !entityData || !cJSON_IsArray(ownerEntityIds) || !cJSON_IsArray(entityData) || !cJSON_IsString(componentName) || !cJSON_IsNumber(componentId))
        {
            TraceLog(LOG_ERROR, "Invalid component class in level file: %s", levelFile);
            continue;
        }

        if (cJSON_GetArraySize(ownerEntityIds) != cJSON_GetArraySize(entityData))
        {
            TraceLog(LOG_ERROR, "Component data mismatch; not importing component data: %s", levelFile);
            continue;
        }

        LevelEntityComponentClass *componentClassEntry = Level_getComponentClassById(level, componentId->valueint);
        char *componentNameStr = componentName->valuestring;
        if (strcmp(componentClassEntry->name, componentNameStr) != 0)
        {
            TraceLog(LOG_WARNING, "Component class name mismatch (%s vs %s); looking up id", componentClassEntry->name, componentNameStr);
            componentClassEntry = Level_getComponentClassByName(level, componentNameStr);
        }
        if (!componentClassEntry)
        {
            TraceLog(LOG_ERROR, "Component class not found; not importing component data: %s", levelFile);
            continue;
        }

        for (int j = 0; j < cJSON_GetArraySize(ownerEntityIds); j++)
        {
            cJSON *entityDataItem = cJSON_GetArrayItem(entityData, j);
            if (cJSON_IsNull(entityDataItem))
            {
                continue;
            }
            cJSON *ownerEntityId = cJSON_GetArrayItem(ownerEntityIds, j);
            if (!cJSON_IsNumber(ownerEntityId) || !cJSON_IsObject(entityDataItem))
            {
                TraceLog(LOG_ERROR, "Invalid component data in level file: %s", levelFile);
                continue;
            }

            LevelEntity* ownerEntity = &level->entities[ownerEntityId->valueint];
            void *componentInstanceData = NULL;
            LevelEntityComponentInstanceId componentInstanceid = Level_addEntityComponentAtIndex(level, j, ownerEntity, componentClassEntry->componentId, &componentInstanceData);
            if (componentInstanceid.id != j || componentInstanceid.generation == 0)
            {
                TraceLog(LOG_ERROR, "Component instance id mismatch (%d vs %d); "
                    "not importing component data: %s", componentInstanceid.id, j, levelFile);
                continue;
            }

            if (componentClassEntry->methods.onDeserializeFn)
            {
                componentClassEntry->methods.onDeserializeFn(level, (LevelEntityInstanceId){ownerEntity->id, ownerEntity->generation}, componentInstanceData, entityDataItem);
            }
        }
    }

    cleanup:
    cJSON_Delete(root);
    UnloadFileText(data);
    if (entityMap)
    {
        free(entityMap);
    }
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
        cJSON *instancesTexId = cJSON_CreateArray();
        cJSON_AddItemToObject(meshObj, "instancesPX", instancesPX);
        cJSON_AddItemToObject(meshObj, "instancesPY", instancesPY);
        cJSON_AddItemToObject(meshObj, "instancesPZ", instancesPZ);
        cJSON_AddItemToObject(meshObj, "instancesRX", instancesRX);
        cJSON_AddItemToObject(meshObj, "instancesRY", instancesRY);
        cJSON_AddItemToObject(meshObj, "instancesRZ", instancesRZ);
        cJSON_AddItemToObject(meshObj, "instancesSX", instancesSX);
        cJSON_AddItemToObject(meshObj, "instancesSY", instancesSY);
        cJSON_AddItemToObject(meshObj, "instancesSZ", instancesSZ);
        cJSON_AddItemToObject(meshObj, "instancesTexId", instancesTexId);

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
            cJSON_AddItemToArray(instancesTexId, cJSON_CreateNumber(instance->textureIndex));
        }
    }

    cJSON *entityNames = cJSON_CreateArray();
    cJSON *entityTRS = cJSON_CreateArray();
    cJSON *entityIds = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "entityNames", entityNames);
    cJSON_AddItemToObject(root, "entityTRS", entityTRS);
    cJSON_AddItemToObject(root, "entityIds", entityIds);

    for (int i = 0; i < level->entityCount; i++)
    {
        LevelEntity *entity = &level->entities[i];
        if (!entity->name)
        {
            cJSON_AddItemToArray(entityNames, cJSON_CreateNull());
            cJSON_AddItemToArray(entityIds, cJSON_CreateNumber(0));
            for (int i = 0; i < 9; i++)
            {
                cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(0));
            }
            continue;
        }

        cJSON_AddItemToArray(entityNames, cJSON_CreateString(entity->name));
        cJSON_AddItemToArray(entityIds, cJSON_CreateNumber(entity->id));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->position.x));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->position.y));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->position.z));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->eulerRotationDeg.x));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->eulerRotationDeg.y));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->eulerRotationDeg.z));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->scale.x));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->scale.y));
        cJSON_AddItemToArray(entityTRS, cJSON_CreateNumber(entity->scale.z));
    }

    cJSON *components = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "components", components);

    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        LevelEntityComponentClass *componentClassEntry = &level->entityComponentClasses[i];
        cJSON *componentClass = cJSON_CreateObject();
        cJSON_AddItemToArray(components, componentClass);
        cJSON_AddStringToObject(componentClass, "name", componentClassEntry->name);
        cJSON_AddNumberToObject(componentClass, "componentId", componentClassEntry->componentId);
        cJSON *ownerEntityIds = cJSON_CreateArray();
        cJSON *entityData = cJSON_CreateArray();
        cJSON_AddItemToObject(componentClass, "ownerEntityIds", ownerEntityIds);
        cJSON_AddItemToObject(componentClass, "entityData", entityData);
        for (int j = 0; j < componentClassEntry->instanceCount; j++)
        {
            if (componentClassEntry->generations[j] == 0 || Level_resolveEntity(level, componentClassEntry->ownerIds[j]) == NULL)
            {
                cJSON_AddItemToArray(ownerEntityIds, cJSON_CreateNumber(0));
                cJSON_AddItemToArray(entityData, cJSON_CreateNull());
                continue;
            }
            LevelEntityInstanceId ownerId = componentClassEntry->ownerIds[j];
            void *componentInstanceData = (char*) componentClassEntry->componentInstanceData + j * componentClassEntry->componentInstanceDataSize;
            cJSON *entityDataObj = cJSON_CreateObject();
            cJSON_AddItemToArray(ownerEntityIds, cJSON_CreateNumber(ownerId.id));
            cJSON_AddItemToArray(entityData, entityDataObj);
            if (componentClassEntry->methods.onSerializeFn)
            {
                componentClassEntry->methods.onSerializeFn(level, ownerId, componentInstanceData, entityDataObj);
            }
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

LevelCollisionResult Level_calcPenetrationDepth(Level *level, Vector3 point, float radius)
{
    LevelCollisionResult result = {0};

    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];

        for (int j = 0; j < mesh->instanceCount; j++)
        {
            LevelMeshInstance *instance = &mesh->instances[j];
            Vector3 localPoint = Vector3Transform(point, MatrixInvert(instance->toWorldTransform));
            float distanceToMesh = Vector3Length(localPoint);

            if (distanceToMesh < radius)
            {
                float depth = radius - distanceToMesh;
                if (depth > result.depth)
                {
                    result.depth = depth;
                    result.direction = Vector3Subtract(localPoint, point);
                    result.direction = Vector3Normalize(result.direction);
                }
            }

        }

    }

    return result;
}


void Level_draw(Level *level)
{
    level->renderTime += GetFrameTime();
    int locTexSize = GetShaderLocation(_modelDitherShader, "texSize");
    int locUvDitherBlockPosScale = GetShaderLocation(_modelDitherShader, "uvDitherBlockPosScale");

    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];
        Material material = {0};
        material.shader = mesh->isDithered ? _modelDitherShader : _modelTexturedShader;
        MaterialMap maps[16];
        Texture2D defaultTex = maps[MATERIAL_MAP_ALBEDO].texture =
            mesh->textureIndex >= 0 ? level->textures[mesh->textureIndex].texture : (Texture2D) {0};
        material.maps = maps;

        for (int j = 0; j < mesh->instanceCount; j++)
        {
            // TraceLog(LOG_INFO, "Drawing mesh: %s", mesh->filename);
            LevelMeshInstance *instance = &mesh->instances[j];
            if (instance->textureIndex >= 0)
            {
                material.maps[MATERIAL_MAP_ALBEDO].texture = level->textures[instance->textureIndex].texture;
            }
            else
            {
                material.maps[MATERIAL_MAP_ALBEDO].texture = defaultTex;
            }
            Vector2 texSize = {material.maps[MATERIAL_MAP_ALBEDO].texture.width, material.maps[MATERIAL_MAP_ALBEDO].texture.height};
            SetShaderValue(material.shader, locTexSize, &texSize, SHADER_UNIFORM_VEC2);
            SetShaderValue(material.shader, locUvDitherBlockPosScale, (float[1]){texSize.x / 8.0f}, SHADER_UNIFORM_FLOAT);
            Game_setFogTextures(&material);
            DrawMesh(mesh->model.meshes[0], material, instance->toWorldTransform);
        }
    }

    for (int i = 0; i < level->entityCount; i++)
    {
        LevelEntity *entity = &level->entities[i];
        if (!entity->name)
        {
            continue;
        }
        if (entity->transformIsDirty)
        {
            Level_updateEntityTransform(entity);
        }
    }

    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        LevelEntityComponentClass *componentClass = &level->entityComponentClasses[i];
        if (componentClass->methods.drawFn)
        {
            for (int j = 0; j < componentClass->instanceCount; j++)
            {
                if (componentClass->generations[j] == 0 || Level_resolveEntity(level, componentClass->ownerIds[j]) == NULL)
                {
                    continue;
                }
                LevelEntityInstanceId ownerId = componentClass->ownerIds[j];
                void *componentInstanceData = (char*) componentClass->componentInstanceData + j * componentClass->componentInstanceDataSize;
                componentClass->methods.drawFn(level, ownerId, componentInstanceData);
            }
        }
    }
}

void Level_unload(Level *level)
{
    for (int i = 0; i < level->entityCount; i++)
    {
        if (level->entities[i].name)
        {
            free(level->entities[i].name);
        }
    }
    free(level->entities);
    level->entities = NULL;
    level->entityCount = 0;

    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        LevelEntityComponentClass *componentClass = &level->entityComponentClasses[i];
        if (componentClass->methods.onDestroyFn)
        {
            for (int j = 0; j < componentClass->instanceCount; j++)
            {
                componentClass->methods.onDestroyFn(level,
                    (LevelEntityInstanceId){0}, (void*)((char*)componentClass->componentInstanceData + j * componentClass->componentInstanceDataSize));
            }
        }
        free(level->entityComponentClasses[i].name);
        free(level->entityComponentClasses[i].componentInstanceData);
        free(level->entityComponentClasses[i].generations);
        free(level->entityComponentClasses[i].ownerIds);
    }

    free(level->entityComponentClasses);
    level->entityComponentClasses = NULL;
    level->entityComponentClassCount = 0;

    for (int i = 0; i < level->textureCount; i++)
    {
        UnloadTexture(level->textures[i].texture);
        free(level->textures[i].filename);

        if (level->textures[i].animationCount > 0)
        {
            for (int j = 0; j < level->textures[i].animationCount; j++)
            {
                free(level->textures[i].animations[j].name);
            }
            free(level->textures[i].animations);
        }
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

LevelEntityComponentClass* Level_getComponentClassById(Level *level, uint32_t componentId)
{
    if (componentId >= level->entityComponentClassCount)
    {
        return NULL;
    }
    return &level->entityComponentClasses[componentId];
}

LevelEntity* Level_resolveEntity(Level *level, LevelEntityInstanceId id)
{
    if (id.id >= level->entityCount)
    {
        return NULL;
    }
    LevelEntity *entity = &level->entities[id.id];
    if (entity->generation != id.generation || !entity->name)
    {
        return NULL;
    }
    return entity;
}

void* Level_resolveComponent(Level *level, LevelEntityComponentInstanceId id)
{
    LevelEntityComponentClass *componentClass = Level_getComponentClassById(level, id.componentId);
    if (!componentClass)
    {
        TraceLog(LOG_ERROR, "Component class not found: %d, this could be serious!", id.componentId);
        return NULL;
    }
    if (id.id >= componentClass->instanceCount)
    {
        return NULL;
    }
    if (componentClass->generations[id.id] != id.generation)
    {
        return NULL;
    }
    LevelEntityInstanceId ownerId = componentClass->ownerIds[id.id];
    LevelEntity *owner = Level_resolveEntity(level, ownerId);
    if (!owner)
    {
        return NULL;
    }
    return (void*)((char*)componentClass->componentInstanceData + id.id * componentClass->componentInstanceDataSize);
}

void Level_registerEntityComponentClass(Level *level, uint32_t componentId, const char *name, LevelEntityComponentClassMethods methods, int componentInstanceDataSize)
{
    if (componentId < level->entityComponentClassCount && level->entityComponentClasses[componentId].componentId == componentId)
    {
        TraceLog(LOG_ERROR, "Component class (%s vs %s) already registered: %d", name, level->entityComponentClasses[componentId].name, componentId);
        return;
    }
    if (componentInstanceDataSize < 4) componentInstanceDataSize = 4;
    if (componentId >= level->entityComponentClassCount)
    {
        int requiredSize = componentId + 1;
        LevelEntityComponentClass *newClasses = (LevelEntityComponentClass*)malloc(requiredSize * sizeof(LevelEntityComponentClass));
        memset(newClasses, 0, requiredSize * sizeof(LevelEntityComponentClass));
        for (int i = 0; i < level->entityComponentClassCount; i++)
        {
            if (level->entityComponentClasses[i].name)
                newClasses[i] = level->entityComponentClasses[i];
        }
        free(level->entityComponentClasses);
        level->entityComponentClasses = newClasses;
        level->entityComponentClassCount = requiredSize;
    }
    LevelEntityComponentClass *componentClass = &level->entityComponentClasses[componentId];
    componentClass->componentId = componentId;
    componentClass->name = strdup(name);
    componentClass->methods = methods;
    componentClass->componentInstanceDataSize = componentInstanceDataSize;
    componentClass->componentInstanceData = NULL;
    componentClass->generations = NULL;
    componentClass->ownerIds = NULL;
    componentClass->instanceCount = 0;
}

void Level_updateEntityTransform(LevelEntity *entity)
{
    entity->transformIsDirty = 0;
    entity->toWorldTransform = MatrixRotateXYZ((Vector3){DEG2RAD * entity->eulerRotationDeg.x, DEG2RAD * entity->eulerRotationDeg.y, DEG2RAD * entity->eulerRotationDeg.z});
    entity->toWorldTransform = MatrixMultiply(entity->toWorldTransform, MatrixScale(entity->scale.x, entity->scale.y, entity->scale.z));
    entity->toWorldTransform = MatrixMultiply(entity->toWorldTransform, MatrixTranslate(entity->position.x, entity->position.y, entity->position.z));
}

// produces a new entity using the prefab data
LevelEntity* Level_instantiatePrefab(Level *level, cJSON *json)
{
    cJSON *name = cJSON_GetObjectItem(json, "name");
    cJSON *id = cJSON_GetObjectItem(json, "id");
    cJSON *x = cJSON_GetObjectItem(json, "x");
    cJSON *y = cJSON_GetObjectItem(json, "y");
    cJSON *z = cJSON_GetObjectItem(json, "z");
    cJSON *rx = cJSON_GetObjectItem(json, "rx");
    cJSON *ry = cJSON_GetObjectItem(json, "ry");
    cJSON *rz = cJSON_GetObjectItem(json, "rz");
    cJSON *sx = cJSON_GetObjectItem(json, "sx");
    cJSON *sy = cJSON_GetObjectItem(json, "sy");
    cJSON *sz = cJSON_GetObjectItem(json, "sz");
    cJSON *components = cJSON_GetObjectItem(json, "components");

    if (!name || !id || !x || !y || !z || !rx || !ry || !rz || !sx || !sy || !sz || !components)
    {
        return NULL;
    }

    LevelEntity *entity = Level_addEntity(level, name->valuestring,
        (Vector3){(float) x->valuedouble, (float) y->valuedouble, (float) z->valuedouble},
        (Vector3){(float) rx->valuedouble, (float) ry->valuedouble, (float) rz->valuedouble},
        (Vector3){(float) sx->valuedouble, (float) sy->valuedouble, (float) sz->valuedouble});
    if (!entity)
    {
        return NULL;
    }

    for (int i = 0; i < cJSON_GetArraySize(components); i++)
    {
        cJSON *component = cJSON_GetArrayItem(components, i);
        cJSON *componentId = cJSON_GetObjectItem(component, "componentId");
        cJSON *componentName = cJSON_GetObjectItem(component, "name");
        cJSON *data = cJSON_GetObjectItem(component, "data");
        if (!componentId || !componentName || !data)
        {
            continue;
        }
        LevelEntityComponentClass *componentClass = Level_getComponentClassByName(level, componentName->valuestring);
        if (!componentClass)
        {
            continue;
        }

        void *componentInstanceData = NULL;
        LevelEntityComponentInstanceId componentInstanceId = Level_addEntityComponent(level, entity, componentClass->componentId, &componentInstanceData);
        if (componentInstanceId.generation == 0)
        {
            continue;
        }
        if (componentClass->methods.onDeserializeFn)
        {
            componentClass->methods.onDeserializeFn(level, (LevelEntityInstanceId){entity->id, entity->generation}, componentInstanceData, data);
        }
    }

    return entity;
}

// a json object representing an entity in a way so it can be used to copy the entity
cJSON* Level_serializeEntityAsPrefab(Level *level, LevelEntityInstanceId instanceId)
{
    LevelEntity *entity = Level_resolveEntity(level, instanceId);
    if (!entity)
    {
        return NULL;
    }

    cJSON *entityData = cJSON_CreateObject();
    cJSON_AddStringToObject(entityData, "name", entity->name);
    cJSON_AddNumberToObject(entityData, "id", entity->id);
    cJSON_AddNumberToObject(entityData, "x", entity->position.x);
    cJSON_AddNumberToObject(entityData, "y", entity->position.y);
    cJSON_AddNumberToObject(entityData, "z", entity->position.z);
    cJSON_AddNumberToObject(entityData, "rx", entity->eulerRotationDeg.x);
    cJSON_AddNumberToObject(entityData, "ry", entity->eulerRotationDeg.y);
    cJSON_AddNumberToObject(entityData, "rz", entity->eulerRotationDeg.z);
    cJSON_AddNumberToObject(entityData, "sx", entity->scale.x);
    cJSON_AddNumberToObject(entityData, "sy", entity->scale.y);
    cJSON_AddNumberToObject(entityData, "sz", entity->scale.z);

    cJSON *components = cJSON_CreateArray();
    cJSON_AddItemToObject(entityData, "components", components);

    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        LevelEntityComponentClass *componentClass = &level->entityComponentClasses[i];
        for (int j = 0; j < componentClass->instanceCount; j++)
        {
            if (componentClass->generations[j] == 0 || componentClass->ownerIds[j].id != entity->id || componentClass->ownerIds[j].generation != entity->generation)
            {
                continue;
            }
            cJSON *componentData = cJSON_CreateObject();
            cJSON_AddItemToArray(components, componentData);
            cJSON_AddNumberToObject(componentData, "componentId", componentClass->componentId);
            cJSON_AddStringToObject(componentData, "name", componentClass->name);
            cJSON *data = cJSON_CreateObject();
            cJSON_AddItemToObject(componentData, "data", data);
            if (componentClass->methods.onSerializeFn)
            {
                void *componentInstanceData = (char*) componentClass->componentInstanceData + j * componentClass->componentInstanceDataSize;
                componentClass->methods.onSerializeFn(level, componentClass->ownerIds[j], componentInstanceData, data);
            }
        }
    }

    return entityData;
}

LevelEntity* Level_addEntity(Level *level, const char *name, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale)
{
    for (int i = 0; i < level->entityCount; i++)
    {
        if (!level->entities[i].name)
        {
            level->entities[i].name = strdup(name);
            level->entities[i].position = position;
            level->entities[i].eulerRotationDeg = eulerRotationDeg;
            level->entities[i].scale = scale;
            level->entities[i].generation++;
            Level_updateEntityTransform(&level->entities[i]);
            return &level->entities[i];
        }
    }

    int entityId = level->entityCount++;
    level->entities = (LevelEntity*)realloc(level->entities, level->entityCount * sizeof(LevelEntity));
    memset(&level->entities[entityId], 0, sizeof(LevelEntity));
    level->entities[entityId].name = strdup(name);
    level->entities[entityId].position = position;
    level->entities[entityId].eulerRotationDeg = eulerRotationDeg;
    level->entities[entityId].scale = scale;
    level->entities[entityId].id = entityId;
    level->entities[entityId].generation = 1;
    Level_updateEntityTransform(&level->entities[entityId]);
    return &level->entities[entityId];
}

// this is required for deserialization; don't use otherwise
LevelEntity* Level_addEntityAtIndex(Level *level, int index, const char *name, Vector3 position, Vector3 eulerRotationDeg, Vector3 scale)
{
    if (index < level->entityCount)
    {
        if (!level->entities[index].name)
        {
            level->entities[index].name = strdup(name);
            level->entities[index].position = position;
            level->entities[index].eulerRotationDeg = eulerRotationDeg;
            level->entities[index].scale = scale;
            level->entities[index].generation++;
            Level_updateEntityTransform(&level->entities[index]);
            return &level->entities[index];
        }
        else
        {
            TraceLog(LOG_ERROR, "Entity already exists at index: %d", index);
            return NULL;
        }
    }

    int newCount = index + 8;
    level->entities = (LevelEntity*)realloc(level->entities, newCount * sizeof(LevelEntity));
    memset(&level->entities[level->entityCount], 0, sizeof(LevelEntity) * (newCount - level->entityCount));
    for (int i = level->entityCount; i < newCount; i++)
    {
        level->entities[i].id = i;
    }
    level->entityCount = newCount;

    level->entities[index].name = strdup(name);
    level->entities[index].position = position;
    level->entities[index].eulerRotationDeg = eulerRotationDeg;
    level->entities[index].scale = scale;
    level->entities[index].id = index;
    level->entities[index].generation = 1;
    Level_updateEntityTransform(&level->entities[index]);
    return &level->entities[index];
}

// this is required for deserialization; don't use otherwise
LevelEntityComponentInstanceId Level_addEntityComponentAtIndex(Level *level, int index, LevelEntity *entity, uint32_t componentId,
    void **componentInstanceData)
{
    LevelEntityComponentClass *componentClass = Level_getComponentClassById(level, componentId);
    if (!componentClass)
    {
        TraceLog(LOG_ERROR, "Component class not found: %d", componentId);
        return (LevelEntityComponentInstanceId){0};
    }

    LevelEntityComponentInstanceId instanceId = {0};

    if (index < componentClass->instanceCount)
    {
        if (componentClass->generations[index] == 0 || Level_resolveEntity(level, componentClass->ownerIds[index]) == NULL)
        {
            componentClass->generations[index]++;
            componentClass->ownerIds[index] = (LevelEntityInstanceId){entity->id, entity->generation};
            instanceId.componentId = componentId;
            instanceId.id = index;
            instanceId.generation = componentClass->generations[index];
        }
        else
        {
            TraceLog(LOG_ERROR, "Component instance already exists at index: %d", index);
            return (LevelEntityComponentInstanceId){0};
        }
    }
    if (instanceId.generation == 0)
    {
        // increase pool size
        int newSize = (componentClass->instanceCount + 4) * 2;
        if (newSize < index + 1)
        {
            newSize = index + 1;
        }
        componentClass->componentInstanceData = realloc(componentClass->componentInstanceData, newSize * componentClass->componentInstanceDataSize);
        componentClass->generations = realloc(componentClass->generations, newSize * sizeof(uint32_t));
        componentClass->ownerIds = realloc(componentClass->ownerIds, newSize * sizeof(LevelEntityInstanceId));
        memset((char*)componentClass->componentInstanceData + componentClass->instanceCount * componentClass->componentInstanceDataSize, 0, (newSize - componentClass->instanceCount) * componentClass->componentInstanceDataSize);
        memset(componentClass->generations + componentClass->instanceCount, 0, (newSize - componentClass->instanceCount) * sizeof(uint32_t));
        memset(componentClass->ownerIds + componentClass->instanceCount, 0, (newSize - componentClass->instanceCount) * sizeof(LevelEntityInstanceId));


        componentClass->generations[index] = 1;
        componentClass->ownerIds[index] = (LevelEntityInstanceId){entity->id, entity->generation};
        instanceId.componentId = componentId;
        instanceId.id = index;
        instanceId.generation = 1;

        componentClass->instanceCount = newSize;
    }

    if (componentClass->methods.onInitFn)
    {
        void *componentInstanceData = (char*)componentClass->componentInstanceData + instanceId.id * componentClass->componentInstanceDataSize;
        componentClass->methods.onInitFn(level, (LevelEntityInstanceId){entity->id, entity->generation}, componentInstanceData);
    }
    if (componentInstanceData)
        *componentInstanceData = (void*)((char*)componentClass->componentInstanceData + instanceId.id * componentClass->componentInstanceDataSize);
    return instanceId;
}

LevelEntityComponentInstanceId Level_addEntityComponent(Level *level, LevelEntity *entity, uint32_t componentId,
    void **componentInstanceData)
{
    LevelEntityComponentClass *componentClass = Level_getComponentClassById(level, componentId);
    if (!componentClass)
    {
        TraceLog(LOG_ERROR, "Component class not found: %d", componentId);
        return (LevelEntityComponentInstanceId){0};
    }

    LevelEntityComponentInstanceId instanceId = {0};

    for (int i = 0; i < componentClass->instanceCount; i++)
    {
        if (componentClass->generations[i] == 0 || Level_resolveEntity(level, componentClass->ownerIds[i]) == NULL)
        {
            componentClass->generations[i]++;
            componentClass->ownerIds[i] = (LevelEntityInstanceId){entity->id, entity->generation};
            instanceId.componentId = componentId;
            instanceId.id = i;
            instanceId.generation = componentClass->generations[i];
            break;
        }
    }
    if (instanceId.generation == 0)
    {
        // increase pool size
        int newSize = (componentClass->instanceCount + 4) * 2;
        componentClass->componentInstanceData = realloc(componentClass->componentInstanceData, newSize * componentClass->componentInstanceDataSize);
        componentClass->generations = realloc(componentClass->generations, newSize * sizeof(uint32_t));
        componentClass->ownerIds = realloc(componentClass->ownerIds, newSize * sizeof(LevelEntityInstanceId));
        memset((char*)componentClass->componentInstanceData + componentClass->instanceCount * componentClass->componentInstanceDataSize, 0, (newSize - componentClass->instanceCount) * componentClass->componentInstanceDataSize);
        memset(componentClass->generations + componentClass->instanceCount, 0, (newSize - componentClass->instanceCount) * sizeof(uint32_t));
        memset(componentClass->ownerIds + componentClass->instanceCount, 0, (newSize - componentClass->instanceCount) * sizeof(LevelEntityInstanceId));
        componentClass->generations[componentClass->instanceCount] = 1;
        componentClass->ownerIds[componentClass->instanceCount] = (LevelEntityInstanceId){entity->id, entity->generation};
        instanceId.componentId = componentId;
        instanceId.id = componentClass->instanceCount;
        instanceId.generation = 1;
        componentClass->instanceCount = newSize;
    }

    if (componentClass->methods.onInitFn)
    {
        void *componentInstanceData = (char*)componentClass->componentInstanceData + instanceId.id * componentClass->componentInstanceDataSize;
        componentClass->methods.onInitFn(level, (LevelEntityInstanceId){entity->id, entity->generation}, componentInstanceData);
    }
    if (componentInstanceData)
        *componentInstanceData = (void*)((char*)componentClass->componentInstanceData + instanceId.id * componentClass->componentInstanceDataSize);
    return instanceId;
}

void Level_deleteEntity(Level *level, LevelEntity *entity)
{
    if (!entity->name)
    {
        return;
    }
    free(entity->name);
    entity->name = NULL;
    entity->generation++;
}