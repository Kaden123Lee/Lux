#pragma once
#include <windows.h>

void InitRenderer(HWND hwnd);
void CleanupRenderer();
void ResizeRenderer(HWND hwnd, int width, int height);
void RenderFrame(HWND hwnd);
