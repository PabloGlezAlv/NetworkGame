#include <stdio.h>
#include <string>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

#include "NetworkManager.h"
#include "Boss.h"


#define SCREEN_WIDTH 1066
#define SCREEN_HEIGHT 600

int main(int argc, char* args[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
		return 1;
	}

	if (!NetworkManager::Instance()->Init())
	{
		return 0;
	}

	srand(time(0));

	/* Main loop */ 
	bool quit = false; 
	bool matchStarted = false;

	BossBehaviour boss = BossBehaviour(100,100,250.f, SCREEN_WIDTH);

	//--------Delta time-------------
	float lastDraw = 0;
	float deltaTime = 0;
	float fps = 0;
	float frametime = 0.f;

	bool inGame = false;

	while (!quit) 
	{ 
		lastDraw = SDL_GetTicks();

		NetworkManager::Instance()->CheckMessagesUDP();
		NetworkManager::Instance()->CheckMessagesTCP();

		NetworkManager::Instance()->Update(deltaTime);

		if (NetworkManager::Instance()->inGame())
		{
			if (inGame == false) inGame = true;

			boss.update(deltaTime);
		}
		else if (inGame) //Last update of the boss
		{
			inGame = false;

			boss = BossBehaviour(100, 100, 250.f, SCREEN_WIDTH);
		}

		deltaTime = ((float)((SDL_GetTicks() - lastDraw)) / 1000.f);
		fps++;
		frametime += deltaTime;
	}

	NetworkManager::Instance()->Release();

	SDL_Quit();
	return 0;
}