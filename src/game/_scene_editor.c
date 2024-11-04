#include "scene.h"
#include "_scenes.h"
#include "dusk-gui.h"
#include <raymath.h>
#include <math.h>
#include "level.h"
#include "util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum EditorMode
{
    EDITOR_MODE_EDITGEOMETRY,
    EDITOR_MODE_EDITENTITIES,

} EditorMode;

Vector3 _worldCursor = {0};

static int _viewIsHovered = 0;
static EditorMode _editorMode = EDITOR_MODE_EDITGEOMETRY;
static Camera _camera;
static int _updateCamera;
static char _levelFileNameBuffer[256] = {0};
static DuskGuiStyleGroup _editor_invisibleStyleGroup = { 0 };
static LevelMesh* _hoveredMesh;
static LevelMeshInstance* _hoveredMeshInstance;

static LevelMeshInstance *_textureMenuInstance = NULL;
static Vector2 _textureMenuPos = {0};
static LevelEntity *_selectedEntity = NULL;
static Vector2 _componentMenu = {0};
static LevelEntityInstanceId _selectedEntityId = {0};
static cJSON *_clipboard = NULL;

static void _drawCollider(Matrix instanceMatrix, LevelCollider *collider)
{
    Vector3 position = collider->position;
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(instanceMatrix));
    
    if (collider->type == LEVEL_COLLIDER_TYPE_AABOX)
    {
        Vector3 size = collider->aabox.size;
        DrawCubeWires(position, size.x, size.y, size.z, DB8_RED);
    }
    else if (collider->type == LEVEL_COLLIDER_TYPE_SPHERE)
    {
        DrawSphereWires(position, collider->sphere.radius, 16, 16, DB8_RED);
    }
    rlPopMatrix();
}

