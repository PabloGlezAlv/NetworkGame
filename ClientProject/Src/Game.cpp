#include "Game.h"
#include "Renderer.h"
#include "NetworkManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Button.h"
#include "Boss.h"
#include "chat.h"


Game::Game()
{
	
}

bool Game::init()
{
	if (!Renderer::Instance()->Init(VSYNC, SCREEN_WIDTH, SCREEN_HEIGHT))
	{
		printf("Error creating the Renderer\n");
		return false;
	}
	if (!NetworkManager::Instance()->Init())
	{
		printf("Error creating the NetworkManager\n");
		return false;
	}
	//--------THREAD---------------------
	running = true;
	netThread = std::thread(&Game::runNetwork, this);

	//----------TIME-------------------------
	deltaTime = 0;
	frametime = 0.f;

	//----------ENTITIES-------------

	p1 = new Player(SCREEN_WIDTH / 4, SCREEN_HEIGHT * 2 / 3, 160, 100);

	b = new Boss(0, SCREEN_HEIGHT * 2 / 3, 160, 100, p1);

	p2 = new Enemy(0, SCREEN_HEIGHT * 2 / 3, 160, 100, p1, b);

	c = new Chat();

	std::string route = "../assets/playButton.png";
	buttonPlay = new Button(SCREEN_WIDTH / 3, SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 8, route.c_str(), true);

	std::string route2 = "../assets/exitPlay.png";
	buttonExit = new Button(SCREEN_WIDTH / 3, SCREEN_HEIGHT * 2 / 3 + SCREEN_HEIGHT / 8 + 10, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 8, route2.c_str(), true);

	std::string route3 = "../assets/rFull.png";
	buttonRFull = new Button(SCREEN_WIDTH / 6, (SCREEN_HEIGHT * 2 / 3) - 50, SCREEN_WIDTH * 3 / 5, SCREEN_HEIGHT / 15, route3.c_str(), false);

	std::string route4 = "../assets/oLeft.png";
	buttonOLeft = new Button(SCREEN_WIDTH / 6, (SCREEN_HEIGHT * 2 / 3) - 50, SCREEN_WIDTH * 3 / 5, SCREEN_HEIGHT / 15, route4.c_str(), false);

	std::string route6 = "../assets/win.png";
	buttonWin = new Button(SCREEN_WIDTH / 6, (SCREEN_HEIGHT * 2 / 3) - 50, SCREEN_WIDTH * 3 / 5, SCREEN_HEIGHT / 15, route6.c_str(), false);

	std::string route7 = "../assets/lose.png";
	buttonLose = new Button(SCREEN_WIDTH / 6, (SCREEN_HEIGHT * 2 / 3) - 50, SCREEN_WIDTH * 3 / 5, SCREEN_HEIGHT / 15, route7.c_str(), false);

	std::string route8 = "../assets/leaveMM.png";
	buttonLMatchmaking = new Button(SCREEN_WIDTH / 3, SCREEN_HEIGHT * 2 / 3 + SCREEN_HEIGHT / 8 + 10, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 8, route8.c_str(), true);

	std::string route9 = "../assets/country.png";
	buttonCountry = new Button(SCREEN_WIDTH / 6, (SCREEN_HEIGHT * 2 / 3) - 50, SCREEN_WIDTH * 3 / 5, SCREEN_HEIGHT / 15, route9.c_str(), false);
}

//----------------MENU SCENE--------------------------
void Game::updateMenu()
{
	std::queue<std::string> messages = NetworkManager::Instance()->getMessages(MenuL);

	while (messages.size() != 0)
	{
		std::string type = NetworkManager::Instance()->findSubstr(messages.front(), 3);

		if (type == "del") //Room full
		{
			buttonPlay->setPressed(false);
			buttonPlay->setState(outside);
			fullMessage = true;
			exitMessage = false;
			win = false;
			lose = false;
			country = false;
			timer = 0;
		}
		if (type == "nco") //Room full
		{
			buttonPlay->setPressed(false);
			buttonPlay->setState(outside);
			fullMessage = false;
			exitMessage = false;
			win = false;
			country = true;
			lose = false;
			timer = 0;
		}
		else if (type == "goG")
		{
			buttonPlay->setPressed(false);
			buttonPlay->setState(outside);
			fullMessage = false;
			exitMessage = false;
			playPressed = false;
			win = false;
			lose = false;
			matchmakingB = true;
			buttonLMatchmaking->setPressed(false);
			buttonLMatchmaking->setState(outside);
			timer = 0;

			p1->Reset();
			p2->Reset();
			b->Reset();
			

			actualScene = game;

			printf("Waiting for play\n");
		}
		messages.pop();
	}
	if (actualScene == game) return;

	SDL_Event events;
	while (SDL_PollEvent(&events))
	{
		buttonPlay->update(deltaTime, events);
		buttonExit->update(deltaTime, events);
		c->handleEvent(events);
	}
	c->update(deltaTime);

	if (buttonPlay->getState() == pressed && !buttonPlay->getPressed())
	{
		fullMessage = false;
		exitMessage = false;
		buttonPlay->setPressed(true);
		p1->Reset();
		p2->Reset();
		b->Reset();
		NetworkManager::Instance()->SendMessageTCP("siz19");
		NetworkManager::Instance()->SendMessageTCP("new" + (std::string)SERVERIP + NetworkManager::Instance()->getUDPPort() + NetworkManager::Instance()->getCountry());
		buttonPlay->setState(outside);
		playPressed = true;
		timer = 0;
	}


	if (buttonExit->getState() == pressed)
	{
		running = false;
		NetworkManager::Instance()->SendMessageTCP("siz03");
		NetworkManager::Instance()->SendMessageTCP("dis");
	}

	if (fullMessage || exitMessage || playPressed || win || lose || country)
	{
		timer += deltaTime;

		if (timer > TEXT_TIME)
		{
			timer = 0;
			exitMessage = false;
			fullMessage = false;
			win = false;
			lose = false;
			country = false;
		}
	}
}

