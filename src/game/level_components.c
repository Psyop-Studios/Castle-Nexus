
#include "level_components.h"
#include "main.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


//# CameraFacingComponent
typedef struct CameraFacingComponent
{
    uint8_t lockX, lockY, lockZ, useDirection;
} CameraFacingComponent;

void CameraFacingComponent_onDraw(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    CameraFacingComponent *component = (CameraFacingComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance)
    {
        return;
    }
    Vector3 forward = Vector3Normalize(Vector3Subtract(_currentCamera.position, 
        component->useDirection ? instance->position : _currentCamera.target));
    Vector3 lockedAxis = (Vector3){0, 1, 0};
    Matrix m = instance->toWorldTransform;
    int isLocked = component->lockX || component->lockY || component->lockZ;
    if (component->lockX) {
        lockedAxis.x = m.m0;
        lockedAxis.y = m.m1;
        lockedAxis.z = m.m2;
    }
    if (component->lockY) {
        lockedAxis.x = m.m4;
        lockedAxis.y = m.m5;
        lockedAxis.z = m.m6;
    }
    if (component->lockZ) {
        lockedAxis.x = m.m8;
        lockedAxis.y = m.m9;
        lockedAxis.z = m.m10;
    }

    Vector3 right = Vector3CrossProduct(lockedAxis, forward);
    Vector3 up = Vector3Normalize(Vector3CrossProduct(forward, right));
    right = Vector3CrossProduct(up, forward);
    m.m0 = right.x;
    m.m1 = right.y;
    m.m2 = right.z;
    m.m4 = up.x;
    m.m5 = up.y;
    m.m6 = up.z;
    m.m8 = forward.x;
    m.m9 = forward.y;
    m.m10 = forward.z;
    instance->toWorldTransform = m;
    instance->transformIsDirty = 1;
}

void CameraFacingComponent_onInit(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    CameraFacingComponent *component = (CameraFacingComponent*)componentInstanceData;
    component->lockX = 0;
    component->lockY = 0;
    component->lockZ = 0;
    component->useDirection = 0;
}

void CameraFacingComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos, int isMouseOver)
{
    CameraFacingComponent *component = (CameraFacingComponent*)componentInstanceData;
    
    *ypos += 20.0f;
}

void CameraFacingComponent_onSerialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    CameraFacingComponent *component = (CameraFacingComponent*)componentInstanceData;
    cJSON_AddBoolToObject(json, "lockX", component->lockX);
    cJSON_AddBoolToObject(json, "lockY", component->lockY);
    cJSON_AddBoolToObject(json, "lockZ", component->lockZ);
    cJSON_AddBoolToObject(json, "useDirection", component->useDirection);
}

void CameraFacingComponent_onDeserialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    CameraFacingComponent *component = (CameraFacingComponent*)componentInstanceData;
    cJSON *lockX = cJSON_GetObjectItem(json, "lockX");
    cJSON *lockY = cJSON_GetObjectItem(json, "lockY");
    cJSON *lockZ = cJSON_GetObjectItem(json, "lockZ");
    cJSON *useDirection = cJSON_GetObjectItem(json, "useDirection");
    if (lockX) component->lockX = lockX->valueint;
    if (lockY) component->lockY = lockY->valueint;
    if (lockZ) component->lockZ = lockZ->valueint;
    if (useDirection) component->useDirection = useDirection->valueint;
}

//# WobblerComponent

#define WOBBLE_TYPE_POSITION_X 0
#define WOBBLE_TYPE_POSITION_Y 1
#define WOBBLE_TYPE_POSITION_Z 2
#define WOBBLE_TYPE_ROTATION_X 3
#define WOBBLE_TYPE_ROTATION_Y 4
#define WOBBLE_TYPE_ROTATION_Z 5
#define WOBBLE_TYPE_SCALE_X 6
#define WOBBLE_TYPE_SCALE_Y 7
#define WOBBLE_TYPE_SCALE_Z 8
#define WOBBLE_TYPE_SCALE_UNIFORM 9

#define WOBBLE_ARGUMENT_TIME 0
#define WOBBLE_ARGUMENT_POSITION_X 1
#define WOBBLE_ARGUMENT_POSITION_Y 2
#define WOBBLE_ARGUMENT_POSITION_Z 3

const char *_wobbleTypes[] = {"Position.x", "Position.y", "Position.z", "Rotation.x", "Rotation.y", "Rotation.z", "Scale.x", "Scale.y", "Scale.z", "Scale.uniform", NULL};
const char *_wobbleArguments[] = {"Time", "Position.x", "Position.y", "Position.z", NULL};
typedef struct WobblerComponent
{
    uint8_t type;
    uint8_t argument;
    uint8_t useGameTime;
    float amplitude;
    float frequency;
    float phase;
    Vector3 pivot;
} WobblerComponent;