static void SceneDraw(GameContext *gameCtx, SceneConfig *SceneConfig)
{

    // ClearBackground(DB8_BG_DEEPPURPLE);
    // TraceLog(LOG_INFO, "SceneDraw: %d", SceneConfig->sceneId);
    BeginMode3D(_camera);
    _currentCamera = _camera;

    Level *level = Game_getLevel();

    Level_draw(level);
    // LevelCollisionResult results[16] = {0};
    // int resultCount = Level_findCollisions(level, Vector3Add(_worldCursor, (Vector3){0,0.5f,0}), 0.5f, 1, 0, results, 16);
    // for (int i = 0; i < resultCount; i++)
    // {
    //     // printf("Collision: %f %f %f\n", results[i].surfaceContact.x, results[i].surfaceContact.y, results[i].surfaceContact.z);
    //     DrawLine3D(results[i].surfaceContact, _worldCursor, DB8_RED);
    // }
    Matrix m = MatrixIdentity();
    for (int i = 0; i < level->colliderCount; i++)
    {
        LevelCollider *collider = &level->colliders[i];
        _drawCollider(m, collider);
    }
    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];
        for (int j = 0; j < mesh->instanceCount; j++)
        {
            LevelMeshInstance *instance = &mesh->instances[j];
            for (int k = 0; k < mesh->colliderCount; k++)
            {
                LevelCollider *collider = &mesh->colliders[k];
                _drawCollider(instance->toWorldTransform, collider);
            }
        }
    }
    if (_hoveredMeshInstance)
    {
        rlDrawRenderBatchActive();
        rlDisableDepthTest();

        Material material = {0};

        MaterialMap maps[16] = {0};

        maps[MATERIAL_MAP_ALBEDO].color = WHITE;
        maps[MATERIAL_MAP_ALBEDO].texture = Level_getTexture(level, "db8-dither.png", (Texture2D){0});
        // maps[MATERIAL_MAP_ALBEDO].texture = Level_getTexture(level, "dawnbringers-8-color-8x.png", (Texture2D){0});
        material.maps = maps;
        material.shader = _modelDitherShader;
        int uvOverrideLoc = GetShaderLocation(_modelDitherShader, "uvOverride");
        int uvBlockScaleLoc = GetShaderLocation(_modelDitherShader, "uvDitherBlockPosScale");
        int texSizeLoc = GetShaderLocation(_modelDitherShader, "texSize");
        SetShaderValue(_modelDitherShader, uvOverrideLoc, (float[]){508.0f/512.0f - 1.0f, 8.0f / 512.0f}, SHADER_UNIFORM_VEC2);
        SetShaderValue(_modelDitherShader, texSizeLoc, (float[]){512.0f, 512.0f}, SHADER_UNIFORM_VEC2);
        SetShaderValue(_modelDitherShader, uvBlockScaleLoc, (float[]){64.0f}, SHADER_UNIFORM_FLOAT);


        DrawMesh(_hoveredMesh->model.meshes[0], material, _hoveredMeshInstance->toWorldTransform);

        SetShaderValue(_modelDitherShader, uvOverrideLoc, (float[]){0.0f, 0.0f}, SHADER_UNIFORM_VEC2);
        SetShaderValue(_modelDitherShader, uvBlockScaleLoc, (float[]){16.0f}, SHADER_UNIFORM_FLOAT);
        SetShaderValue(_modelDitherShader, texSizeLoc, (float[]){128.0f, 128.0f}, SHADER_UNIFORM_VEC2);

        rlDrawRenderBatchActive();
        rlEnableDepthTest();
    }


    // DrawModel(_model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    BeginShaderMode(_modelTexturedShader);

    // draw a corner grid around a point
    const int gridCount = 4;

    for (int x = -gridCount; x < gridCount; x++)
    {
        for (int z = -gridCount; z < gridCount; z++)
        {

            Vector3 pos = {x + .5f, 0.0f, z + .5f};

            pos = Vector3Add(pos, _worldCursor);

            DrawLine3D((Vector3){pos.x - .1f, pos.y, pos.z}, (Vector3){pos.x + .1f, pos.y, pos.z}, DB8_GREY);

            DrawLine3D((Vector3){pos.x, pos.y, pos.z - .1f}, (Vector3){pos.x, pos.y, pos.z + .1f}, DB8_GREY);

        }
    }
    DrawCubeWires((Vector3){_worldCursor.x, _worldCursor.y + .5f, _worldCursor.z}, 1.0f, 1.0f, 1.0f, DB8_RED);

    if (_editorMode == EDITOR_MODE_EDITENTITIES)
    {
        for (int i = 0; i < level->entityCount; i++)
        {
            LevelEntity *entity = &level->entities[i];
            if (entity->name)
            {
                DrawCube(entity->position, .2f, .2f, .2f, DB8_BLUE);
            }
        }
    }

    // highlight the quad our mouse is hovering
    Vector3 hitPos = {0};
    Ray ray = GetMouseRay(GetMousePosition(), _camera);
    if (ray.direction.y != 0.0f && _viewIsHovered)
    {
        float dist = (_worldCursor.y - ray.position.y) / ray.direction.y;
        hitPos = Vector3Add(ray.position, Vector3Scale(ray.direction, dist));
        hitPos.x = floorf(hitPos.x + .5f);
        hitPos.y = hitPos.y;
        hitPos.z = floorf(hitPos.z + .5f);
        DrawCubeWires(hitPos, 1.0f, 0.0f, 1.0f, DB8_GREEN);
        if (IsKeyDown(KEY_SPACE))
        {
            _worldCursor = hitPos;
        }
        if (IsKeyDown(KEY_TWO))
        {
            _camera.position.y += 5.0 * GetFrameTime();
        }
        if (IsKeyDown(KEY_X))
        {
            _camera.position.y -= 5.0 * GetFrameTime();
        }
        if (IsKeyDown(KEY_ONE))
        {
            _camera.position.y = 1.70f;
        }
    }

    EndShaderMode();

    EndMode3D();


    if (_updateCamera)
    {

        UpdateCamera(&_camera, CAMERA_FIRST_PERSON);

    }
}

