#include "../Gameplay/TargetSystem.h"
#include "../Core/Config.h"
#include <cstdlib>
#include <cmath>

void SpawnCorrectionMode() {
    float playHalfW = 1.3f; 
    float minHeight = -0.6f;
    float maxHeight = 0.6f;

    Target t;
    t.radius = 0.4f; 
    t.vx = t.vy = t.vz = 0;
    t.health = 1.0f;
    t.maxHealth = 1.0f;
    t.movementTimer = 0;

    if (targets.empty()) {
        t.x = (float)(rand() % 2001 / 1000.0f - 1.0f) * playHalfW;
        t.y = minHeight + (float)(rand() % 1001 / 1000.0f) * (maxHeight - minHeight);
        t.z = (float)(rand() % 1001 / 1000.0f) + 1.0f;
    } else {
        float avgX = 0, avgY = 0, avgZ = 0;
        for(const auto& existing : targets) {
            avgX += existing.x; avgY += existing.y; avgZ += existing.z;
        }
        avgX /= targets.size(); avgY /= targets.size(); avgZ /= targets.size();

        bool valid = false;
        int tries = 0;
        while(!valid && tries < 50) {
            t.x = avgX + (rand() % 1001 / 1000.0f - 0.5f) * 0.8f;
            t.y = avgY + (rand() % 1001 / 1000.0f - 0.5f) * 0.8f;
            t.z = avgZ + (rand() % 1001 / 1000.0f - 0.5f) * 0.2f;
            
            if (t.x < -playHalfW) t.x = -playHalfW; if (t.x > playHalfW) t.x = playHalfW;
            if (t.y < minHeight) t.y = minHeight; if (t.y > maxHeight) t.y = maxHeight;
            if (t.z < 1.0f) t.z = 1.0f; if (t.z > 2.0f) t.z = 2.0f;

            valid = true;
            for(const auto& existing : targets) {
                float dx = t.x - existing.x;
                float dy = t.y - existing.y;
                if (sqrt(dx*dx + dy*dy) < 0.15f) { valid = false; break; }
            }
            tries++;
        }
    }
    
    t.active = true;
    targets.push_back(t);
}
