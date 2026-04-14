#pragma once
#include <vector>

struct Target {
    float x, y, z;
    float vx, vy, vz;
    float health;
    float maxHealth;
    float movementTimer;
    float radius;
    bool active;
};

extern std::vector<Target> targets;
extern int score;

void InitGame();
void SpawnTarget();
void UpdateTargets(float deltaTime);
void HandleClick(int x, int y, float deltaTime);

// Mode-specific functions (defined in Modes/ directory)
void SpawnPrecisionMode();
void SpawnCorrectionMode();
void SpawnDynamicMode();
void SpawnTrackingMode();
void SpawnSwitchingMode();

void UpdateDynamicMode(float deltaTime);
void UpdateTrackingMode(float deltaTime);
void UpdateSwitchingMode(float deltaTime);