static void SceneDrawUi_mainBar(GameContext *gameCtx, SceneConfig *sceneConfig)
{
    DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { -5, -5, Game_getWidth() + 10, 30 },
        .rayCastTarget = 1,
    });


    DuskGui_label((DuskGuiParams) {
        .text = TextFormat("Cursor: %.1f %.1f %.1f", _worldCursor.x, _worldCursor.y, _worldCursor.z),
        .bounds = (Rectangle) { 10, 7, 180, 20 },
    });


    DuskGui_label((DuskGuiParams) {
        .text = "Level name:",
        .bounds = (Rectangle) { 180-190, 7, 180, 20 },
        .styleGroup = DuskGui_getStyleGroup(DUSKGUI_STYLE_LABEL_ALIGNRIGHT)
    });
    char *resultBuffer = NULL;
    DuskGui_textInputField((DuskGuiParams) {
        .text = TextFormat("%s##levelFileNameTextField", _levelFileNameBuffer),
        .isFocusable = 1,
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 180, 7, 180, 20 },
    }, &resultBuffer);
    if (resultBuffer)
    {
        strncpy(_levelFileNameBuffer, resultBuffer, 256);
        DuskGui_getLastEntry()->isFocused = 0;
    }

    if (DuskGui_button((DuskGuiParams) {
        .text = "Save",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 360, 7, 50, 20 },
    }))

    {
        Level_save(Game_getLevel(), _levelFileNameBuffer);
    }


    if (DuskGui_button((DuskGuiParams) {
        .text = "Load",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 410, 7, 50, 20 },
    }))
    {
        DuskGui_openMenu("LoadMenu");
    }

    if (DuskGui_button((DuskGuiParams) {
        .text = "New",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 460, 7, 50, 20 },
    }))
    {
        Level_clearInstances(Game_getLevel());
    }

    if (DuskGui_button((DuskGuiParams) {
        .text = "Play scene",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 510, 7, 80, 20 },
    }))
    {
        DuskGui_openMenu("PlaySceneMenu");
    }

    int switchScene = -1;
    static const char *playableScenes[] = {
        "Intro",
        "Docks",
        "Island",
        "Puzzle 1",
        "Puzzle 2",
        "Puzzle 3",
        "Finish",
        NULL,
    };
    if (DuskGui_contextMenuItemsPopup((DuskGuiParams) {
        .text = "PlaySceneMenu",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 510, 7, 80, 20 },
    }, 0, playableScenes, &switchScene))
    {
        Game_setNextScene(switchScene + 1);
    }

    static int showHelp = 0;
    if (DuskGui_button((DuskGuiParams) {
        .text = "HELP",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 600, 7, 80, 20 },
    }))
    {
        showHelp = 1;
    }

    float posX = Game_getWidth() - 105;

    const char* toggleModeName = _editorMode == EDITOR_MODE_EDITGEOMETRY ? "Geometry mode##mode_selection" : "Entity mode##mode_selection";
    if (DuskGui_button((DuskGuiParams) {
        .text = toggleModeName,
        .rayCastTarget = 1,
        .bounds = (Rectangle) { posX, 7, 100, 20 },
    }) || IsKeyPressed(KEY_TAB))
    {
        _editorMode = _editorMode == EDITOR_MODE_EDITGEOMETRY ? EDITOR_MODE_EDITENTITIES : EDITOR_MODE_EDITGEOMETRY;
    }

    posX -= 80;

    DuskGui_endPanel(panel);

    if (showHelp)
    {
        int helpWidth = 500;
        int helpHeight = 380;
        DuskGuiParamsEntryId helpPanel = DuskGui_beginPanel((DuskGuiParams) {
            .bounds = (Rectangle) { 
                (Game_getWidth() - helpWidth) * 0.5f, 
                (Game_getHeight() - helpHeight) * 0.5f, 
                helpWidth, helpHeight },
            .rayCastTarget = 1,
        });

        if (DuskGui_button((DuskGuiParams){
            .text = "X##closeHelp",
            .isFocusable = 1,
            .rayCastTarget = 1,
            .bounds = (Rectangle) { helpWidth - 20, 5, 20, 20 },
        }))
        {
            showHelp = 0;
        }

        static DuskGuiStyleGroup _helpStyleGroup = {0};
        static DuskGuiFontStyle _fontStyle = {0};

        _helpStyleGroup = *DuskGui_getStyleGroup(DUSKGUI_STYLE_LABEL);
        _fontStyle = *_helpStyleGroup.fallbackStyle.fontStyle;
        _fontStyle.fontSize = _fntMedium.baseSize * 1.0f;
        _helpStyleGroup.fallbackStyle.textOverflowType = DUSKGUI_OVERFLOW_CLIP;
        _helpStyleGroup.fallbackStyle.textAlignment.x = 0.0f;
        _helpStyleGroup.fallbackStyle.textAlignment.y = 0.0f;
        _helpStyleGroup.fallbackStyle.fontStyle = &_fontStyle;


        DuskGui_label((DuskGuiParams) {
            .text = "This is the editor developed during the game jam (we assumed the topic to be called 'editor', it seems).\n"
                "When you EDIT a textbox, press ENTER to confirm the change. Otherwise, it doesn't register.\n"
                "The various unnamed text boxes (like on the left) are search filters.\n"
                "\n"
                "Space: Place cursor on grid at the current mouse position\n"
                "1: Set camera to default height (1.7m)\n"
                "2: Move camera up\n"
                "X: Move camera down\n"
                "Tab: Switch between geometry and entity editing mode\n"
                "\n"
                "Geometry are the static objects in the scene, like walls, floors, etc.\n"
                "Some geometry models have a configured collider (not all XD).\n"
                "Entities are the dynamic objects in the scene, like the crates.\n"
                "Entities can have components that define their appearance and behavior\n"
            ,
            .bounds = (Rectangle) { 10, 30, helpWidth - 20, helpHeight - 40 },
            .styleGroup = &_helpStyleGroup, 
        });



        DuskGui_endPanel(helpPanel);

    }
}

