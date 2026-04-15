#include "Config.h"

float camX = 0.0f;
float camY = 0.0f;
float sensitivity = 0.002f;
float mouseSensitivity = 0.002f;
bool cursorHidden = false;

GameState currentGameState = STATE_MENU;
GameMode currentGameMode = MODE_PRECISION;
GameType currentSelectedGame = GAME_LUX;
int score = 0;
std::string sensitivityInput = "";
bool isTypingSensitivity = false;
bool isGameDropdownOpen = false;


int windowWidth = 800;
int windowHeight = 600;

int ballColorR = 0;
int ballColorG = 255;
int ballColorB = 255;
