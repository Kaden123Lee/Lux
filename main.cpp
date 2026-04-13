#include <windows.h>
#include <vector>
#include <string>
#include "Config.h"
#include "Renderer.h"
#include "TargetSystem.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE:
        ResizeRenderer(hwnd, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    const char CLASS_NAME[] = "LuxAimTrainer";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Lux Aim Trainer", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nShowCmd);

    InitGame();

    MSG msg = {};
    bool isRunning = true;

    LARGE_INTEGER frequency, lastTime, currentTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastTime);
    double targetFrameTime = 1.0 / 144.0;

    while (isRunning) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) isRunning = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (GetActiveWindow() == hwnd) {
            RECT rect;
            GetWindowRect(hwnd, &rect);

            if (currentGameState == STATE_PLAYING) {
                if (!cursorHidden) {
                    ShowCursor(FALSE);
                    cursorHidden = true;
                }

                POINT mousePos;
                GetCursorPos(&mousePos);
                int centerX = rect.left + windowWidth / 2;
                int centerY = rect.top + windowHeight / 2;

                int dx = mousePos.x - centerX;
                int dy = mousePos.y - centerY;

                if (dx != 0 || dy != 0) {
                    camX += dx * sensitivity;
                    camY += dy * sensitivity;

                    // Clamping
                    if (camX < -1.5f) camX = -1.5f;
                    if (camX > 1.5f) camX = 1.5f;
                    if (camY < -0.9f) camY = -0.9f;
                    if (camY > 0.9f) camY = 0.9f;

                    SetCursorPos(centerX, centerY);
                }

                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    currentGameState = STATE_MENU;
                    Sleep(200); 
                }
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    HandleClick(windowWidth / 2, windowHeight / 2);
                }
            } else {
                // MENU STATE
                if (cursorHidden) {
                    ShowCursor(TRUE);
                    cursorHidden = false;
                }

                if (GetAsyncKeyState('1') & 0x8000) {
                    currentGameMode = MODE_PRECISION;
                    InitGame(); 
                    currentGameState = STATE_PLAYING;
                    Sleep(200);
                }
                if (GetAsyncKeyState('2') & 0x8000) {
                    currentGameMode = MODE_CORRECTION;
                    InitGame(); 
                    currentGameState = STATE_PLAYING;
                    Sleep(200);
                }

                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    InitGame();
                    currentGameState = STATE_PLAYING;
                    Sleep(200);
                }
                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    isRunning = false;
                }

            }
        }

        QueryPerformanceCounter(&currentTime);
        double elapsed = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;

        if (elapsed >= targetFrameTime) {
            RenderFrame(hwnd);
            lastTime = currentTime;
        } else {
            Sleep(0);
        }
    }

    CleanupRenderer();
    return 0;
}