static void SceneDrawUi_meshCreationBar(GameContext *gameCtx, SceneConfig *sceneConfig)
{
    DuskGuiParamsEntryId objectCreatePanel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { -2, 22, 200, Game_getHeight() - 20},
        .rayCastTarget = 1,
    });

    Level *level = Game_getLevel();
    float yPos = 10.0f;

    static char meshFileFilter[256] = {0};
    char *resultBuffer = NULL;
    DuskGui_textInputField((DuskGuiParams) {
        .text = TextFormat("%s##meshNameFilter", meshFileFilter),
        .isFocusable = 1,
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 10, yPos, 180, 20 },
    }, &resultBuffer);
    if (resultBuffer)
    {
        strncpy(meshFileFilter, resultBuffer, 256);
        DuskGui_getLastEntry()->isFocused = 0;
    }

    yPos += 22.0f;

    for (int i = 0; i < level->meshCount; i++)
    {

        LevelMesh *mesh = &level->meshes[i];

        char *lastSlash = strrchr(mesh->filename, '/');
        const char *pureName = GetFileNameWithoutExt(lastSlash ? lastSlash + 1 : mesh->filename);
        if (strlen(meshFileFilter) > 0 && !strstr(pureName, meshFileFilter))
        {
            continue;
        }

        if (DuskGui_button((DuskGuiParams) {
            .rayCastTarget = 1,
            .text = pureName,
            .bounds = (Rectangle) { 10, 10 + yPos, 180, 20 },
        }))
        {
            Level_addInstance(level, mesh->filename, _worldCursor, (Vector3){0, 0, 0}, (Vector3){1, 1, 1});
        }
        yPos += 20.0f;

    }
    DuskGui_endPanel(objectCreatePanel);
}

static void SceneDrawUi_meshInspector(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    Level *level = Game_getLevel();

    DuskGuiParamsEntryId objectEditPanel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { Game_getWidth() - 198, 22, 200, Game_getHeight() - 20},
        .rayCastTarget = 1,
    });


    float posY = 10.0f;
    DuskGuiParamsEntryId scrollArea = DuskGui_beginScrollArea((DuskGuiParams) {
        .text = "##MeshInspectorList",
        .bounds = (Rectangle) { 0, posY, 200, DuskGui_getAvailableSpace().y - posY - 10},
        .styleGroup = &_editor_invisibleStyleGroup,
        .rayCastTarget = 1,
    });
    posY = 0.0f;


    _hoveredMeshInstance = NULL;
    _hoveredMesh = NULL;

    for (int i = 0; i < level->meshCount; i++)
    {
        LevelMesh *mesh = &level->meshes[i];

        for (int j = 0; j < mesh->instanceCount; j++)
        {

            LevelMeshInstance *instance = &mesh->instances[j];

            Vector3 diff = Vector3Subtract(instance->position, _worldCursor);

            float dx = fabsf(diff.x);

            float dz = fabsf(diff.z);

            if (dx <= 1.0f && dz <= 1.0f)
            {
                DuskGuiParamsEntry* prev = DuskGui_getEntry(TextFormat("##Instance %d:%d", i, j), 1);
                float height = 100.0f;
                if (prev)
                {
                    height = prev->params.bounds.height;
                }
                DuskGuiParamsEntryId objectPanel = DuskGui_beginPanel((DuskGuiParams) {
                    .bounds = (Rectangle) { 0, posY, DuskGui_getAvailableSpace().x, height },
                    .rayCastTarget = 1,
                    .text = TextFormat("##Instance %d:%d", i, j),
                });
                float y = 5;
                DuskGui_horizontalLine((DuskGuiParams) {
                    .text = strrchr(mesh->filename, '/') + 1,
                    .bounds = (Rectangle) { 10, y, 180, 12 },
                });

                y += 14.0f;


                char buffer[128];
                sprintf(buffer, "%d:%d", i, j);

                if (SceneDrawUi_transformUi(&y, buffer, &instance->position, &instance->eulerRotationDeg, &instance->scale, &_worldCursor, (Vector3){1,10,1}))
                {
                    Level_updateInstanceTransform(instance);
                }

                y += 5.0f;

                DuskGui_label((DuskGuiParams) {
                    .text = "Texture",
                    .bounds = (Rectangle) { 10, y, 40, 20 },
                });
                const char *texName = "Default";
                if (instance->textureIndex >= 0 && instance->textureIndex < level->textureCount)
                {
                    texName = strrchr(level->textures[instance->textureIndex].filename, '/') + 1;
                }
                if (DuskGui_button((DuskGuiParams) {
                    .text = TextFormat("%s##%d:%d", texName, i, j),
                    .rayCastTarget = 1,
                    .bounds = (Rectangle) { 50, y, 140, 20 },
                }))
                {
                    DuskGui_openMenu("TextureMenu");
                    _textureMenuInstance = instance;
                    _textureMenuPos = DuskGui_toScreenSpace((Vector2){50, y});
                }

                y += 20.0f;

                if (DuskGui_button((DuskGuiParams) {
                    .text = "Delete",
                    .rayCastTarget = 1,
                    .bounds = (Rectangle) { 10, y, 180, 20 },
                }))
                {
                    for (int k = j; k < mesh->instanceCount - 1; k++)
                    {
                        mesh->instances[k] = mesh->instances[k + 1];
                    }
                    mesh->instanceCount--;
                }


                y += 25.0f;

                DuskGuiParamsEntry *panelEntry = DuskGui_getEntryById(objectPanel);
                panelEntry->params.bounds.height = y;
                if (panelEntry->isMouseOver)
                {
                    _hoveredMesh = mesh;
                    _hoveredMeshInstance = instance;
                }

                DuskGui_endPanel(objectPanel);

                posY += y + 5.0f;
            }
        }
    }

    DuskGuiParamsEntry *entry = DuskGui_getEntryById(scrollArea);
    entry->contentSize = (Vector2){200, posY + 10};
    DuskGui_endScrollArea(scrollArea);

    DuskGui_endPanel(objectEditPanel);
}

