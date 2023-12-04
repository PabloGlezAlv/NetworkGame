#include "Player.h"
#include <SDL_events.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <math.h>

#include "Renderer.h"
#include "NetworkManager.h"

Player::Player(int positionX, int positionY, float _speed, int _life) : Entity(positionX, positionY, _speed, _life)
{
	startingPositionY = positionY;
}

int Player::getLife()
{
	return life;
}

//Set default values when restart the game
void Player::Reset()
{
	start = false;
	lastMovX = 0;
	lastMovY = 0;
	closeGame = false;
	life = 100;
	timeSinceStart = 0;
	movX = 0;
	movY = 0;
	sendPos = 0;
	posY = startingPositionY;

	confirmLose = false;
}

void Player::manageMessages()
{
	std::queue<std::string> messages = NetworkManager::Instance()->getMessages(PlayerL);

	while (messages.size() != 0)
	{
		std::string type = NetworkManager::Instance()->findSubstr(messages.front(), 3);

		if (type == "sta")
		{
			start = true;
			if (messages.front()[3] == '1')
			{
				posX = wWidth / 4;
				entityColor = blue;
				printf("Blue player\n");
			}
			else
			{
				posX = wWidth * 3 / 4;
				entityColor = red;
				printf("Red player\n");
			}

			life = 100;
		}
		else if (type == "los")
		{
			confirmLose = true;
		}
		else if (type == "lgb")
		{
			life += DAMAGE;
		}
		else if (type == "pkg")
		{
			int percentagelose = (messages.front()[3] - '0') * 10 + (messages.front()[4] - '0');

			int expectedMessages = PACKAGES_LOST_TIME / PREDICTION_TIME;

			int lost = expectedMessages - (expectedMessages * (percentagelose / 100.f));

			//std::cout << "Packages lost: " << lost << std::endl;

			if (lost > 1) // One package can be lost 
			{
				float newPackagesSec = (1 / PREDICTION_TIME) + ((percentagelose / 2) / 100) * (1 / PREDICTION_TIME);

				float newTime = (1 / newPackagesSec);
				
				fixSendRate = PREDICTION_TIME - newTime;

				//std::cout << "Send rate: " << PREDICTION_TIME - fixSendRate << std::endl;
			}
		}
		messages.pop();
	}
}

bool Player::update(float deltaTime)
{
	manageMessages();

	if (!start) return true;

	timeSinceStart += deltaTime;
	sendPos += deltaTime;
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		/* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */

		switch (event.type) {
		case SDL_QUIT:
			closeGame = true;
			return false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LCTRL)
			{
				start = false;
				return false;
			}
			if (event.key.keysym.sym == SDLK_RIGHT)
			{
				movX = 1;
			}
			if (event.key.keysym.sym == SDLK_LEFT)
			{
				movX = -1;
			}
			if (event.key.keysym.sym == SDLK_UP)
			{
				movY = -1;
			}
			if (event.key.keysym.sym == SDLK_DOWN)
			{
				movY = 1;
			}
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_RIGHT)
			{
				movX = 0;
			}
			if (event.key.keysym.sym == SDLK_LEFT)
			{
				movX = 0;
			}
			if (event.key.keysym.sym == SDLK_UP)
			{
				movY = 0;
			}
			if (event.key.keysym.sym == SDLK_DOWN)
			{
				movY = 0;
			}
			break;
		default:
			break;
		}
	}
	//Do the movement
	if (movX != 0 && movY != 0)
	{
		//0.707 Solution of normalized vector
		posX += speed * deltaTime * 0.707 * movX;
		posY += speed * deltaTime * 0.707 * movY;
	}
	else
	{
		posX += speed * deltaTime * movX;
		posY += speed * deltaTime * movY;
	}

	checkCollision();
	//Send data to the server
	if (PREDICTION_TIME - fixSendRate < sendPos && entityColor != none)
	{
		std::stringstream ss1;
		ss1 << std::setw(4) << std::setfill('0') << (int)posX;
		std::string sX = ss1.str();
		std::stringstream ss2;
		ss2 << std::setw(4) << std::setfill('0') << (int)posY;
		std::string sY = ss2.str();
		//Send the time
		//Seconds
		float secs;
		float decimal = modf(timeSinceStart, &secs) * pow(10, 8);
		std::stringstream ss3;
		ss3 << std::setw(3) << std::setfill('0') << (int)secs;
		std::string sTs = ss3.str();
		//Milisecs...
		std::stringstream ss4;
		ss4 << std::setw(8) << std::setfill('0') << (int)(decimal);
		std::string sTM = ss4.str();

		std::string data = "pos" + sX + sY + sTs + sTM;
		NetworkManager::Instance()->SendMessageUDP(data);

		//std::cout << "Sending message of pos: "<<data<<"\n";
		sendPos = 0;
	}

	lastMovX = movX;
	lastMovY = movY;

	return true;
}

void Player::render()
{
	if (!start) return;


	if (entityColor == blue)
		Renderer::Instance()->drawRect(posX, posY, width, height, 0, 0, life / MAX_LIFE * 255, 255);
	else if(entityColor == red)
		Renderer::Instance()->drawRect(posX, posY, width, height, life / MAX_LIFE * 255, 0, 0, 255);
}

bool Player::endGame()
{
	return closeGame;
}
bool Player::getConfirmedLose()
{
	return confirmLose;
}
Player::~Player()
{

}