void WobblerComponent_onDraw(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    WobblerComponent *component = (WobblerComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance)
    {
        return;
    }
    double t = component->useGameTime ? level->gameTime : level->renderTime;
    double arg = 0;
    Vector3 position = (Vector3){0};
    Vector3 rotation = (Vector3){0};
    Vector3 scale = (Vector3){1.0f,1.0f,1.0f};
    switch (component->argument)
    {
    case WOBBLE_ARGUMENT_TIME:
        arg = t;
        break;
    case WOBBLE_ARGUMENT_POSITION_X:
        arg = position.x;
        break;
    case WOBBLE_ARGUMENT_POSITION_Y:
        arg = position.y;
        break;
    case WOBBLE_ARGUMENT_POSITION_Z:
        arg = position.z;
        break;
    }
    double amplitude = component->amplitude;
    // if (component->type == WOBBLE_TYPE_ROTATION_X || component->type == WOBBLE_TYPE_ROTATION_Y || component->type == WOBBLE_TYPE_ROTATION_Z)
    // {
    //     amplitude = RAD2DEG * amplitude;
    // }
    double sinfreq = component->frequency * arg + component->phase;
    sinfreq = fmod(sinfreq, 2.0 * PI);
    float wobble = amplitude * sin(sinfreq);
    switch (component->type)
    {
    case WOBBLE_TYPE_POSITION_X:
        position.x += wobble;
        break;
    case WOBBLE_TYPE_POSITION_Y:
        position.y += wobble;
        break;
    case WOBBLE_TYPE_POSITION_Z:
        position.z += wobble;
        break;
    case WOBBLE_TYPE_ROTATION_X:
        rotation.x += wobble;
        break;
    case WOBBLE_TYPE_ROTATION_Y:
        rotation.y += wobble;
        break;
    case WOBBLE_TYPE_ROTATION_Z:
        rotation.z += wobble;
        break;
    case WOBBLE_TYPE_SCALE_X:
        scale.x += wobble;
        break;
    case WOBBLE_TYPE_SCALE_Y:
        scale.y += wobble;
        break;
    case WOBBLE_TYPE_SCALE_Z:
        scale.z += wobble;
        break;
    case WOBBLE_TYPE_SCALE_UNIFORM:
        scale.x += wobble;
        scale.y += wobble;
        scale.z += wobble;
        break;
    }

    Matrix m = MatrixScale(scale.x, scale.y, scale.z);
    m = MatrixMultiply(m, MatrixRotateXYZ((Vector3){DEG2RAD * rotation.x, DEG2RAD * rotation.y, DEG2RAD * rotation.z}));
    m = MatrixMultiply(m, MatrixTranslate(position.x, position.y, position.z));
    instance->toWorldTransform.m12 -= component->pivot.x;
    instance->toWorldTransform.m13 -= component->pivot.y;
    instance->toWorldTransform.m14 -= component->pivot.z;
    instance->toWorldTransform = MatrixMultiply(m, instance->toWorldTransform);
    instance->toWorldTransform.m12 += component->pivot.x;
    instance->toWorldTransform.m13 += component->pivot.y;
    instance->toWorldTransform.m14 += component->pivot.z;
    instance->transformIsDirty = 1;
}

void WobblerComponent_onInit(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    WobblerComponent *component = (WobblerComponent*)componentInstanceData;
    component->type = 0;
    component->argument = 0;
    component->useGameTime = 0;
    component->amplitude = 0.1f;
    component->frequency = 1.0f;
    component->phase = 0.0f;
}

void WobblerComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos, int isMouseOver)
{
    WobblerComponent *component = (WobblerComponent*)componentInstanceData;
    component->type = DuskGui_comboMenu((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = TextFormat("WobblerType-%p", component),
        .rayCastTarget = 1,
    }, _wobbleTypes, component->type);
    *ypos += 20.0f;
    component->argument = DuskGui_comboMenu((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = TextFormat("WobblerArgument-%p", component),
        .rayCastTarget = 1,
    }, _wobbleArguments, component->argument);

    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = TextFormat("%.2f (Amplitude)##wobler-amplitude-%p", component->amplitude, component),
        .rayCastTarget = 1,
    }, &component->amplitude, -400.0f, 400.0f, fmaxf(fabsf(component->amplitude * 0.01f), 0.0025));
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = TextFormat("%.2f (Frequency)##wobler-frequency-%p", component->frequency, component),
        .rayCastTarget = 1,
    }, &component->frequency, -400.0f, 400.0f, fmaxf(fabsf(component->frequency * 0.01f), 0.0025f));
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = TextFormat("%.2f (Phase)##wobler-phase-%p", component->phase, component),
        .rayCastTarget = 1,
    }, &component->phase, -400.0f, 400.0f, fmaxf(fabsf(component->phase * 0.01f), 0.0025f));
    *ypos += 20.0f;

    DuskGui_label((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = "Pivot",
    });
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 60, 20},
        .text = TextFormat("%.2f##wobler-pivot-x-%p", component->pivot.x, component),
        .rayCastTarget = 1,
    }, &component->pivot.x, -400.0f, 400.0f, fmaxf(fabsf(component->pivot.x * 0.01f), 0.0025f));
    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){70, *ypos, 60, 20},
        .text = TextFormat("%.2f##wobler-pivot-y-%p", component->pivot.y, component),
        .rayCastTarget = 1,
    }, &component->pivot.y, -400.0f, 400.0f, fmaxf(fabsf(component->pivot.y * 0.01f), 0.0025f));
    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){130, *ypos, 60, 20},
        .text = TextFormat("%.2f##wobler-pivot-z-%p", component->pivot.z, component),
        .rayCastTarget = 1,
    }, &component->pivot.z, -400.0f, 400.0f, fmaxf(fabsf(component->pivot.z * 0.01f), 0.0025f));
    *ypos += 20.0f;

    if (DuskGui_button((DuskGuiParams){
        .text = TextFormat("Reset pivot##reset-wobbler-%p", component),
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .rayCastTarget = 1,
    }))
    {
        component->pivot = (Vector3){0};
    }
    *ypos += 20.0f;
}

