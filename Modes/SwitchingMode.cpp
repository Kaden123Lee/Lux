#include "../Gameplay/TargetSystem.h"
#include "../Core/Config.h"
#include <cstdlib>
#include <cmath>

void SpawnSwitchingMode() {
    float playHalfW = 1.3f; 
    float maxHeight = 0.6f;
    Target t;
    t.radius = 0.5f; 
    t.health = 1.5f; // Half of Tracking mode (3.0 / 2 = 1.5)
    t.maxHealth = 1.5f;
    t.movementTimer = 0;
    
    t.x = (float)(rand() % 2001 / 1000.0f - 1.0f) * playHalfW;
    t.y = (float)(rand() % 2001 / 1000.0f - 1.0f) * maxHeight;
    t.z = (float)(rand() % 1001 / 1000.0f) + 1.0f;
    
    float angle = (float)(rand() % 360) * PI / 180.0f;
    t.vx = cos(angle) * 2.0f;
    t.vy = sin(angle) * 2.0f;
    t.vz = 0;
    
    t.active = true;
    targets.push_back(t);
}

void UpdateSwitchingMode(float deltaTime) {
    // Exactly same as Tracking mode movement
    const float playHalfW = roomWidth / 2.5f;
    const float playHalfH = roomHeight / 2.5f;
    for (auto& t : targets) {
        if (!t.active) continue;
        t.movementTimer -= deltaTime;
        if (t.movementTimer <= 0) {
            float angle = (float)(rand() % 360) * PI / 180.0f;
            float speed = 2.0f + (rand() % 1000 / 1000.0f) * 2.0f;
            t.vx = cos(angle) * speed;
            t.vy = sin(angle) * speed;
            t.movementTimer = 0.3f + (rand() % 1000 / 1000.0f) * 0.7f;
        }
        t.x += t.vx * deltaTime;
        t.y += t.vy * deltaTime;
        if (t.x < -playHalfW) { t.x = -playHalfW; t.vx = abs(t.vx); t.movementTimer = 0; }
        if (t.x > playHalfW) { t.x = playHalfW; t.vx = -abs(t.vx); t.movementTimer = 0; }
        if (t.y < -playHalfH) { t.y = -playHalfH; t.vy = abs(t.vy); t.movementTimer = 0; }
        if (t.y > playHalfH) { t.y = playHalfH; t.vy = -abs(t.vy); t.movementTimer = 0; }
    }
}
