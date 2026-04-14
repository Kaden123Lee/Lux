#include "../Gameplay/TargetSystem.h"
#include "../Core/Config.h"
#include <cstdlib>
#include <cmath>

void SpawnDynamicMode() {
    float playHalfW = 1.3f; 
    Target t;
    t.radius = 0.4f; 
    t.health = 1.0f;
    t.maxHealth = 1.0f;
    t.movementTimer = 0;
    
    t.x = (float)(rand() % 2001 / 1000.0f - 1.0f) * playHalfW;
    t.y = groundY - 0.5f; 
    t.z = (float)(rand() % 1001 / 1000.0f) + 1.0f;
    t.vx = (float)(rand() % 2001 / 1000.0f - 1.0f) * 1.5f;
    t.vy = -(float)(rand() % 1001 / 1000.0f) * 3.0f; 
    t.vz = 0;
    
    t.active = true;
    targets.push_back(t);
}

void UpdateDynamicMode(float deltaTime) {
    const float gravity = 9.8f;
    const float playHalfW = roomWidth / 4.0f; 
    for (auto& t : targets) {
        if (!t.active) continue;
        t.vy += gravity * deltaTime;
        t.x += t.vx * deltaTime;
        t.y += t.vy * deltaTime;
        if (t.y > (groundY - 0.1f)) {
            t.y = groundY - 0.1f;
            t.vy = -abs(t.vy) * 1.0f; 
            if (abs(t.vy) < 4.0f) t.vy = -6.0f; 
        }
        if (t.y < (ceilY + 0.1f)) { t.y = ceilY + 0.1f; t.vy = abs(t.vy); }
        if (t.x < -playHalfW || t.x > playHalfW) { t.vx = -t.vx; t.x = (t.x < 0) ? -playHalfW : playHalfW; }
    }
}