static void SceneDrawUi_drawEntityUi(Level *level, float *posY, LevelEntity* entity)
{
    char *nameBuffer = NULL;
    DuskGui_textInputField((DuskGuiParams) {
        .text = TextFormat("%s##entity-name-textfield-%p", entity->name, entity),
        .isFocusable = 1,
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 10, *posY, 180, 20 },
    }, &nameBuffer);
    if (nameBuffer)
    {
        free(entity->name);
        entity->name = strdup(nameBuffer);
        DuskGui_getLastEntry()->isFocused = 0;
    }

    *posY += 20.0f;
    
    if (DuskGui_button((DuskGuiParams) {
        .text = TextFormat("Add Component##AddComponent-%d-%d", entity->id, entity->generation),
        .bounds = (Rectangle) { 10, *posY, 180, 20 },
        .rayCastTarget = 1,}
        ))
    {
        DuskGui_openMenu("AddComponentMenu");
        _selectedEntity = entity;
        _componentMenu = DuskGui_toScreenSpace((Vector2){10, *posY});
    }

    *posY += 25.0f;

    *posY += 4.0f;
    DuskGui_horizontalLine((DuskGuiParams) {
        .bounds = (Rectangle) { 10, *posY, 180, 2 },
    });
    *posY += 4.0f;

    char buffer[128];
    sprintf(buffer, "%d-%d", entity->id, entity->generation);
    if (SceneDrawUi_transformUi(posY, buffer, &entity->position, &entity->eulerRotationDeg, &entity->scale, &_worldCursor, (Vector3){1,10,1}))
    {
        Level_updateEntityTransform(entity);
    }


    *posY += 5.0f;

    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        LevelEntityComponentClass *componentClass = &level->entityComponentClasses[i];
        if (componentClass->instanceCount > 0)
        {
            for (int j = 0; j < componentClass->instanceCount; j++)
            {
                LevelEntityInstanceId ownerId = componentClass->ownerIds[j];
                if (componentClass->generations[j] > 0 && ownerId.id == entity->id && ownerId.generation == entity->generation)
                {
                    sprintf(buffer, "%s-%d", componentClass->name, j);
                    DuskGui_horizontalLine((DuskGuiParams) {
                        .text = componentClass->name,
                        .bounds = (Rectangle) { 10, *posY, 180, 12.0f },
                    });
                    *posY += 12.0f;

                    if (componentClass->methods.onEditorInspectFn)
                    {
                        const char *panelId = TextFormat("##component-ui-%d-%d", i, j);
                        DuskGuiParamsEntry *prevPanel = DuskGui_getEntry(panelId, 1);
                        DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams) {
                            .bounds = (Rectangle) { 0, *posY, DuskGui_getAvailableSpace().x, 
                                prevPanel ? prevPanel->params.bounds.height : 100
                            },
                            .styleGroup = &_editor_invisibleStyleGroup,
                            .rayCastTarget = 1,
                            .text = panelId,
                        });
                        float y = 0.0f;
                        void *componentInstanceData = (char*)componentClass->componentInstanceData + j * componentClass->componentInstanceDataSize;
                        DuskGuiParamsEntry *panelEntry = DuskGui_getEntryById(panel);
                        componentClass->methods.onEditorInspectFn(level, (LevelEntityInstanceId){entity->id, entity->generation}, componentInstanceData, &y, panelEntry->isMouseOver);
                        panelEntry->params.bounds.height = y;
                        DuskGui_endPanel(panel);
                        *posY += y + 4.0f;
                    }

                    if (DuskGui_button((DuskGuiParams) {
                        .text = TextFormat("Delete %s##DeleteComponent-%s", componentClass->name, buffer),
                        .rayCastTarget = 1,
                        .bounds = (Rectangle) { 10, *posY, 180, 20 },
                    }))
                    {
                        componentClass->generations[j] = 0;
                    }
                    *posY += 25.0f;
                }
            }
        }
    }


    *posY += 25.0f;
    if (DuskGui_button((DuskGuiParams) {
        .text = TextFormat("Delete Entity##DeleteEntity-%d-%d", entity->id, entity->generation),
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 10, *posY, 180, 20 },
    }))
    {
        Level_deleteEntity(level, entity);
    }
    *posY += 20.0f;

    *posY += 10.0f;
}

