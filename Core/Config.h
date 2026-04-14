#pragma once
#include <windows.h>

const float wallZ = 20.0f;
const float roomWidth = 6.0f; 
const float roomHeight = 4.0f;
const float groundY = 1.8f;
const float ceilY = -1.8f;
const float backWallScale = 0.3f;

const float fovDeg = 90.0f;
const float PI = 3.14159265358979323846f;

// Crosshair and Input globals
extern float camX;
extern float camY;
extern float sensitivity;
extern bool cursorHidden;

// Game States and Modes
enum GameState { STATE_MENU, STATE_PLAYING };
enum GameMode { MODE_PRECISION, MODE_CORRECTION, MODE_DYNAMIC, MODE_TRACKING, MODE_SWITCHING };

extern GameState currentGameState;
extern GameMode currentGameMode;
extern int score;


// Window size
extern int windowWidth;
extern int windowHeight;