void WobblerComponent_onSerialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    WobblerComponent *component = (WobblerComponent*)componentInstanceData;
    cJSON_AddNumberToObject(json, "type", component->type);
    cJSON_AddNumberToObject(json, "argument", component->argument);
    cJSON_AddNumberToObject(json, "useGameTime", component->useGameTime);
    cJSON_AddNumberToObject(json, "amplitude", component->amplitude);
    cJSON_AddNumberToObject(json, "frequency", component->frequency);
    cJSON_AddNumberToObject(json, "phase", component->phase);
    cJSON_AddNumberToObject(json, "pivotX", component->pivot.x);
    cJSON_AddNumberToObject(json, "pivotY", component->pivot.y);
    cJSON_AddNumberToObject(json, "pivotZ", component->pivot.z);
}

void WobblerComponent_onDeserialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    WobblerComponent *component = (WobblerComponent*)componentInstanceData;
    cJSON *type = cJSON_GetObjectItem(json, "type");
    cJSON *argument = cJSON_GetObjectItem(json, "argument");
    cJSON *useGameTime = cJSON_GetObjectItem(json, "useGameTime");
    cJSON *amplitude = cJSON_GetObjectItem(json, "amplitude");
    cJSON *frequency = cJSON_GetObjectItem(json, "frequency");
    cJSON *phase = cJSON_GetObjectItem(json, "phase");
    cJSON *pivotX = cJSON_GetObjectItem(json, "pivotX");
    cJSON *pivotY = cJSON_GetObjectItem(json, "pivotY");
    cJSON *pivotZ = cJSON_GetObjectItem(json, "pivotZ");
    if (type) component->type = type->valueint;
    if (argument) component->argument = argument->valueint;
    if (useGameTime) component->useGameTime = useGameTime->valueint;
    if (amplitude) component->amplitude = amplitude->valuedouble;
    if (frequency) component->frequency = frequency->valuedouble;
    if (phase) component->phase = phase->valuedouble;
    if (pivotX && pivotY && pivotZ) component->pivot = (Vector3){pivotX->valuedouble, pivotY->valuedouble, pivotZ->valuedouble};
}