static void SceneDrawUi_entitySelection(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { -2, 20, 200, Game_getHeight() - 15 },
        .rayCastTarget = 1,
    });
    Level *level = Game_getLevel();
    float posY = 10.0f;

    if (DuskGui_button((DuskGuiParams) {
        .text = "Add Entity",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 10, posY, 180, 20 },
    }))
    {
        LevelEntity *newEntity = Level_addEntity(level, "New Entity", _worldCursor, (Vector3){0,0,0}, (Vector3){1,1,1});
        _selectedEntityId = (LevelEntityInstanceId){newEntity->id, newEntity->generation};
    }

    posY += 30.0f;
    if (_clipboard)
    {
        if (DuskGui_button((DuskGuiParams) {
            .text = "Paste Entity",
            .rayCastTarget = 1,
            .bounds = (Rectangle) { 10, posY, 180, 20 },
        }))
        {
            LevelEntity *newEntity = Level_instantiatePrefab(level, _clipboard);
            if (newEntity)
            {
                newEntity->position = _worldCursor;
                Level_updateEntityTransform(newEntity);
                _selectedEntityId = (LevelEntityInstanceId){newEntity->id, newEntity->generation};
            }
        }

        posY += 30.0f;
    }

    static char entityFilter[256] = {0};
    char *resultBuffer = NULL;
    DuskGui_textInputField((DuskGuiParams) {
        .text = TextFormat("%s##entityNameFilter", entityFilter),
        .isFocusable = 1,
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 10, posY, 180, 20 },
    }, &resultBuffer);
    if (resultBuffer)
    {
        strncpy(entityFilter, resultBuffer, 256);
        DuskGui_getLastEntry()->isFocused = 0;
    }

    posY += 25.0f;

    for (int i = 0; i < level->entityCount; i++)
    {
        LevelEntity *entity = &level->entities[i];
        if (entity->name && (strlen(entityFilter) == 0 || strstr(entity->name, entityFilter)))
        {
            if (DuskGui_button((DuskGuiParams) {
                .text = entity->name,
                .rayCastTarget = 1,
                .bounds = (Rectangle) { 10, posY, 180, 20 },
            }))
            {
                _selectedEntityId = (LevelEntityInstanceId){entity->id, entity->generation};
                _worldCursor = entity->position;
                _worldCursor.x = floorf(_worldCursor.x + .5f);
                _worldCursor.y = floorf(_worldCursor.y + .5f);
                _worldCursor.z = floorf(_worldCursor.z + .5f);

            }
            posY += 20.0f;
        }
    }
    DuskGui_endPanel(panel);
}

