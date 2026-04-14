#include <windows.h>
#include <vector>
#include <string>
#include "Core/Config.h"
#include "Renderer/Renderer.h"
#include "Gameplay/TargetSystem.h"
#include "Core/GameLibrary.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE:
        ResizeRenderer(hwnd, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_CHAR:
        if (currentGameState == STATE_SETTINGS && isTypingSensitivity) {
            if (wParam == VK_BACK) {
                if (!sensitivityInput.empty()) sensitivityInput.pop_back();
            } else if (wParam == VK_RETURN) {
                try {
                    float val = std::stof(sensitivityInput);
                    float mult = GetGameMultiplier(currentSelectedGame);
                    float converted = val * mult;
                    if (converted < 0.0001f) converted = 0.0001f;
                    if (converted > 0.1f) converted = 0.1f;
                    mouseSensitivity = converted;
                    sensitivity = converted;
                } catch (...) {}
                isTypingSensitivity = false;
            } else if (wParam == VK_ESCAPE) {
                isTypingSensitivity = false;
            } else {
                char c = (char)wParam;
                if ((c >= '0' && c <= '9') || c == '.') {
                    sensitivityInput += c;
                }
            }
        }
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
        QueryPerformanceCounter(&currentTime);
        double elapsed = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;

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
                    camX += dx * mouseSensitivity;
                    camY += dy * mouseSensitivity;

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
            } else if (currentGameState == STATE_MENU) {
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
                if (GetAsyncKeyState('3') & 0x8000) {
                    currentGameMode = MODE_DYNAMIC;
                    InitGame(); 
                    currentGameState = STATE_PLAYING;
                    Sleep(200);
                }
                if (GetAsyncKeyState('4') & 0x8000) {
                    currentGameMode = MODE_TRACKING;
                    InitGame(); 
                    currentGameState = STATE_PLAYING;
                    Sleep(200);
                }
                if (GetAsyncKeyState('5') & 0x8000) {
                    currentGameMode = MODE_SWITCHING;
                    InitGame();
                    currentGameState = STATE_PLAYING;
                    Sleep(200);
                }
                if (GetAsyncKeyState('6') & 0x8000) {
                    currentGameState = STATE_SETTINGS;
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
            } else if (currentGameState == STATE_SETTINGS) {
                if (cursorHidden) {
                    ShowCursor(TRUE);
                    cursorHidden = false;
                }

                if (!isTypingSensitivity) {
                    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                        POINT m;
                        GetCursorPos(&m);
                        ScreenToClient(hwnd, &m);
                        
                        int cx = windowWidth / 2;
                        int cy = windowHeight / 2;

                        // Check if clicking sensitivity text/bar
                        if (m.x > cx - 110 && m.x < cx + 110 && m.y > cy - 40 && m.y < cy + 25) {
                            if (!isGameDropdownOpen) {
                                isTypingSensitivity = true;
                                float displayVal = mouseSensitivity;
                                if (currentSelectedGame != GAME_LUX) {
                                    displayVal = mouseSensitivity / GetGameMultiplier(currentSelectedGame);
                                }
                                sensitivityInput = std::to_string(displayVal);
                                size_t dotPos = sensitivityInput.find('.');
                                if (dotPos != std::string::npos) {
                                    sensitivityInput.erase(sensitivityInput.find_last_not_of('0') + 1, std::string::npos);
                                    if (sensitivityInput.back() == '.') sensitivityInput.pop_back();
                                }
                                Sleep(200);
                            }
                        }
                        // Check if clicking Game Header
                        else if (m.x > cx - 100 && m.x < cx + 100 && m.y > cy + 25 && m.y < cy + 55) {
                            isGameDropdownOpen = !isGameDropdownOpen;
                            Sleep(200);
                        }
                        // Check if clicking Options (if open)
                        else if (isGameDropdownOpen) {
                            if (m.x > cx - 100 && m.x < cx + 100) {
                                if (m.y > cy + 55 && m.y < cy + 85) {
                                    currentSelectedGame = GAME_LUX;
                                    isGameDropdownOpen = false;
                                    Sleep(200);
                                } else if (m.y > cy + 85 && m.y < cy + 115) {
                                    currentSelectedGame = GAME_VALORANT;
                                    isGameDropdownOpen = false;
                                    Sleep(200);
                                } else if (m.y > cy + 115 && m.y < cy + 145) {
                                    currentSelectedGame = GAME_CS2;
                                    isGameDropdownOpen = false;
                                    Sleep(200);
                                } else if (m.y > cy + 145 && m.y < cy + 175) {
                                    currentSelectedGame = GAME_OVERWATCH;
                                    isGameDropdownOpen = false;
                                    Sleep(200);
                                }
                            }
                        } else {
                            // Clicked outside, close dropdown
                            isGameDropdownOpen = false;
                        }
                    }
                    if (GetAsyncKeyState('T') & 0x8000) {
                        isTypingSensitivity = true;
                        float displayVal = mouseSensitivity;
                        if (currentSelectedGame != GAME_LUX) displayVal = mouseSensitivity / GetGameMultiplier(currentSelectedGame);
                        sensitivityInput = std::to_string(displayVal);
                        size_t dotPos = sensitivityInput.find('.');
                        if (dotPos != std::string::npos) {
                            sensitivityInput.erase(sensitivityInput.find_last_not_of('0') + 1, std::string::npos);
                            if (sensitivityInput.back() == '.') sensitivityInput.pop_back();
                        }
                        Sleep(200);
                    }
                }

                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    if (isTypingSensitivity) {
                        isTypingSensitivity = false;
                    } else {
                        currentGameState = STATE_MENU;
                    }
                    Sleep(200);
                }
            }
        }

        if (elapsed >= targetFrameTime) {
            if (currentGameState == STATE_PLAYING && GetActiveWindow() == hwnd) {
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    HandleClick(windowWidth / 2, windowHeight / 2, (float)elapsed);
                }
            }

            UpdateTargets((float)elapsed);
            RenderFrame(hwnd);
            lastTime = currentTime;
        } else {
            Sleep(0);
        }
    }

    CleanupRenderer();
    return 0;
}