void Game::drawMenu()
{
	buttonPlay->render();
	buttonExit->render();
	c->render();

	if (fullMessage)
	{
		buttonRFull->render();
	}
	else if (exitMessage)
	{
		buttonOLeft->render();
	}
	else if (win)
	{
		buttonWin->render();
	}
	else if (lose)
	{
		buttonLose->render();
	}
	else if (country)
	{
		buttonCountry->render();
	}
}

//---------------GAME SCENE--------------------------
void Game::updateGame()
{
	std::queue<std::string> messages = NetworkManager::Instance()->getMessages(GameL);
	while (messages.size() != 0)
	{
		std::string type = NetworkManager::Instance()->findSubstr(messages.front(), 3);
		if (type == "out")
		{
			actualScene = menu;
			exitMessage = true;
			timer = 0;
			NetworkManager::Instance()->CleanGameMessages();
		}
		else if (type == "win")
		{
			actualScene = menu;
			win = true;
			timer = 0;
			NetworkManager::Instance()->CleanGameMessages();
		}
		else if (type == "sta")
		{
			matchmakingB = false;
		}
		messages.pop();
	}
	if (actualScene == menu) return;

	if (matchmakingB)
	{
		SDL_Event events;
		while (SDL_PollEvent(&events))
		{
			buttonLMatchmaking->update(deltaTime, events);
		}
		if (buttonLMatchmaking->getState() == pressed && !buttonLMatchmaking->getPressed())
		{
			actualScene = menu;
			matchmakingB = false;
			buttonLMatchmaking->setPressed(true);
			NetworkManager::Instance()->SendMessageTCP("siz03");
			NetworkManager::Instance()->SendMessageTCP("lmm");
			buttonLMatchmaking->setState(outside);
		}
	}
	else
	{
		if (!p1->update(deltaTime))
		{
			actualScene = menu;
			NetworkManager::Instance()->SendMessageTCP("siz03");
			NetworkManager::Instance()->SendMessageTCP("out");
			if (p1->endGame())
			{
				running = false;
			}
			return;
		}
		if (p1->getConfirmedLose())
		{
			actualScene = menu;
			lose = true;
			timer = 0;
			NetworkManager::Instance()->CleanGameMessages();
		}

		p2->update(deltaTime);

		b->update(deltaTime);
	}
}

void Game::drawGame()
{
	if (matchmakingB)
	{
		buttonLMatchmaking->render();
	}
	p1->render();
	p2->render();
	b->render();
}

//------------MAIN GAME-------------------------
void Game::runNetwork()
{
	while (running)
	{
		NetworkManager::Instance()->CheckMessages();
	}
}


void Game::run()
{
	/* Main loop */
	while (running) {

		frame_start = Renderer::Instance()->getTicks();

		//------CLEAR-----------		
		Renderer::Instance()->clear(0x00000000);
		//----------------------

		if (actualScene == game)
		{
			updateGame();
			drawGame();
		}
		else
		{
			updateMenu();
			drawMenu();
		}

		//------PRESENT---------
		Renderer::Instance()->present();

		//------DeltaTime and FPS
		if (frameDelay > Renderer::Instance()->getTicks() - frame_start)
		{
			SDL_Delay(Renderer::Instance()->getTicks() - frame_start);
		}
		deltaTime = ((float)((Renderer::Instance()->getTicks() - frame_start)) / 1000.f);
		frametime += deltaTime;
	}

	netThread.join(); //Wait thread to finish
}

Game::~Game()
{
	Renderer::Instance()->Release();
	NetworkManager::Instance()->Release();

	delete buttonLMatchmaking;
	delete buttonCountry;
	delete buttonOLeft;
	delete buttonRFull;
	delete buttonExit;
	delete buttonPlay;
	delete buttonWin;
	delete buttonLose;

	delete c;
	delete b;
	delete p2;
	delete p1;
}