//# MeshRendererComponent

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
    component->transform = MatrixScale(component->scale.x, component->scale.y, component->scale.z);
    component->transform = MatrixMultiply(component->transform, MatrixRotateXYZ((Vector3){DEG2RAD * component->eulerRotationDeg.x, DEG2RAD * component->eulerRotationDeg.y, DEG2RAD * component->eulerRotationDeg.z}));
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
    Vector2 texSize = {material.maps[MATERIAL_MAP_ALBEDO].texture.width, material.maps[MATERIAL_MAP_ALBEDO].texture.height};
    int locTexSize = GetShaderLocation(_modelDitherShader, "texSize");
    int locUvDitherBlockPosScale = GetShaderLocation(_modelDitherShader, "uvDitherBlockPosScale");
    SetShaderValue(material.shader, locTexSize, &texSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(material.shader, locUvDitherBlockPosScale, (float[1]){texSize.x / 8.0f}, SHADER_UNIFORM_FLOAT);
    
    Matrix m = MatrixMultiply(component->transform, instance->toWorldTransform);
    Game_setFogTextures(&material);
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

void MeshRendererComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos, int isMouseOver)
{
    MeshRendererComponent *component = (MeshRendererComponent*)componentInstanceData;
    float width = DuskGui_getAvailableSpace().x - 20;
    if (DuskGui_button((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, width, 20},
        .text = TextFormat("Mesh: %s##MeshRendererMesh-%p", component->meshIndex >= 0 ? strrchr(level->meshes[component->meshIndex].filename, '/') : "None", component),
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
    if (SceneDrawUi_transformUi(ypos, buffer, &component->position, &component->eulerRotationDeg, &component->scale, (&(Vector3){0,0,0})))
    {
        MeshRendererComponent_updateTransform(component);
    }
}

void MeshRendererComponent_onEditorMenu(Level *level)
{
    DuskGuiParamsEntry *meshSelectMenu;
    if ((meshSelectMenu = DuskGui_beginMenu((DuskGuiParams){
        .bounds = (Rectangle){_selectedMeshRendererMenuPos.x, _selectedMeshRendererMenuPos.y, _selectedMeshRendererMenuWidth, level->meshCount * 20 + 30},
        .text = "MeshSelectMenu",
    })))
    {
        static char filterBuffer[256] = {0};
        char *filter = NULL;
        DuskGui_textInputField((DuskGuiParams){
            .bounds = (Rectangle){10, 5, _selectedMeshRendererMenuWidth - 20, 20},
            .text = TextFormat("%s##MeshFilter-name", filterBuffer),
            .rayCastTarget = 1,
            .isFocusable = 1,
        }, &filter);
        
        if (filter) {
            strncpy(filterBuffer, filter, 256);
        }

        float yPos = 5 + 20;
        for (int i = 0; i < level->meshCount; i++)
        {
            LevelMesh *mesh = &level->meshes[i];
            char *lastSlash = strrchr(mesh->filename, '/');
            if (lastSlash)
            {
                lastSlash = lastSlash + 1;
            }
            else
            {
                lastSlash = mesh->filename;
            }

            if (filterBuffer[0] && !strstr(lastSlash, filterBuffer))
            {
                continue;
            }
            if (DuskGui_menuItem(0, (DuskGuiParams){
                .bounds = (Rectangle){10, yPos, _selectedMeshRendererMenuWidth, 20},
                .text = lastSlash,
                .rayCastTarget = 1,
            }))
            {
                _selectedMeshRendererComponent->meshIndex = i;
                DuskGui_closeMenu("MeshSelectMenu");
            }
            yPos += 20; 
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

//# SpriteRendererComponent
typedef struct SpriteRendererComponent
{
    int16_t textureIndex;
    char *selectedSprite;
    char *selectedAnimation;
    Vector3 position;
    Vector3 eulerRotationDeg;
    Vector3 scale;
    Matrix transform;
    float animationTime;
} SpriteRendererComponent;

static SpriteRendererComponent *_selectedSpriteRendererComponent = NULL;
static Vector2 _selectedSpriteRendererMenuPos = {0};
static float _selectedSpriteRendererMenuWidth = 0.0f;


static void SpriteRendererComponent_updateTransform(SpriteRendererComponent *component)
{
    component->transform = MatrixScale(component->scale.x, component->scale.y, component->scale.z);
    component->transform = MatrixMultiply(component->transform, MatrixRotateXYZ((Vector3){DEG2RAD * component->eulerRotationDeg.x, DEG2RAD * component->eulerRotationDeg.y, DEG2RAD * component->eulerRotationDeg.z}));
    component->transform = MatrixMultiply(component->transform, MatrixTranslate(component->position.x, component->position.y, component->position.z));
}

void SpriteRendererComponent_onDraw(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    SpriteRendererComponent *component = (SpriteRendererComponent*)componentInstanceData;
    component->animationTime += GetFrameTime();
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance || component->textureIndex < 0)
    {
        return;
    }
    LevelTexture *texture = &level->textures[component->textureIndex];
    Rectangle srcRect = (Rectangle){0, 0, (float)texture->texture.width, (float)texture->texture.height};
    Shader shader = _modelTexturedShader;
    Material material = {0};
    material.shader = shader;
    MaterialMap maps[16];
    maps[MATERIAL_MAP_ALBEDO].texture = texture->texture;
    maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    material.maps = maps;
    Matrix m = MatrixMultiply(component->transform, instance->toWorldTransform);
    LevelMesh *mesh = Level_getMesh(level, "sprite_mesh.glb");

    if (component->selectedSprite && component->selectedAnimation)
    {
        for (int i = 0; i < texture->animationCount; i++)
        {
            LevelTextureSpriteAnimation *anim = &texture->animations[i];
            if (strcmp(anim->name, component->selectedAnimation) == 0)
            {
                int frame = (int)(component->animationTime * anim->frameRate) % anim->frameCount;
                srcRect.x = anim->offset.x + frame * anim->frameSize.x;
                srcRect.y = anim->offset.y;
                srcRect.width = anim->frameSize.x;
                srcRect.height = anim->frameSize.y;
                break;
            }
        }
    }

    int uvTextureFrameLoc = GetShaderLocation(shader, "uvTextureFrame");
    srcRect.x /= texture->texture.width;
    srcRect.y /= texture->texture.height;
    srcRect.width /= texture->texture.width;
    srcRect.height /= texture->texture.height;
    SetShaderValue(shader, uvTextureFrameLoc, (float[4]){srcRect.x, srcRect.y, srcRect.width, srcRect.height}, SHADER_UNIFORM_VEC4);
    Game_setFogTextures(&material);
    DrawMesh(mesh->model.meshes[0], material, m);
    SetShaderValue(shader, uvTextureFrameLoc, (float[4]){0, 0, 0, 0}, SHADER_UNIFORM_VEC4);
}

void SpriteRendererComponent_onInit(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    SpriteRendererComponent *component = (SpriteRendererComponent*)componentInstanceData;
    component->textureIndex = -1;
    component->selectedSprite = NULL;
    component->position = (Vector3){0};
    component->eulerRotationDeg = (Vector3){0};
    component->scale = (Vector3){1.0f, 1.0f, 1.0f};
    SpriteRendererComponent_updateTransform(component);
}

void SpriteRendererComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos, int isMouseOver)
{
    SpriteRendererComponent *component = (SpriteRendererComponent*)componentInstanceData;
    float width = DuskGui_getAvailableSpace().x - 20;
    if (DuskGui_button((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, width, 20},
        .text = TextFormat("Texture: %s##SpriteRendererTexture-%p", component->textureIndex >= 0 ? level->textures[component->textureIndex].filename : "None", component),
        .rayCastTarget = 1,
    }))
    {
        DuskGui_openMenu("TextureSelectMenu");
        _selectedSpriteRendererMenuPos = DuskGui_toScreenSpace((Vector2){10, *ypos});
        _selectedSpriteRendererMenuWidth = width;
        _selectedSpriteRendererComponent = component;
    }

    if (component->textureIndex >= 0)
    {
        LevelTexture *texture = &level->textures[component->textureIndex];
        if (texture->animationCount > 0)
        {
            *ypos += 20.0f;
            if (DuskGui_button((DuskGuiParams){
                .bounds = (Rectangle){10, *ypos, width, 20},
                .text = TextFormat("Animation: %s##SpriteRendererSprite-%p", component->selectedAnimation, component),
                .rayCastTarget = 1,
            }))
            {
                DuskGui_openMenu("AnimationSelectMenu");
                _selectedSpriteRendererMenuPos = DuskGui_toScreenSpace((Vector2){10, *ypos});
                _selectedSpriteRendererMenuWidth = width;
                _selectedSpriteRendererComponent = component;
            }
        }
    }

    *ypos += 20.0f;

    char buffer[128];
    sprintf(buffer, "SpriteRendererTransform-%p", component);
    if (SceneDrawUi_transformUi(ypos, buffer, &component->position, &component->eulerRotationDeg, &component->scale, (&(Vector3){0,0,0})))
    {
        SpriteRendererComponent_updateTransform(component);
    }
}

void SpriteRendererComponent_onEditorMenu(Level *level)
{
    DuskGuiParamsEntry *textureSelectMenu;
    if ((textureSelectMenu = DuskGui_beginMenu((DuskGuiParams){
        .bounds = (Rectangle){_selectedSpriteRendererMenuPos.x, _selectedSpriteRendererMenuPos.y, _selectedSpriteRendererMenuWidth, level->textureCount * 20 + 10},
        .text = "TextureSelectMenu",
    })))
    {
        if (DuskGui_menuItem(0, (DuskGuiParams){
            .bounds = (Rectangle){10, 5, _selectedSpriteRendererMenuWidth - 10, 20},
            .text = "None",
            .rayCastTarget = 1,
        }))
        {
            _selectedSpriteRendererComponent->textureIndex = -1;
            _selectedSpriteRendererComponent->selectedSprite = NULL;
            DuskGui_closeMenu("TextureSelectMenu");
        }

        for (int i = 0; i < level->textureCount; i++)
        {
            LevelTexture *texture = &level->textures[i];
            if (DuskGui_menuItem(0, (DuskGuiParams){
                .bounds = (Rectangle){10, 5 + (i+1) * 20, _selectedSpriteRendererMenuWidth - 10, 20},
                .text = texture->filename,
                .rayCastTarget = 1,
            }))
            {
                _selectedSpriteRendererComponent->textureIndex = i;
                _selectedSpriteRendererComponent->selectedSprite = texture->filename;
                DuskGui_closeMenu("TextureSelectMenu");
            }
        }
        DuskGui_endMenu();
    }

    DuskGuiParamsEntry *animationSelectMenu;
    if ((animationSelectMenu = DuskGui_beginMenu((DuskGuiParams){
        .bounds = (Rectangle){_selectedSpriteRendererMenuPos.x, _selectedSpriteRendererMenuPos.y, _selectedSpriteRendererMenuWidth, level->textureCount * 20 + 10},
        .text = "AnimationSelectMenu",
    })))
    {
        LevelTexture *texture = &level->textures[_selectedSpriteRendererComponent->textureIndex];
        if (DuskGui_menuItem(0, (DuskGuiParams){
            .bounds = (Rectangle){10, 5, _selectedSpriteRendererMenuWidth - 10, 20},
            .text = "None",
            .rayCastTarget = 1,
        }))
        {
            _selectedSpriteRendererComponent->selectedAnimation = NULL;
            DuskGui_closeMenu("AnimationSelectMenu");
        }

        for (int i = 0; i < texture->animationCount; i++)
        {
            LevelTextureSpriteAnimation *animation = &texture->animations[i];
            if (DuskGui_menuItem(0, (DuskGuiParams){
                .bounds = (Rectangle){10, 5 + (i+1) * 20, _selectedSpriteRendererMenuWidth - 10, 20},
                .text = animation->name,
                .rayCastTarget = 1,
            }))
            {
                _selectedSpriteRendererComponent->selectedAnimation = animation->name;
                DuskGui_closeMenu("AnimationSelectMenu");
            }
        }
        DuskGui_endMenu();
    }
}

void SpriteRendererComponent_onSerialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    SpriteRendererComponent *component = (SpriteRendererComponent*)componentInstanceData;
    if (component->textureIndex >= 0)
    {
        cJSON_AddStringToObject(json, "texture", level->textures[component->textureIndex].filename);
    }
    cJSON_AddStringToObject(json, "sprite", component->selectedSprite);
    cJSON_AddStringToObject(json, "animation", component->selectedAnimation);
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

void SpriteRendererComponent_onDeserialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    SpriteRendererComponent *component = (SpriteRendererComponent*)componentInstanceData;
    component->textureIndex = -1;
    component->selectedSprite = NULL;
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
    SpriteRendererComponent_updateTransform(component);

    cJSON *spriteName = cJSON_GetObjectItem(json, "sprite");
    component->textureIndex = -1;
    component->selectedSprite = NULL;
    component->selectedAnimation = NULL;
    if (spriteName)
    {
        LevelTexture *texture = Level_getLevelTexture(level, spriteName->valuestring);
        if (texture)
        {
            component->textureIndex = texture->index;
            component->selectedSprite = texture->filename;

            cJSON *animationName = cJSON_GetObjectItem(json, "animation");
            if (animationName)
            {
                for (int i = 0; i < texture->animationCount; i++)
                {
                    if (strcmp(texture->animations[i].name, animationName->valuestring) == 0)
                    {
                        component->selectedAnimation = texture->animations[i].name;
                        break;
                    }
                }
            }
        }
        else {
            TraceLog(LOG_WARNING, "SpriteRendererComponent: Failed to find sprite %s", spriteName->valuestring);
        }
    }
}

//# ColliderBoxComponent

static ColliderBoxComponent *_selectedColliderBoxComponent = NULL;

void ColliderBoxComponent_onInit(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    ColliderBoxComponent *component = (ColliderBoxComponent*)componentInstanceData;
    component->size = (Vector3){1.0f, 1.0f, 1.0f};
    component->offset = (Vector3){0.0f, 0.0f, 0.0f};
}

void ColliderBoxComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos, int isMouseOver)
{
    ColliderBoxComponent *component = (ColliderBoxComponent*)componentInstanceData;
    if (isMouseOver)
    {
        _selectedColliderBoxComponent = component;
    }
    else if (component <= _selectedColliderBoxComponent)
    {
        // new iteration, let's reset
        _selectedColliderBoxComponent = NULL;
    }
    DuskGui_label((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = "Size",
    });
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 60, 20},
        .text = TextFormat("%.2f##collider-box-size-x-%p", component->size.x, component),
        .rayCastTarget = 1,
    }, &component->size.x, 0.01f, 100.0f, 0.01f);
    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){70, *ypos, 60, 20},
        .text = TextFormat("%.2f##collider-box-size-y-%p", component->size.y, component),
        .rayCastTarget = 1,
    }, &component->size.y, 0.01f, 100.0f, 0.01f);
    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){130, *ypos, 60, 20},
        .text = TextFormat("%.2f##collider-box-size-z-%p", component->size.z, component),
        .rayCastTarget = 1,
    }, &component->size.z, 0.01f, 100.0f, 0.01f);
    *ypos += 20.0f;

    DuskGui_label((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = "Offset",
    });
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 60, 20},
        .text = TextFormat("%.2f##collider-box-offset-x-%p", component->offset.x, component),
        .rayCastTarget = 1,
    }, &component->offset.x, -100.0f, 100.0f, 0.01f);
    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){70, *ypos, 60, 20},
        .text = TextFormat("%.2f##collider-box-offset-y-%p", component->offset.y, component),
        .rayCastTarget = 1,
    }, &component->offset.y, -100.0f, 100.0f, 0.01f);
    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){130, *ypos, 60, 20},
        .text = TextFormat("%.2f##collider-box-offset-z-%p", component->offset.z, component),
        .rayCastTarget = 1,
    }, &component->offset.z, -100.0f, 100.0f, 0.01f);
    *ypos += 20.0f;

    if (DuskGui_button((DuskGuiParams){
        .text = TextFormat("IsTrigger: %s##collider-box-trigger-%p", component->isTrigger ? "True" : "False", component),
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .rayCastTarget = 1,
    }))
    {
        component->isTrigger = !component->isTrigger;
    }
    *ypos += 20.0f;
}

