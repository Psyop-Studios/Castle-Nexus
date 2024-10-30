
#include "level_components.h"
#include "main.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <string.h>
#include <stdio.h>

#define COMPONENT_TYPE_MESHRENDERER 0
#define COMPONENT_TYPE_DOOR 1

typedef struct MeshRendererComponent
{
    int16_t meshIndex;
    int16_t textureIndex;
    int8_t isDithered;
    Vector3 position;
    Vector3 eulerRotationDeg;
    Vector3 scale;
    Matrix transform;
} MeshRendererComponent;

static void MeshRendererComponent_updateTransform(MeshRendererComponent *component)
{
    component->transform = MatrixRotateXYZ((Vector3){DEG2RAD * component->eulerRotationDeg.x, DEG2RAD * component->eulerRotationDeg.y, DEG2RAD * component->eulerRotationDeg.z});
    component->transform = MatrixMultiply(component->transform, MatrixScale(component->scale.x, component->scale.y, component->scale.z));
    component->transform = MatrixMultiply(component->transform, MatrixTranslate(component->position.x, component->position.y, component->position.z));
}

void MeshRendererComponent_onDraw(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    MeshRendererComponent *component = (MeshRendererComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance || component->meshIndex < 0)
    {
        return;
    }
    // TraceLog(LOG_INFO, "Drawing mesh: %d", component->meshIndex);
    LevelMesh *mesh = &level->meshes[component->meshIndex];
    Material material = {0};
    material.shader = mesh->isDithered ? _modelDitherShader : _modelTexturedShader;
    MaterialMap maps[16];
    maps[MATERIAL_MAP_ALBEDO].texture = 
        mesh->textureIndex >= 0 ? level->textures[mesh->textureIndex].texture : (Texture2D) {0};
    material.maps = maps;
    Matrix m = MatrixMultiply(component->transform, instance->toWorldTransform);
    DrawMesh(mesh->model.meshes[0], material, m);
}

void MeshRendererComponent_onInit(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    MeshRendererComponent *component = (MeshRendererComponent*)componentInstanceData;
    component->meshIndex = -1;
    component->textureIndex = -1;
    component->isDithered = 0;
    component->position = (Vector3){0};
    component->eulerRotationDeg = (Vector3){0};
    component->scale = (Vector3){1.0f, 1.0f, 1.0f};
    MeshRendererComponent_updateTransform(component);
}

static MeshRendererComponent *_selectedMeshRendererComponent = NULL;
static Vector2 _selectedMeshRendererMenuPos = {0};
static float _selectedMeshRendererMenuWidth = 0.0f;

void MeshRendererComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos)
{
    MeshRendererComponent *component = (MeshRendererComponent*)componentInstanceData;
    float width = DuskGui_getAvailableSpace().x - 20;
    if (DuskGui_button((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, width, 20},
        .text = TextFormat("Mesh: %s##MeshRendererMesh-%p", component->meshIndex >= 0 ? level->meshes[component->meshIndex].filename : "None", component),
        .rayCastTarget = 1,
    }))
    {
        DuskGui_openMenu("MeshSelectMenu");
        _selectedMeshRendererMenuPos = DuskGui_toScreenSpace((Vector2){10, *ypos});
        _selectedMeshRendererMenuWidth = width;
        _selectedMeshRendererComponent = component;
    }

    *ypos += 20.0f;

    char buffer[128];
    sprintf(buffer, "MeshRendererTransform-%p", component);
    if (SceneDrawUi_transformUi(ypos, buffer, &component->position, &component->eulerRotationDeg, &component->scale))
    {
        MeshRendererComponent_updateTransform(component);
    }
}

void MeshRendererComponent_onEditorMenu(Level *level)
{
    DuskGuiParamsEntry *meshSelectMenu;
    if ((meshSelectMenu = DuskGui_beginMenu((DuskGuiParams){
        .bounds = (Rectangle){_selectedMeshRendererMenuPos.x, _selectedMeshRendererMenuPos.y, _selectedMeshRendererMenuWidth, level->meshCount * 20 + 10},
        .text = "MeshSelectMenu",
    })))
    {
        for (int i = 0; i < level->meshCount; i++)
        {
            LevelMesh *mesh = &level->meshes[i];
            if (DuskGui_menuItem(0, (DuskGuiParams){
                .bounds = (Rectangle){10, 5 + i * 20, _selectedMeshRendererMenuWidth, 20},
                .text = mesh->filename,
                .rayCastTarget = 1,
            }))
            {
                _selectedMeshRendererComponent->meshIndex = i;
                DuskGui_closeMenu("MeshSelectMenu");
            }
        }
        DuskGui_endMenu();
    }
}