static void SceneDrawUi_entityInspector(GameContext *gameCtx, SceneConfig *sceneConfig)
{
    Level *level = Game_getLevel();

    DuskGuiParamsEntryId objectEditPanel = DuskGui_beginPanel((DuskGuiParams) {
        .bounds = (Rectangle) { Game_getWidth() - 198, 22, 200, Game_getHeight() - 20},
        .rayCastTarget = 1,
        .text = "##entityInspector",
    });

    float posY = 10.0f;

    DuskGuiParamsEntryId scrollArea = DuskGui_beginScrollArea((DuskGuiParams) {
        .text = "##EntityList",
        .bounds = (Rectangle) { 0, posY, 200, DuskGui_getAvailableSpace().y - posY - 10},
        .styleGroup = &_editor_invisibleStyleGroup,
        .rayCastTarget = 1,
    });

    posY = 10.0f;

    LevelEntity *entity = Level_resolveEntity(level, _selectedEntityId);
    if (entity)
    {
        if (DuskGui_button((DuskGuiParams) {
            .text = "Copy",
            .rayCastTarget = 1,
            .bounds = (Rectangle) { 10, posY, 90, 20 },
        }))
        {
            if (_clipboard) cJSON_Delete(_clipboard);
            _clipboard = Level_serializeEntityAsPrefab(level, _selectedEntityId);
        }
        posY += 25.0f;
        SceneDrawUi_drawEntityUi(level, &posY, entity);
    }

    DuskGuiParamsEntry *entry = DuskGui_getEntryById(scrollArea);
    entry->contentSize = (Vector2){200, posY + 10};
    // TraceLog(LOG_INFO, "contentSize: %f %f, isMouseOver: %d %d", entry->contentSize.x, entry->contentSize.y, entry->isMouseOver, entry->isHovered);

    DuskGui_endScrollArea(scrollArea);


    DuskGui_endPanel(objectEditPanel);

}
static void SceneDrawUi(GameContext *gameCtx, SceneConfig *sceneConfig)
{
    Level *level = Game_getLevel();

    _updateCamera = DuskGui_dragArea((DuskGuiParams) {
        .text = "Camera",
        .bounds = (Rectangle) { 0, 0, Game_getWidth(), Game_getHeight() },
        .rayCastTarget = 1,
    });
    _viewIsHovered = DuskGui_getLastEntry()->isHovered;

    if (_editorMode == EDITOR_MODE_EDITGEOMETRY)
    {
        SceneDrawUi_meshCreationBar(gameCtx, sceneConfig);
        SceneDrawUi_meshInspector(gameCtx, sceneConfig);
    }
    if (_editorMode == EDITOR_MODE_EDITENTITIES)
    {
        SceneDrawUi_entitySelection(gameCtx, sceneConfig);
        SceneDrawUi_entityInspector(gameCtx, sceneConfig);
    }
    SceneDrawUi_mainBar(gameCtx, sceneConfig);

    DuskGuiParamsEntry* textureMenu;
    if ((textureMenu = DuskGui_beginMenu((DuskGuiParams) {
        .text = "TextureMenu",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { _textureMenuPos.x, _textureMenuPos.y, 160, 60 },
    })))
    {
        if (DuskGui_menuItem(0, (DuskGuiParams) {
            .text = "None",
            .rayCastTarget = 1,
            .bounds = (Rectangle) { 5, 5, 150, 20 },
        }))
        {
            _textureMenuInstance->textureIndex = -1;
            DuskGui_closeMenu("TextureMenu");
        }
        for (int i = 0; i < level->textureCount; i++)
        {

            LevelTexture *texture = &level->textures[i];

            if (DuskGui_menuItem(0, (DuskGuiParams) {
                .text = strrchr(texture->filename, '/') + 1,
                .rayCastTarget = 1,
                .bounds = (Rectangle) { 5, 25 + i * 20, 170, 20 },
            }))
            {
                _textureMenuInstance->textureIndex = i;
                DuskGui_closeMenu("TextureMenu");
            }

        }
        textureMenu->params.bounds.height = 30 + level->textureCount * 20;

        DuskGui_endMenu();
    }
    else
    {
        DuskGui_closeMenu("TextureMenu");
    }

    DuskGuiParamsEntry* menu;
    static FilePathList levelFiles = {0};
    if ((menu = DuskGui_beginMenu((DuskGuiParams) {
        .text = "LoadMenu",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { 530, 25, 110, 60 },
    })))
    {
        if (levelFiles.paths == NULL)
            levelFiles = LoadDirectoryFilesEx("resources/levels", ".lvl", 0);

        for (int i = 0; i < levelFiles.count; i++)
        {

            char *filename = levelFiles.paths[i];
            filename = replacePathSeps(filename);
            char *lastSlash = strrchr(filename, '/');
            if (lastSlash)
            {
                filename = lastSlash + 1;
            } else {
                lastSlash = filename;
            }


            if (DuskGui_menuItem(0, (DuskGuiParams) {
                .text = lastSlash,
                .rayCastTarget = 1,
                .bounds = (Rectangle) { 5, 5 + i * 20, 100, 20 },
            }))
            {
                Level_load(Game_getLevel(), levelFiles.paths[i]);
                const char *filename = GetFileNameWithoutExt(levelFiles.paths[i]);
                strncpy(_levelFileNameBuffer, filename, 256);
                DuskGui_closeMenu("LoadMenu");
            }

        }
        menu->params.bounds.height = 10 + levelFiles.count * 20;

        DuskGui_endMenu();
    }
    else
    {
        if (levelFiles.paths)
        {
            UnloadDirectoryFiles(levelFiles);
            levelFiles.paths = NULL;
        }

        DuskGui_closeMenu("LoadMenu");
    }



    DuskGuiParamsEntry *componentMenu = DuskGui_beginMenu((DuskGuiParams) {
        .text = "AddComponentMenu",
        .rayCastTarget = 1,
        .bounds = (Rectangle) { _componentMenu.x, _componentMenu.y, 180, 20 * level->entityComponentClassCount + 10.0f },
    });
    if (componentMenu)
    {
        for (int i = 0; i < level->entityComponentClassCount; i++)
        {
            LevelEntityComponentClass *componentClass = &level->entityComponentClasses[i];
            if (DuskGui_menuItem(0, (DuskGuiParams) {
                .text = componentClass->name,
                .rayCastTarget = 1,
                .bounds = (Rectangle) { 5, 5 + i * 20, 170, 20 },
            }))
            {
                Level_addEntityComponent(level, _selectedEntity, componentClass->componentId, NULL);
                DuskGui_closeMenu("AddComponentMenu");
            }
        }

        DuskGui_endMenu();
    }

    for (int i = 0; i < level->entityComponentClassCount; i++)
    {
        LevelEntityComponentClass *componentClass = &level->entityComponentClasses[i];
        if (componentClass->methods.onEditorMenuFn)
        {
            componentClass->methods.onEditorMenuFn(level);
        }
    }
}