void ColliderBoxComponent_onSerialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    ColliderBoxComponent *component = (ColliderBoxComponent*)componentInstanceData;
    cJSON* size = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "size", size);
    cJSON_AddItemToArray(size, cJSON_CreateNumber(component->size.x));
    cJSON_AddItemToArray(size, cJSON_CreateNumber(component->size.y));
    cJSON_AddItemToArray(size, cJSON_CreateNumber(component->size.z));
    cJSON* offset = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "offset", offset);
    cJSON_AddItemToArray(offset, cJSON_CreateNumber(component->offset.x));
    cJSON_AddItemToArray(offset, cJSON_CreateNumber(component->offset.y));
    cJSON_AddItemToArray(offset, cJSON_CreateNumber(component->offset.z));
    cJSON_AddNumberToObject(json, "isTrigger", component->isTrigger);
}

void ColliderBoxComponent_onDeserialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    ColliderBoxComponent *component = (ColliderBoxComponent*)componentInstanceData;
    cJSON* size = cJSON_GetObjectItem(json, "size");
    component->size.x = (float) cJSON_GetArrayItem(size, 0)->valuedouble;
    component->size.y = (float) cJSON_GetArrayItem(size, 1)->valuedouble;
    component->size.z = (float) cJSON_GetArrayItem(size, 2)->valuedouble;
    cJSON* offset = cJSON_GetObjectItem(json, "offset");
    component->offset.x = (float) cJSON_GetArrayItem(offset, 0)->valuedouble;
    component->offset.y = (float) cJSON_GetArrayItem(offset, 1)->valuedouble;
    component->offset.z = (float) cJSON_GetArrayItem(offset, 2)->valuedouble;
    if (cJSON_HasObjectItem(json, "isTrigger"))
    {
        component->isTrigger = (int8_t) cJSON_GetObjectItem(json, "isTrigger")->valueint;
    }
}

