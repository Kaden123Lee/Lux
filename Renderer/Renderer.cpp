#include "Renderer.h"
#include "../Core/Config.h"
#include "../Core/MathUtils.h"
#include "../Core/GameLibrary.h"
#include "../Gameplay/TargetSystem.h"
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
        if (hOldBmp) SelectObject(hBackBufferDC, hOldBmp);
        if (hBackBufferBmp) DeleteObject(hBackBufferBmp);
        DeleteDC(hBackBufferDC);
        hBackBufferDC = NULL;
        hBackBufferBmp = NULL;
        hOldBmp = NULL;
    }
}

void ResizeRenderer(HWND hwnd, int width, int height) {
    if (width <= 0 || height <= 0) return;
    CleanupRenderer();
    HDC hdc = GetDC(hwnd);
    hBackBufferDC = CreateCompatibleDC(hdc);
    hBackBufferBmp = CreateCompatibleBitmap(hdc, width, height);
    hOldBmp = (HBITMAP)SelectObject(hBackBufferDC, hBackBufferBmp);
    ReleaseDC(hwnd, hdc);
    windowWidth = width;
    windowHeight = height;

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

static void DrawGradient(HDC hdc, int w, int h, int cy) {
    int steps = 64;
    int stepH = h / steps + 1;
    for (int i = 0; i < steps; ++i) {
        int y = i * stepH;
        float distFromCenter = abs((y + stepH / 2) - cy) / (float)(h / 2);
        if (distFromCenter > 1.0f) distFromCenter = 1.0f;
        int val = (int)(15 + 15 * (1.0f - distFromCenter));
        
        HBRUSH hBrush = CreateSolidBrush(RGB(val, val, val + 5));
        RECT r = { 0, y, w, y + stepH };
        FillRect(hdc, &r, hBrush);
        DeleteObject(hBrush);
    }
}

void RenderFrame(HWND hwnd) {
    if (!hBackBufferDC) return;

    int cx = windowWidth / 2;
    int cy = windowHeight / 2;

    // 1. Draw Background
    DrawGradient(hBackBufferDC, windowWidth, windowHeight, cy);
    
    // 2. Draw 3D Box (Wireframe)
    HPEN roomPen = CreatePen(PS_SOLID, 1, RGB(55, 55, 75));
    HGDIOBJ hOldPen = SelectObject(hBackBufferDC, roomPen);
    
    float sideX = roomWidth / 2.0f;
    
    for (float gz = 0.5f; gz <= 50.0f; gz += 2.0f) {
        float s = 1.0f / (gz * 0.1f + 1.0f);
        int xL = cx + (int)((-sideX - camX * 2.0f) * (windowWidth/2) * s);
        int xR = cx + (int)((sideX - camX * 2.0f) * (windowWidth/2) * s);
        int yT = cy + (int)((ceilY - camY * 2.0f) * (windowHeight/2) * s);
        int yB = cy + (int)((groundY - camY * 2.0f) * (windowHeight/2) * s);

        MoveToEx(hBackBufferDC, xL, yT, NULL);
        LineTo(hBackBufferDC, xR, yT);
        LineTo(hBackBufferDC, xR, yB);
        LineTo(hBackBufferDC, xL, yB);
        LineTo(hBackBufferDC, xL, yT);
    }

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

    SelectObject(hBackBufferDC, hOldPen);
    DeleteObject(roomPen);

    // 3. Draw Stars
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
        int sx = cx + (int)((t.x - camX * 2.0f) * (windowWidth / 2) * s);
        int sy = cy + (int)((t.y - camY * 2.0f) * (windowHeight / 2) * s);
        
        float baseR = 16.0f;
        if (currentGameMode == MODE_DYNAMIC) baseR = 48.0f;
        if (currentGameMode == MODE_TRACKING || currentGameMode == MODE_SWITCHING) baseR = 144.0f;
        float r = (baseR * s);

        int brightness = (int)(255 * s * 1.5f);
        if (brightness > 255) brightness = 255;
        
        HBRUSH shadowBrush = CreateSolidBrush(RGB(5, 5, 8));
        HGDIOBJ hOldObj = SelectObject(hBackBufferDC, GetStockObject(NULL_PEN));
        SelectObject(hBackBufferDC, shadowBrush);
        int shadowY = cy + (int)((groundY - camY * 2.0f) * (windowHeight/2) * s);
        Ellipse(hBackBufferDC, sx - (int)r, shadowY - (int)r/4, sx + (int)r, shadowY + (int)r/4);
        
        HBRUSH targetBrush = CreateSolidBrush(RGB(0, brightness, brightness));
        SelectObject(hBackBufferDC, targetBrush);
        Ellipse(hBackBufferDC, sx - (int)r, sy - (int)r, sx + (int)r, sy + (int)r);
        
        // Health Bar for Tracking / Switching
        if (currentGameMode == MODE_TRACKING || currentGameMode == MODE_SWITCHING) {
            int barWidth = (int)(r * 2.0f);
            int barHeight = 6;
            int barY = sy - (int)r - 12;
            
            // Background
            HBRUSH bgBrush = CreateSolidBrush(RGB(40, 40, 40));
            RECT bgRect = { sx - barWidth / 2, barY, sx + barWidth / 2, barY + barHeight };
            FillRect(hBackBufferDC, &bgRect, bgBrush);
            DeleteObject(bgBrush);
            
            // Health
            float hpPercent = t.health / t.maxHealth;
            if (hpPercent < 0) hpPercent = 0;
            HBRUSH hpBrush = CreateSolidBrush(RGB(0, 255, 100));
            RECT hpRect = { sx - barWidth / 2, barY, sx - barWidth / 2 + (int)(barWidth * hpPercent), barY + barHeight };
            FillRect(hBackBufferDC, &hpRect, hpBrush);
            DeleteObject(hpBrush);
        }
        SelectObject(hBackBufferDC, hOldObj);
        DeleteObject(shadowBrush);
        DeleteObject(targetBrush);
    }
    
    // 5. Crosshair
    HPEN crossPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    HGDIOBJ hOldCrossPen = SelectObject(hBackBufferDC, crossPen);
    MoveToEx(hBackBufferDC, cx - 8, cy, NULL); LineTo(hBackBufferDC, cx + 9, cy);
    MoveToEx(hBackBufferDC, cx, cy - 8, NULL); LineTo(hBackBufferDC, cx, cy + 9);
    SelectObject(hBackBufferDC, hOldCrossPen);
    DeleteObject(crossPen);

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
        const char* opt3 = "[3] DYNAMIC (B180)";
        const char* opt4 = "[4] TRACKING (Smoothbot)";
        const char* opt5 = "[5] SWITCHING (PatTargetSwitch)";
        const char* prompt = "PRESS 1, 2, 3, 4 OR 5 TO START";

        TextOutA(hBackBufferDC, cx - 60, cy - 80, title, (int)strlen(title));
        
        SetTextColor(hBackBufferDC, (currentGameMode == MODE_PRECISION ? RGB(0, 255, 255) : RGB(150, 150, 150)));
        TextOutA(hBackBufferDC, cx - 60, cy - 30, opt1, (int)strlen(opt1));
        
        SetTextColor(hBackBufferDC, (currentGameMode == MODE_CORRECTION ? RGB(0, 255, 255) : RGB(150, 150, 150)));
        TextOutA(hBackBufferDC, cx - 60, cy, opt2, (int)strlen(opt2));

        SetTextColor(hBackBufferDC, (currentGameMode == MODE_DYNAMIC ? RGB(0, 255, 255) : RGB(150, 150, 150)));
        TextOutA(hBackBufferDC, cx - 60, cy + 30, opt3, (int)strlen(opt3));

        SetTextColor(hBackBufferDC, (currentGameMode == MODE_TRACKING ? RGB(0, 255, 255) : RGB(150, 150, 150)));
        TextOutA(hBackBufferDC, cx - 60, cy + 60, opt4, (int)strlen(opt4));

        SetTextColor(hBackBufferDC, (currentGameMode == MODE_SWITCHING ? RGB(0, 255, 255) : RGB(150, 150, 150)));
        TextOutA(hBackBufferDC, cx - 60, cy + 90, opt5, (int)strlen(opt5));
        
        SetTextColor(hBackBufferDC, RGB(200, 200, 200));
        TextOutA(hBackBufferDC, cx - 90, cy + 130, prompt, (int)strlen(prompt));

        const char* opt6 = "[6] SETTINGS";
        SetTextColor(hBackBufferDC, RGB(100, 100, 255));
        TextOutA(hBackBufferDC, 10, windowHeight - 30, opt6, (int)strlen(opt6));
    } else if (currentGameState == STATE_SETTINGS) {
        HBRUSH overlayBrush = CreateSolidBrush(RGB(5, 5, 12));
        RECT overlayRect = {0, 0, windowWidth, windowHeight};
        FillRect(hBackBufferDC, &overlayRect, overlayBrush);
        DeleteObject(overlayBrush);

        SetBkMode(hBackBufferDC, TRANSPARENT);
        SetTextColor(hBackBufferDC, RGB(255, 255, 255));

        const char* title = "SETTINGS";
        TextOutA(hBackBufferDC, cx - 30, cy - 100, title, (int)strlen(title));

        std::string sensTxt;
        if (isTypingSensitivity) {
            sensTxt = gameLibrary[currentSelectedGame].name + " Sens: " + sensitivityInput + "_";
            SetTextColor(hBackBufferDC, RGB(0, 255, 255));
        } else {
            float displayVal = mouseSensitivity;
            if (currentSelectedGame != GAME_LUX) {
                displayVal = mouseSensitivity / GetGameMultiplier(currentSelectedGame);
            }
            sensTxt = gameLibrary[currentSelectedGame].name + " Sens: " + std::to_string(displayVal);
            // Cleaner float display
            size_t dotPos = sensTxt.find('.');
            if (dotPos != std::string::npos) {
                sensTxt.erase(sensTxt.find_last_not_of('0') + 1, std::string::npos);
                if (sensTxt.back() == '.') sensTxt.pop_back();
            }
        }
        TextOutA(hBackBufferDC, cx - 60, cy - 30, sensTxt.c_str(), (int)sensTxt.length());

        // Game Selection Dropdown
        int ddX = cx - 100;
        int ddY = cy + 30;
        int ddW = 200;
        int ddH = 30;

        // Draw Header
        HBRUSH headerBrush = CreateSolidBrush(RGB(20, 20, 40));
        HPEN borderPen = CreatePen(PS_SOLID, 1, isGameDropdownOpen ? RGB(0, 255, 255) : RGB(100, 100, 150));
        HGDIOBJ hOldB = SelectObject(hBackBufferDC, headerBrush);
        HGDIOBJ hOldP = SelectObject(hBackBufferDC, borderPen);
        
        Rectangle(hBackBufferDC, ddX, ddY, ddX + ddW, ddY + ddH);

        SetTextColor(hBackBufferDC, RGB(255, 255, 255));
        std::string currentLabel = "Game: " + gameLibrary[currentSelectedGame].name;
        TextOutA(hBackBufferDC, ddX + 10, ddY + 7, currentLabel.c_str(), (int)currentLabel.length());
        
        // Small Arrow
        TextOutA(hBackBufferDC, ddX + ddW - 25, ddY + 7, isGameDropdownOpen ? "^" : "v", 1);

        if (isGameDropdownOpen) {
            // Draw Options
            int optY = ddY + ddH;
            GameType types[] = { GAME_LUX, GAME_VALORANT, GAME_CS2, GAME_OVERWATCH };
            
            for (int i = 0; i < 4; ++i) {
                HBRUSH optBrush = CreateSolidBrush(RGB(30 + i*5, 30 + i*5, 60 + i*5));
                SelectObject(hBackBufferDC, optBrush);
                Rectangle(hBackBufferDC, ddX, optY, ddX + ddW, optY + ddH);
                
                SetTextColor(hBackBufferDC, currentSelectedGame == types[i] ? RGB(0, 255, 255) : RGB(200, 200, 200));
                std::string name = gameLibrary[types[i]].name;
                TextOutA(hBackBufferDC, ddX + 10, optY + 7, name.c_str(), (int)name.length());
                
                optY += ddH;
                DeleteObject(optBrush);
            }
        }

        SelectObject(hBackBufferDC, hOldB);
        SelectObject(hBackBufferDC, hOldP);
        DeleteObject(headerBrush);
        DeleteObject(borderPen);

        // Draw Sensitivity Bar
        int barWidth = 200;
        int barHeight = 20;
        int barX = cx - barWidth / 2;
        int barY = cy;

        // Background
        HBRUSH bgBrush = CreateSolidBrush(RGB(40, 40, 40));
        RECT bgRect = { barX, barY, barX + barWidth, barY + barHeight };
        FillRect(hBackBufferDC, &bgRect, bgBrush);
        DeleteObject(bgBrush);

        // Fill based on current sensitivity (map 0-0.05 to 0-barWidth)
        float currentDisplaySens = mouseSensitivity;
        if (isTypingSensitivity && currentSelectedGame == GAME_LUX) {
            try { currentDisplaySens = std::stof(sensitivityInput); } catch(...) {}
        }
        float fillPercent = currentDisplaySens / 0.05f;
        if (fillPercent > 1.0f) fillPercent = 1.0f;
        if (fillPercent < 0.0f) fillPercent = 0.0f;
        
        HBRUSH fillBrush = CreateSolidBrush(isTypingSensitivity ? RGB(255, 255, 0) : RGB(0, 255, 255));
        RECT fillRect = { barX, barY, barX + (int)(barWidth * fillPercent), barY + barHeight };
        FillRect(hBackBufferDC, &fillRect, fillBrush);
        DeleteObject(fillBrush);

        if (isTypingSensitivity) {
            const char* help = "Type value and press ENTER to apply";
            const char* back = "Press ESC to cancel";
            SetTextColor(hBackBufferDC, RGB(200, 200, 200));
            TextOutA(hBackBufferDC, cx - 110, cy + 100, help, (int)strlen(help));
            TextOutA(hBackBufferDC, cx - 65, cy + 130, back, (int)strlen(back));
        } else {
            const char* help = "Click values to change / Press T to type";
            const char* back = "Press ESC to return to menu";
            SetTextColor(hBackBufferDC, RGB(200, 200, 200));
            TextOutA(hBackBufferDC, cx - 110, cy + 100, help, (int)strlen(help));
            TextOutA(hBackBufferDC, cx - 75, cy + 130, back, (int)strlen(back));
        }
    } else {
        SetBkMode(hBackBufferDC, TRANSPARENT);
        SetTextColor(hBackBufferDC, RGB(0, 200, 200));
        std::string modeTxt = "MODE: ";
        if (currentGameMode == MODE_PRECISION) modeTxt += "PRECISION";
        else if (currentGameMode == MODE_CORRECTION) modeTxt += "CORRECTION";
        else if (currentGameMode == MODE_DYNAMIC) modeTxt += "DYNAMIC";
        else if (currentGameMode == MODE_TRACKING) modeTxt += "TRACKING";
        else modeTxt += "SWITCHING";
        TextOutA(hBackBufferDC, 10, 10, modeTxt.c_str(), (int)modeTxt.length());

        SetTextColor(hBackBufferDC, RGB(255, 255, 255));
        std::string scoreTxt = "SCORE: " + std::to_string(score);
        TextOutA(hBackBufferDC, 10, 30, scoreTxt.c_str(), (int)scoreTxt.length());
    }

    HDC hdc = GetDC(hwnd);
    BitBlt(hdc, 0, 0, windowWidth, windowHeight, hBackBufferDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}
