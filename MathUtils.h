#pragma once
#include "Config.h"
#include <cmath>

inline void Project(float x, float y, float z, int winW, int winH, float& screenX, float& screenY) {
    float aspect = (float)winW / (float)winH;
    float fovRad = fovDeg * (PI / 180.0f);
    float scale = 1.0f / tan(fovRad / 2.0f);
    float ndcX = (x * scale) / z;
    float ndcY = (y * scale) / z;
    screenX = (ndcX * aspect + 1.0f) * 0.5f * winW;
    screenY = (1.0f - ndcY) * 0.5f * winH;
}
