#include "Renderer.h"
#include "Config.h"
#include "MathUtils.h"
#include "TargetSystem.h"
#include <string>
#include <algorithm>
#include <vector>

// Double buffering globals
static HDC hBackBufferDC = NULL;
static HBITMAP hBackBufferBmp = NULL;
static HBITMAP hOldBmp = NULL;

// Particle system for 3D depth cue
struct Particle {
    float x, y, z;
};
static std::vector<Particle> starfield;

void CleanupRenderer() {
    if (hBackBufferDC) {
        SelectObject(hBackBufferDC, hOldBmp);
        DeleteObject(hBackBufferBmp);
        DeleteDC(hBackBufferDC);
        hBackBufferDC = NULL;
    }
}

void ResizeRenderer(HWND hwnd, int width, int height) {
    CleanupRenderer();
    HDC hdc = GetDC(hwnd);
    hBackBufferDC = CreateCompatibleDC(hdc);
    hBackBufferBmp = CreateCompatibleBitmap(hdc, width, height);
    hOldBmp = (HBITMAP)SelectObject(hBackBufferDC, hBackBufferBmp);
    ReleaseDC(hwnd, hdc);
    windowWidth = width;
    windowHeight = height;

    // Initialize Starfield (Static depth cues)
    if (starfield.empty()) {
        for (int i = 0; i < 200; ++i) {
            Particle p;
            p.x = (float)((rand() % 4001 / 1000.0f) - 2.0f) * 10.0f;
            p.y = (float)((rand() % 4001 / 1000.0f) - 2.0f) * 10.0f;
            p.z = (float)(rand() % 2001 / 1000.0f) * 10.0f + 5.0f;
            starfield.push_back(p);
        }
    }
}