void ColliderBoxComponent_onDraw(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    if (!level->isEditor)
    {
        return;
    }
    ColliderBoxComponent *component = (ColliderBoxComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance)
    {
        return;
    }
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(instance->toWorldTransform));
    if (_selectedColliderBoxComponent == component)
    {
        DrawCubeV(component->offset, component->size, DB8_RED);
    }
    DrawCubeWires((Vector3){component->offset.x, component->offset.y, component->offset.z}, component->size.x, component->size.y, component->size.z, DB8_RED);
    rlPopMatrix();
}

void ColliderBoxComponent_register(Level *level)
{
    Level_registerEntityComponentClass(level, COMPONENT_TYPE_COLLIDER_BOX, "ColliderBox", 
        (LevelEntityComponentClassMethods){
            .onInitFn = ColliderBoxComponent_onInit,
            .onDestroyFn = NULL,
            .onDisableFn = NULL,
            .onEnableFn = NULL,
            .onSerializeFn = ColliderBoxComponent_onSerialize,
            .onDeserializeFn = ColliderBoxComponent_onDeserialize,
            .onEditorInspectFn = ColliderBoxComponent_onInspectorUi,
            .updateFn = NULL,
            .drawFn = ColliderBoxComponent_onDraw,
            .onEditorMenuFn = NULL,
        }, sizeof(ColliderBoxComponent));
}

//# RigidSphereComponent
// a component that is like a rigidbody but uses only a sphere as approximation for collision,
// aka a poor man's physics engine
typedef struct RigidSphereComponent
{
    float radius;
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    float drag;
    float bounce;
    uint8_t hasGroundContact;
} RigidSphereComponent;

void RigidSphereComponent_onInit(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    RigidSphereComponent *component = (RigidSphereComponent*)componentInstanceData;
    component->radius = 1.0f;
    component->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    component->acceleration = (Vector3){0.0f, 0.0f, 0.0f};
    component->mass = 1.0f;
    component->drag = 0.0f;
    component->bounce = 0.0f;
}

void RigidSphereComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos, int isMouseOver)
{
    RigidSphereComponent *component = (RigidSphereComponent*)componentInstanceData;
    DuskGui_label((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = "Radius",
    });
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 60, 20},
        .text = TextFormat("%.2f##rigid-sphere-radius-%p", component->radius, component),
        .rayCastTarget = 1,
    }, &component->radius, 0.01f, 100.0f, 0.01f);
    *ypos += 20.0f;

    DuskGui_label((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = "Mass",
    });
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 60, 20},
        .text = TextFormat("%.2f##rigid-sphere-mass-%p", component->mass, component),
        .rayCastTarget = 1,
    }, &component->mass, 0.01f, 100.0f, 0.01f);
    *ypos += 20.0f;

    DuskGui_label((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = "Drag",
    });
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 60, 20},
        .text = TextFormat("%.2f##rigid-sphere-drag-%p", component->drag, component),
        .rayCastTarget = 1,
    }, &component->drag, 0.0f, 100.0f, 0.01f);
    *ypos += 20.0f;

    DuskGui_label((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 180, 20},
        .text = "Bounce",
    });
    *ypos += 20.0f;

    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, 60, 20},
        .text = TextFormat("%.2f##rigid-sphere-bounce-%p", component->bounce, component),
        .rayCastTarget = 1,
    }, &component->bounce, 0.0f, 1.0f, 0.01f);

    *ypos += 20.0f;

}

void RigidSphereComponent_onSerialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    RigidSphereComponent *component = (RigidSphereComponent*)componentInstanceData;
    cJSON_AddNumberToObject(json, "radius", component->radius);
    cJSON_AddNumberToObject(json, "mass", component->mass);
    cJSON_AddNumberToObject(json, "drag", component->drag);
    cJSON_AddNumberToObject(json, "bounce", component->bounce);
}

void RigidSphereComponent_onDeserialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    RigidSphereComponent *component = (RigidSphereComponent*)componentInstanceData;
    component->radius = (float) cJSON_GetObjectItem(json, "radius")->valuedouble;
    component->mass = (float) cJSON_GetObjectItem(json, "mass")->valuedouble;
    component->drag = (float) cJSON_GetObjectItem(json, "drag")->valuedouble;
    component->bounce = (float) cJSON_GetObjectItem(json, "bounce")->valuedouble;
}

void RigidSphereComponent_onDraw(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    if (!level->isEditor)
    {
        return;
    }
    RigidSphereComponent *component = (RigidSphereComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance)
    {
        return;
    }
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(instance->toWorldTransform));
    DrawSphereWires((Vector3){0, 0, 0}, component->radius, 16, 16, DB8_RED);
    rlPopMatrix();
}

void RigidSphereComponent_onUpdate(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float dt)
{
    RigidSphereComponent *component = (RigidSphereComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance)
    {
        return;
    }
    component->velocity = Vector3Add(component->velocity, component->acceleration);
    component->velocity = Vector3Scale(component->velocity, 1.0f - component->drag * dt);
    instance->position = Vector3Add(instance->position, Vector3Scale(component->velocity, dt));
    LevelCollisionResult results[8];
    int resultCount = Level_findCollisions(level, instance->position, component->radius, 1, 0, results, 8);

    // gravity
    component->velocity = Vector3Add(component->velocity, (Vector3) {0, -24.0f * dt, 0});
    
    Vector3 totalShift = {0};
    component->hasGroundContact = 0;

    for (int i = 0; i < resultCount; i++)
    {
        Vector3 normal = results[i].normal;

        if (normal.y > 0.95f)
        {
            // lets assume upward facing normals are flat floors to avoid glitches
            normal = (Vector3){0,1.0f,0};
            component->hasGroundContact = 1;
        }
        Vector3 shift = Vector3Scale(normal, results[i].depth);
        if (fabsf(shift.y) > fabsf(totalShift.y))
        {
            totalShift.y = shift.y;
        }
        if (fabsf(shift.x) > fabsf(totalShift.x))
        {
            totalShift.x = shift.x;
        }
        if (fabsf(shift.z) > fabsf(totalShift.z))
        {
            totalShift.z = shift.z;
        }
        // cancel velocity in direction of normal
        // printf("velocity: %f %f %f -> ", camera->velocity.x, camera->velocity.y, camera->velocity.z);
        if (normal.y > 0.95f)
        {
            component->velocity.y = 0;
        }
        // camera->velocity = Vector3Subtract(camera->velocity, Vector3Scale(normal, Vector3DotProduct(camera->velocity, results[i].normal)));
        // printf(" %f %f %f\n", camera->velocity.x, camera->velocity.y, camera->velocity.z);
    }

    instance->position = Vector3Add(instance->position, totalShift);

    Level_updateEntityTransform(instance);
}

void RigidSphereComponent_register(Level *level)
{
    Level_registerEntityComponentClass(level, COMPONENT_TYPE_RIGID_SPHERE, "RigidSphere", 
        (LevelEntityComponentClassMethods){
            .onInitFn = RigidSphereComponent_onInit,
            .onDestroyFn = NULL,
            .onDisableFn = NULL,
            .onEnableFn = NULL,
            .onSerializeFn = RigidSphereComponent_onSerialize,
            .onDeserializeFn = RigidSphereComponent_onDeserialize,
            .onEditorInspectFn = RigidSphereComponent_onInspectorUi,
            .updateFn = RigidSphereComponent_onUpdate,
            .drawFn = RigidSphereComponent_onDraw,
            .onEditorMenuFn = NULL,
        }, sizeof(RigidSphereComponent));
}

//# CollisionDetector
typedef struct CollisionDetectorComponent
{
    float radius;
} CollisionDetectorComponent;