static float fsign(float x)
{
    return x > 0 ? 1.0f : -1.0f;
}

static void SceneUpdate(GameContext *gameCtx, SceneConfig *SceneConfig, float dt)
{

    Vector3 camForward = Vector3Normalize((Vector3){_camera.target.x - _camera.position.x, _camera.target.y - _camera.position.y, _camera.target.z - _camera.position.z});
    Vector3 forward = fabsf(camForward.x) < fabsf(camForward.z) ? (Vector3){0, 0, fsign(camForward.z)} : (Vector3){fsign(camForward.x), 0, 0};
    Vector3 right = Vector3CrossProduct(forward, (Vector3){0, 1, 0});


    if (IsKeyReleased(KEY_UP))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            _worldCursor = Vector3Add(_worldCursor, (Vector3){0, 1, 0});
        }
        else
        {
            _worldCursor = Vector3Add(_worldCursor, forward);
        }
    }

    if (IsKeyReleased(KEY_DOWN))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            _worldCursor = Vector3Subtract(_worldCursor, (Vector3){0, 1, 0});
        }
        else
        {
            _worldCursor = Vector3Subtract(_worldCursor, forward);
        }
    }

    if (IsKeyReleased(KEY_LEFT))
    {
        _worldCursor = Vector3Subtract(_worldCursor, right);
    }

    if (IsKeyReleased(KEY_RIGHT))
    {
        _worldCursor = Vector3Add(_worldCursor, right);
    }

}

static void SceneInit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    EnableCursor();
    TraceLog(LOG_INFO, "Editor : %d", SceneConfig->sceneId);
    // _model = LoadModel("resources/level-blocks.glb");
    // _model.materials[0].shader = _modelDitherShader;
    // _model.materials[1].shader = _modelDitherShader;
    // _model.materials[2].shader = _modelTexturedShader;

    _camera = (Camera){0};
    _camera.position = (Vector3){ 5.0f, 1.70f, 2.0f };
    _camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    _camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    _camera.fovy = 45.0f;
    _camera.projection = CAMERA_PERSPECTIVE;

    Level *level = Game_getLevel();
    level->isEditor = 1;
    if (level->filename)
    {
        char *lastSlash = strrchr(level->filename, '/');
        if (lastSlash)
        {
            strncpy(_levelFileNameBuffer, lastSlash + 1, 256);
        }
        else
        {
            strncpy(_levelFileNameBuffer, level->filename, 256);
        }
        for (int i = 0; _levelFileNameBuffer[i]; i++)
        {
            if (_levelFileNameBuffer[i] == '.')
            {
                _levelFileNameBuffer[i] = '\0';
                break;
            }
        }

        Level_load(level, level->filename);
    }

    TraceLog(LOG_INFO, "SceneInit: %d done", SceneConfig->sceneId);
}

static void SceneDeinit(GameContext *gameCtx, SceneConfig *SceneConfig)
{
    if (_clipboard)
    {
        cJSON_Delete(_clipboard);
        _clipboard = NULL;
    }
    // UnloadModel(_model);
}

SceneConfig _scene_editor = {
    .drawLevelFn = SceneDraw,
    .updateFn = SceneUpdate,
    .initFn = SceneInit,
    .deinitFn = SceneDeinit,
    .drawUiFn = SceneDrawUi,
    .sceneId = SCENE_ID_EDITOR,
};