#include "TargetSystem.h"
#include "Config.h"
#include "MathUtils.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

std::vector<Target> targets;


void SpawnTarget() {
    Target t;
    t.radius = 0.4f; // Uniform size for both modes

    float playHalfW = 1.3f; 
    float minHeight = -0.6f;
    float maxHeight = 0.6f;

    if (currentGameMode == MODE_CORRECTION && !targets.empty()) {
        // Find the center of existing targets to keep the cluster together
        float avgX = 0, avgY = 0, avgZ = 0;
        for(const auto& existing : targets) {
            avgX += existing.x; avgY += existing.y; avgZ += existing.z;
        }
        avgX /= targets.size(); avgY /= targets.size(); avgZ /= targets.size();

        // Spawn nearby but not too close
        bool valid = false;
        int tries = 0;
        while(!valid && tries < 50) {
            t.x = avgX + (rand() % 1001 / 1000.0f - 0.5f) * 0.8f;
            t.y = avgY + (rand() % 1001 / 1000.0f - 0.5f) * 0.8f;
            t.z = avgZ + (rand() % 1001 / 1000.0f - 0.5f) * 0.2f;
            
            // Constrain to room
            if (t.x < -playHalfW) t.x = -playHalfW; if (t.x > playHalfW) t.x = playHalfW;
            if (t.y < minHeight) t.y = minHeight; if (t.y > maxHeight) t.y = maxHeight;
            if (t.z < 0.0f) t.z = 0.0f; if (t.z > 1.0f) t.z = 1.0f;

            valid = true;
            for(const auto& existing : targets) {
                float dx = t.x - existing.x;
                float dy = t.y - existing.y;
                if (sqrt(dx*dx + dy*dy) < 0.15f) { valid = false; break; }
            }
            tries++;
        }
    } else {
        // Random spawn (Precision mode or first Correction target)
        t.x = (float)(rand() % 2001 / 1000.0f - 1.0f) * playHalfW;
        t.y = minHeight + (float)(rand() % 1001 / 1000.0f) * (maxHeight - minHeight);
        t.z = (float)(rand() % 1001 / 1000.0f);
    }
    
    t.active = true;
    targets.push_back(t);
}

void InitGame() {
    srand((unsigned int)time(NULL));
    targets.clear();
    score = 0;
    int count = (currentGameMode == MODE_CORRECTION) ? 6 : 4;
    for (int i = 0; i < count; ++i) {
        SpawnTarget();
    }
}

void HandleClick(int x, int y) {
    int cx = windowWidth / 2;
    int cy = windowHeight / 2;

    for (size_t i = 0; i < targets.size(); ++i) {
        if (targets[i].active) {
            float depth = targets[i].z * 15.0f + 2.0f; 
            float s = 1.0f / (depth * 0.1f + 1.0f);
            
            float sx = (float)cx + (targets[i].x - camX) * (windowWidth/2) * s;
            float sy = (float)cy + (targets[i].y - camY) * (windowHeight/2) * s;
            
            float r = 16.0f * s; // Small for both modes now

            float dx = (float)x - sx;
            float dy = (float)y - sy;
            if ((dx * dx + dy * dy) <= (r * r)) {
                targets.erase(targets.begin() + i);
                score++;
                SpawnTarget();
                break;
            }
        }
    }
}



