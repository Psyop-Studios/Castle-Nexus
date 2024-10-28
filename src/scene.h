#ifndef PSY_SCENE_H
#define PSY_SCENE_H



typedef enum Scene { RAYLOGO, LOADING, MAINMENU, GAME, OPTIONS } scene_t;

void ChangeScene(scene_t scene);
void UpdateCurrentScene(void);
void DrawCurrentScene(void);


#endif