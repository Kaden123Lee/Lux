#pragma once
#include <vector>

struct Target {
    float x, y, z;
    float radius;
    bool active;
};

extern std::vector<Target> targets;
extern int score;

void InitGame();
void SpawnTarget();
void HandleClick(int x, int y);
