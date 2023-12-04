#pragma once
#include <stdint.h>
#include <thread>
#include <atomic> 
#include <cstdlib>
#include <iostream>
#include <windows.h>
#include <print>
#include <string>
#include "checkML.h"

#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH SCREEN_HEIGHT * 16/9
#define VSYNC true
#define FPS 120

#define TEXT_TIME 5.0f

class Player;
class Enemy;
class Boss;
class Chat;

class Button;

enum scene{menu, game}; //Different scenes of the game

class Game
{
private:
	void runNetwork();

	Player* p1 = nullptr;
	Enemy* p2 = nullptr;
	Boss* b = nullptr;
	Chat* c = nullptr;

	Button* buttonPlay = nullptr;
	Button* buttonExit = nullptr;
	Button* buttonRFull = nullptr;
	Button* buttonOLeft = nullptr;
	Button* buttonWin = nullptr;
	Button* buttonLose = nullptr;
	Button* buttonLMatchmaking = nullptr;
	Button* buttonCountry = nullptr;

	bool fullMessage = false;
	bool exitMessage = false;
	bool playPressed = false;
	bool win = false;
	bool lose = false;
	bool matchmakingB = false;
	bool country = false;

	float timer = 0;

	//--------TIME SETUP-------------
	float deltaTime = 0;
	float frametime = 0.f;

	// Limit FPS
	int frameDelay = 1000 / FPS; 
	uint32_t frame_start = 0, frame_time = 0;
	//-------------------------------

	scene actualScene = menu;
	//---------GAME SCENE--------
	void updateGame();
	void drawGame();
	//----------MENU SCENE-----------
	void updateMenu();
	void drawMenu();

	//Check messages
	std::thread netThread;
	std::atomic<bool> running;
public:
	Game();

	bool init();

	void run();

	~Game();
};

