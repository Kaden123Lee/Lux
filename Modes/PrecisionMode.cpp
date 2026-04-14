#include "../Gameplay/TargetSystem.h"
#include "../Core/Config.h"
#include <cstdlib>

void SpawnPrecisionMode() {
    float playHalfW = 1.3f; 
    float minHeight = -0.6f;
    float maxHeight = 0.6f;

    Target t;
    t.radius = 0.4f; 
    t.vx = t.vy = t.vz = 0;
    t.health = 1.0f;
    t.maxHealth = 1.0f;
    t.movementTimer = 0;
    
    t.x = (float)(rand() % 2001 / 1000.0f - 1.0f) * playHalfW;
    t.y = minHeight + (float)(rand() % 1001 / 1000.0f) * (maxHeight - minHeight);
    t.z = (float)(rand() % 1001 / 1000.0f) + 1.0f;
    
    t.active = true;
    targets.push_back(t);
}
