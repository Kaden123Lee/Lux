#include "TargetSystem.h"
#include "../Core/Config.h"
#include "../Core/MathUtils.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

std::vector<Target> targets;
// Iterate to add new mode
void SpawnTarget() {
    switch (currentGameMode) {
        case MODE_PRECISION:  SpawnPrecisionMode();  break;
        case MODE_CORRECTION: SpawnCorrectionMode(); break;
        case MODE_DYNAMIC:    SpawnDynamicMode();    break;
        case MODE_TRACKING:   SpawnTrackingMode();   break;
        case MODE_SWITCHING:  SpawnSwitchingMode();  break;
        
    }
}

void UpdateTargets(float deltaTime) {
    switch (currentGameMode) {
        case MODE_DYNAMIC:   UpdateDynamicMode(deltaTime);   break;
        case MODE_TRACKING:  UpdateTrackingMode(deltaTime);  break;
        case MODE_SWITCHING: UpdateSwitchingMode(deltaTime); break;
        default: break; 
    }
}

void InitGame() {
    srand((unsigned int)time(NULL));
    targets.clear();
    score = 0;
    
    int count = 4;
    if (currentGameMode == MODE_CORRECTION) count = 6;
    if (currentGameMode == MODE_DYNAMIC)    count = 5;
    if (currentGameMode == MODE_TRACKING)   count = 1;
    if (currentGameMode == MODE_SWITCHING)  count = 4;

    for (int i = 0; i < count; ++i) {
        SpawnTarget();
    }
}

void HandleClick(int x, int y, float deltaTime) {
    int cx = windowWidth / 2;
    int cy = windowHeight / 2;

    for (size_t i = 0; i < targets.size(); ++i) {
        if (targets[i].active) {
            float depth = targets[i].z * 15.0f + 2.0f; 
            float s = 1.0f / (depth * 0.1f + 1.0f);
            
            float sx = (float)cx + (targets[i].x - camX * 2.0f) * (windowWidth / 2) * s;
            float sy = (float)cy + (targets[i].y - camY * 2.0f) * (windowHeight / 2) * s;
            
            float baseR = 16.0f;
            if (currentGameMode == MODE_DYNAMIC)  baseR = 48.0f;
            if (currentGameMode == MODE_TRACKING || currentGameMode == MODE_SWITCHING) baseR = 144.0f;
            float r = baseR * s; 

            float dx = (float)x - sx;
            float dy = (float)y - sy;
            if ((dx * dx + dy * dy) <= (r * r)) {
                if (currentGameMode == MODE_TRACKING || currentGameMode == MODE_SWITCHING) {
                    targets[i].health -= deltaTime;
                    score += 1;
                    if (targets[i].health <= 0) {
                        targets.erase(targets.begin() + i);
                        score += (currentGameMode == MODE_TRACKING) ? 500 : 200;
                        SpawnTarget();
                    }
                } else {
                    targets.erase(targets.begin() + i);
                    score++;
                    SpawnTarget();
                }
                break;
            }
        }
    }
}