void MeshRendererComponent_onSerialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    MeshRendererComponent *component = (MeshRendererComponent*)componentInstanceData;
    if (component->meshIndex >= 0)
    {
        cJSON_AddStringToObject(json, "mesh", level->meshes[component->meshIndex].filename);
    }
    if (component->textureIndex >= 0)
    {
        cJSON_AddStringToObject(json, "texture", level->textures[component->textureIndex].filename);
    }
    cJSON_AddNumberToObject(json, "isDithered", component->isDithered);
    cJSON* trs = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "trs", trs);
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->position.x));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->position.y));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->position.z));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->eulerRotationDeg.x));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->eulerRotationDeg.y));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->eulerRotationDeg.z));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->scale.x));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->scale.y));
    cJSON_AddItemToArray(trs, cJSON_CreateNumber(component->scale.z));
}

void MeshRendererComponent_onDeserialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    MeshRendererComponent *component = (MeshRendererComponent*)componentInstanceData;
    component->meshIndex = -1;
    component->textureIndex = -1;
    cJSON* trs = cJSON_GetObjectItem(json, "trs");
    component->position.x = (float) cJSON_GetArrayItem(trs, 0)->valuedouble;
    component->position.y = (float) cJSON_GetArrayItem(trs, 1)->valuedouble;
    component->position.z = (float) cJSON_GetArrayItem(trs, 2)->valuedouble;
    component->eulerRotationDeg.x = (float) cJSON_GetArrayItem(trs, 3)->valuedouble;
    component->eulerRotationDeg.y = (float) cJSON_GetArrayItem(trs, 4)->valuedouble;
    component->eulerRotationDeg.z = (float) cJSON_GetArrayItem(trs, 5)->valuedouble;
    component->scale.x = (float) cJSON_GetArrayItem(trs, 6)->valuedouble;
    component->scale.y = (float) cJSON_GetArrayItem(trs, 7)->valuedouble;
    component->scale.z = (float) cJSON_GetArrayItem(trs, 8)->valuedouble;
    MeshRendererComponent_updateTransform(component);

    component->isDithered = (int8_t) cJSON_GetObjectItem(json, "isDithered")->valueint;

    cJSON *instancesMesh = cJSON_GetObjectItem(json, "mesh");
    if (instancesMesh)
    {
        const char *meshName = instancesMesh->valuestring;
        for (int i = 0; i < level->meshCount; i++)
        {
            if (strcmp(level->meshes[i].filename, meshName) == 0)
            {
                component->meshIndex = i;
                break;
            }
        }
    }
    cJSON *instancesTex = cJSON_GetObjectItem(json, "texture");
    if (instancesTex)
    {
        const char *texName = instancesTex->valuestring;
        for (int i = 0; i < level->textureCount; i++)
        {
            if (strcmp(level->textures[i].filename, texName) == 0)
            {
                component->textureIndex = i;
                break;
            }
        }
    }
}

void LevelComponents_register(Level *level)
{
    Level_registerEntityComponentClass(level, COMPONENT_TYPE_MESHRENDERER, "MeshRenderer", 
        (LevelEntityComponentClassMethods){
            .onInitFn = MeshRendererComponent_onInit,
            .onDestroyFn = NULL,
            .onDisableFn = NULL,
            .onEnableFn = NULL,
            .onSerializeFn = MeshRendererComponent_onSerialize,
            .onDeserializeFn = MeshRendererComponent_onDeserialize,
            .onEditorInspectFn = MeshRendererComponent_onInspectorUi,
            .updateFn = NULL,
            .drawFn = MeshRendererComponent_onDraw,
            .onEditorMenuFn = MeshRendererComponent_onEditorMenu,
        }, sizeof(MeshRendererComponent));
}