#ifndef PSY_TIMER_H
#define PSY_TIMER_H

#include <raylib.h>


typedef struct Timer {
        float duration;
        float progress;
        bool  isRunning;
} Timer;

void StartTimer(Timer *timer, float duration) {
    timer->duration   = duration;
    timer->progress   = 0.0f;
    timer->isRunning = true;
}

// TODO: pass deltaTime here
void UpdateTimer(Timer *timer) {
    if (!timer->isRunning) return;

    timer->progress   += GetFrameTime();
    timer->isRunning  = timer->progress < timer->duration;
}

#endif