void RenderFrame(HWND hwnd) {
    if (!hBackBufferDC) return;

    int cx = windowWidth / 2;
    int cy = windowHeight / 2;

    // 1. Draw Background (Depth Gradient)
    for (int i = 0; i < windowHeight; ++i) {
        float distFromCenter = abs(i - cy) / (float)(windowHeight / 2);
        int val = (int)(15 + 15 * (1.0f - distFromCenter));
        HPEN gradePen = CreatePen(PS_SOLID, 1, RGB(val, val, val + 5));
        SelectObject(hBackBufferDC, gradePen);
        MoveToEx(hBackBufferDC, 0, i, NULL);
        LineTo(hBackBufferDC, windowWidth, i);
        DeleteObject(gradePen);
    }
    
    // 2. Draw 3D Box (Super-Dense Wireframe)
    HPEN roomPen = CreatePen(PS_SOLID, 1, RGB(55, 55, 75));
    SelectObject(hBackBufferDC, roomPen);
    
    float sideX = roomWidth / 2.0f;
    
    // Draw 50 rings for an even deeper, high-precision perspective
    for (float gz = 0.5f; gz <= 50.0f; gz += 1.0f) {
        float s = 1.0f / (gz * 0.1f + 1.0f);
        int xL = cx + (int)((-sideX - camX * 2.0f) * (windowWidth/2) * s);
        int xR = cx + (int)((sideX - camX * 2.0f) * (windowWidth/2) * s);
        int yT = cy + (int)((ceilY - camY * 2.0f) * (windowHeight/2) * s);
        int yB = cy + (int)((groundY - camY * 2.0f) * (windowHeight/2) * s);

        // Draw the ring
        MoveToEx(hBackBufferDC, xL, yT, NULL);
        LineTo(hBackBufferDC, xR, yT);
        LineTo(hBackBufferDC, xR, yB);
        LineTo(hBackBufferDC, xL, yB);
        LineTo(hBackBufferDC, xL, yT);
    }



    // Perspective Lines for corners
    float corners[4][2] = {{-sideX, ceilY}, {sideX, ceilY}, {sideX, groundY}, {-sideX, groundY}};
    for (int i = 0; i < 4; ++i) {
        float sNear = 1.0f / (0.5f * 0.1f + 1.0f);
        float sFar = 1.0f / (50.0f * 0.1f + 1.0f);
        int x1 = cx + (int)((corners[i][0] - camX * 2.0f) * (windowWidth/2) * sNear);
        int y1 = cy + (int)((corners[i][1] - camY * 2.0f) * (windowHeight/2) * sNear);
        int x2 = cx + (int)((corners[i][0] - camX * 2.0f) * (windowWidth/2) * sFar);
        int y2 = cy + (int)((corners[i][1] - camY * 2.0f) * (windowHeight/2) * sFar);
        MoveToEx(hBackBufferDC, x1, y1, NULL);
        LineTo(hBackBufferDC, x2, y2);
    }

    DeleteObject(roomPen);

    // 3. Draw Stars (Deep Parallax)
    for (const auto& p : starfield) {
        float s = 1.0f / (p.z * 0.1f + 1.0f); 
        int sx = cx + (int)((p.x - camX * 2.0f) * (windowWidth/2) * s);
        int sy = cy + (int)((p.y - camY * 2.0f) * (windowHeight/2) * s);
        if (sx >= 0 && sx < windowWidth && sy >= 0 && sy < windowHeight) {
            SetPixel(hBackBufferDC, sx, sy, RGB(70, 70, 80));
        }
    }

    // 4. Draw Targets
    std::vector<Target> sortedTargets = targets;
    std::sort(sortedTargets.begin(), sortedTargets.end(), [](const Target& a, const Target& b) {
        return a.z > b.z; 
    });

    for (const auto& t : sortedTargets) {
        if (!t.active) continue;

        float depth = t.z * 15.0f + 2.0f; 
        float s = 1.0f / (depth * 0.1f + 1.0f);
        int sx = cx + (int)((t.x - camX) * (windowWidth/2) * s);
        int sy = cy + (int)((t.y - camY) * (windowHeight/2) * s);
        
        float r = (int)(16 * s); // Small for both modes

        int brightness = (int)(255 * s * 1.5f);
        if (brightness > 255) brightness = 255;
        
        HBRUSH shadowBrush = CreateSolidBrush(RGB(5, 5, 8));
        SelectObject(hBackBufferDC, GetStockObject(NULL_PEN));
        SelectObject(hBackBufferDC, shadowBrush);
        int shadowY = cy + (int)((groundY - camY * 2.0f) * (windowHeight/2) * s);
        Ellipse(hBackBufferDC, sx - r, shadowY - (r/4), sx + r, shadowY + (r/4));
        DeleteObject(shadowBrush);

        HBRUSH targetBrush = CreateSolidBrush(RGB(0, brightness, brightness));
        SelectObject(hBackBufferDC, targetBrush);
        Ellipse(hBackBufferDC, sx - r, sy - r, sx + r, sy + r);
        DeleteObject(targetBrush);
    }
    
    // 5. Crosshair (Red dot)
    SetPixel(hBackBufferDC, cx, cy, RGB(255, 0, 0));
    SetPixel(hBackBufferDC, cx-1, cy, RGB(255, 0, 0));
    SetPixel(hBackBufferDC, cx+1, cy, RGB(255, 0, 0));
    SetPixel(hBackBufferDC, cx, cy-1, RGB(255, 0, 0));
    SetPixel(hBackBufferDC, cx, cy+1, RGB(255, 0, 0));

    // 6. HUD / MENU OVERLAY
    if (currentGameState == STATE_MENU) {
        HBRUSH overlayBrush = CreateSolidBrush(RGB(5, 5, 12));
        RECT overlayRect = {0, 0, windowWidth, windowHeight};
        FillRect(hBackBufferDC, &overlayRect, overlayBrush);
        DeleteObject(overlayBrush);

        SetBkMode(hBackBufferDC, TRANSPARENT);
        SetTextColor(hBackBufferDC, RGB(255, 255, 255));
        
        const char* title = "LUX AIM TRAINER";
        const char* opt1 = "[1] PRECISION";
        const char* opt2 = "[2] CORRECTION (6-Sphere)";
        const char* prompt = "PRESS 1 OR 2 TO START";

        TextOutA(hBackBufferDC, cx - 60, cy - 80, title, (int)strlen(title));
        
        SetTextColor(hBackBufferDC, (currentGameMode == MODE_PRECISION ? RGB(0, 255, 255) : RGB(150, 150, 150)));
        TextOutA(hBackBufferDC, cx - 60, cy - 30, opt1, (int)strlen(opt1));
        
        SetTextColor(hBackBufferDC, (currentGameMode == MODE_CORRECTION ? RGB(0, 255, 255) : RGB(150, 150, 150)));
        TextOutA(hBackBufferDC, cx - 60, cy, opt2, (int)strlen(opt2));
        
        SetTextColor(hBackBufferDC, RGB(200, 200, 200));
        TextOutA(hBackBufferDC, cx - 75, cy + 50, prompt, (int)strlen(prompt));
    } else {
        SetBkMode(hBackBufferDC, TRANSPARENT);
        SetTextColor(hBackBufferDC, RGB(0, 200, 200));
        std::string modeName = (currentGameMode == MODE_PRECISION) ? "PRECISION" : "CORRECTION";
        std::string modeTxt = "MODE: " + modeName;
        TextOutA(hBackBufferDC, 10, 10, modeTxt.c_str(), (int)modeTxt.length());

        SetTextColor(hBackBufferDC, RGB(255, 255, 255));
        std::string scoreTxt = "SCORE: " + std::to_string(score);
        TextOutA(hBackBufferDC, 10, 30, scoreTxt.c_str(), (int)scoreTxt.length());
    }


    
    HDC hdc = GetDC(hwnd);
    BitBlt(hdc, 0, 0, windowWidth, windowHeight, hBackBufferDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}