void CollisionDetectorComponent_onInit(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    CollisionDetectorComponent *component = (CollisionDetectorComponent*)componentInstanceData;
    component->radius = 0.5f;
}

void CollisionDetectorComponent_onInspectorUi(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float *ypos, int isMouseOver)
{
    CollisionDetectorComponent *component = (CollisionDetectorComponent*)componentInstanceData;
    
    DuskGui_floatInputField((DuskGuiParams){
        .bounds = (Rectangle){10, *ypos, DuskGui_getAvailableSpace().x - 20, 20},
        .text = TextFormat("Radius: %.2f##collision-detector-radius-%p", component->radius, component),
        .rayCastTarget = 1,
    }, &component->radius, 0.01f, 100.0f, 0.01f);
    *ypos += 20.0f;
}

void CollisionDetectorComponent_onSerialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    CollisionDetectorComponent *component = (CollisionDetectorComponent*)componentInstanceData;
    cJSON_AddNumberToObject(json, "radius", component->radius);
}

void CollisionDetectorComponent_onDeserialize(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, cJSON *json)
{
    CollisionDetectorComponent *component = (CollisionDetectorComponent*)componentInstanceData;
    component->radius = (float) cJSON_GetObjectItem(json, "radius")->valuedouble;
}

void CollisionDetectorComponent_onDraw(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData)
{
    if (!level->isEditor)
    {
        return;
    }
    CollisionDetectorComponent *component = (CollisionDetectorComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance)
    {
        return;
    }
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(instance->toWorldTransform));
    DrawSphereWires((Vector3){0, 0, 0}, component->radius, 16, 16, DB8_RED);
    rlPopMatrix();
}

void CollisionDetectorComponent_onUpdate(Level *level, LevelEntityInstanceId ownerId, void *componentInstanceData, float dt)
{
    CollisionDetectorComponent *component = (CollisionDetectorComponent*)componentInstanceData;
    LevelEntity *instance = Level_resolveEntity(level, ownerId);
    if (!instance)
    {
        return;
    }
    LevelCollisionResult results[8];
    int resultCount = Level_findCollisions(level, instance->position, component->radius, 1, 0, results, 8);
    for (int i = 0; i < resultCount; i++)
    {
        if (results[i].ownerId.id || results[i].ownerId.generation)
        {
            Level_addTriggerId(level, instance->name);
            break;
        }
    }
}

void CollisionDetectorComponent_register(Level *level)
{
    Level_registerEntityComponentClass(level, COMPONENT_TYPE_COLLISION_DETECTOR, "CollisionDetector", 
        (LevelEntityComponentClassMethods){
            .onInitFn = CollisionDetectorComponent_onInit,
            .onDestroyFn = NULL,
            .onDisableFn = NULL,
            .onEnableFn = NULL,
            .onSerializeFn = CollisionDetectorComponent_onSerialize,
            .onDeserializeFn = CollisionDetectorComponent_onDeserialize,
            .onEditorInspectFn = CollisionDetectorComponent_onInspectorUi,
            .updateFn = CollisionDetectorComponent_onUpdate,
            .drawFn = CollisionDetectorComponent_onDraw,
            .onEditorMenuFn = NULL,
        }, sizeof(CollisionDetectorComponent));
}

//# Registration
void LevelComponents_register(Level *level)
{
    Level_registerEntityComponentClass(level, COMPONENT_TYPE_CAMERAFACING, "CameraFacing", 
        (LevelEntityComponentClassMethods){
            .onInitFn = CameraFacingComponent_onInit,
            .onDestroyFn = NULL,
            .onDisableFn = NULL,
            .onEnableFn = NULL,
            .onSerializeFn = CameraFacingComponent_onSerialize,
            .onDeserializeFn = CameraFacingComponent_onDeserialize,
            .onEditorInspectFn = CameraFacingComponent_onInspectorUi,
            .updateFn = NULL,
            .drawFn = CameraFacingComponent_onDraw,
            .onEditorMenuFn = NULL,
        }, sizeof(CameraFacingComponent));

    Level_registerEntityComponentClass(level, COMPONENT_TYPE_WOBBLER, "Wobbler",
        (LevelEntityComponentClassMethods){
            .onInitFn = WobblerComponent_onInit,
            .onDestroyFn = NULL,
            .onDisableFn = NULL,
            .onEnableFn = NULL,
            .onSerializeFn = WobblerComponent_onSerialize,
            .onDeserializeFn = WobblerComponent_onDeserialize,
            .onEditorInspectFn = WobblerComponent_onInspectorUi,
            .updateFn = NULL,
            .drawFn = WobblerComponent_onDraw,
            .onEditorMenuFn = NULL,
        }, sizeof(WobblerComponent));

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

    Level_registerEntityComponentClass(level, COMPONENT_TYPE_SPRITERENDERER, "SpriteRenderer",
        (LevelEntityComponentClassMethods){
            .onInitFn = SpriteRendererComponent_onInit,
            .onDestroyFn = NULL,
            .onDisableFn = NULL,
            .onEnableFn = NULL,
            .onSerializeFn = SpriteRendererComponent_onSerialize,
            .onDeserializeFn = SpriteRendererComponent_onDeserialize,
            .onEditorInspectFn = SpriteRendererComponent_onInspectorUi,
            .updateFn = NULL,
            .drawFn = SpriteRendererComponent_onDraw,
            .onEditorMenuFn = SpriteRendererComponent_onEditorMenu,
        }, sizeof(SpriteRendererComponent));

    ColliderBoxComponent_register(level);
    RigidSphereComponent_register(level);
    CollisionDetectorComponent_register(level);
}