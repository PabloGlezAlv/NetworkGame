#include "Boss.h"

#include <chrono>
#include <iostream>

#include "Player.h"
#include "Renderer.h"
#include "NetworkManager.h"
#include "Bullet.h"

Boss::Boss(int positionX, int positionY, float _speed, int _life, Player* p) : Entity(positionX, positionY, _speed, _life)
{
	player = p;

	movX = 0;
	movY = 1;

	posX = wWidth / 3.5f;
	posY = wWidth / 6;

	lastPosXMessage = posX;
	lastPosYMessage = posY;
}

//Set default values when restart the game
void Boss::Reset()
{
	start = false;

	movX = 0;
	movY = 1;
	timeSinceStart = 0;

	posX = wWidth / 3.5f;
	posY = wWidth / 6;

	lastPosXMessage = posX;
	lastPosYMessage = posY;
	firstMessage = true;

	bullets.clear();
}


void Boss::linearPrediction()
{
	float distanceBetween = sqrt(pow(newPosXMessage - lastPosXMessage, 2) + pow(newPosYMessage - lastPosYMessage, 2));
	float timeBetween = timeActualMessage - timeLastMessage;
	speed = distanceBetween / timeBetween;
	if (std::isnan(speed))
	{
		speed = 0;
	}

	if (distanceBetween != 0.0f)
	{
		movX = (newPosXMessage - lastPosXMessage) / distanceBetween;
		movY = (newPosYMessage - lastPosYMessage) / distanceBetween;
	}
	else
	{
		movX = 0;
		movY = 0;
	}

	displacementX = speed * (timeSinceStart - timeActualMessage) * movX;
	displacementY = speed * (timeSinceStart - timeActualMessage) * movY;
}


//Destroy de closest bullet to the position sent
void Boss::destroyBullet(int id)
{
	auto it = bullets.begin();
	while (it != bullets.end())
	{
		if (id == it->getBulletID())
		{
			bullets.erase(it);
			break;
		}

		if (it != bullets.end())
			it++;
	}

}

void Boss::manageMessages()
{
	std::queue<std::string> messages = NetworkManager::Instance()->getMessages(BossL);

	while (messages.size() != 0)
	{
		std::string type = NetworkManager::Instance()->findSubstr(messages.front(), 3);

		if (type == "sta")
		{
			start = true;

			posX = wWidth / 3.5f;
			posY = wWidth / 6;

			int actualSecs = 0;
			for (int i = 4; i < 10; i++)
			{
				actualSecs += (messages.front()[i] - '0') * pow(10, 9 - i);
			}

			auto currentTime = std::chrono::system_clock::now();

			auto durationInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch());
			long long milliseconds = durationInMilliseconds.count();

			int lastData = milliseconds % 1000000;

			float milisecsPassed = lastData - actualSecs;
			timeSinceStart = milisecsPassed / 1000.f;

		}
		else if (type == "bpo") //Position message
		{
			lastPosXMessage = posX;
			lastPosYMessage = posY;
			timeLastMessage = timeActualMessage;

			//Check posX of other player
			newPosXMessage = 0;
			for (int i = 3; i < 7; i++)
			{
				newPosXMessage += (messages.front()[i] - '0') * pow(10, 6 - i);
			}
			//Check posY of other player
			newPosYMessage = 0;
			for (int i = 7; i < 11; i++)
			{
				newPosYMessage += (messages.front()[i] - '0') * pow(10, 10 - i);
			}

			//Seconds
			float secs = 0;
			for (int i = 11; i < 14; i++)
			{
				secs += (messages.front()[i] - '0') * pow(10, 13 - i);
			}
			//Decimal
			float decimal = 0;
			for (int i = 14; i < 22; i++)
			{
				decimal += (messages.front()[i] - '0') * pow(10, 21 - i);
			}

			timeActualMessage = secs + (decimal / pow(10, 8));

			if (firstMessage)
			{
				firstMessage = false;
				lastPosXMessage = newPosXMessage;
				lastPosYMessage = newPosYMessage;
			}
		}
		else if (type == "bul" && messages.front().size() == 10)
		{
			float dirX, dirY;
			//Check X direction of bullet
			if (messages.front()[3] - '0' == 0)
			{
				if (messages.front()[4] - '0' == 1)
					dirX = 1;
				else
					dirX = 0;
			}
			else
				dirX = -1;
			//Check Y direction of bullet
			if (messages.front()[5] - '0' == 0)
			{
				if (messages.front()[6] - '0' == 1)
					dirY = 1;
				else
					dirY = 0;
			}
			else
				dirY = -1;

			int id = (messages.front()[7] - '0') * 100 + (messages.front()[8] - '0') * 10 + (messages.front()[9] - '0');

			bullets.push_back(Bullet(posX + width / 2, posY + height / 2, 400, dirX, dirY, id, player));
		}

		messages.pop();
	}
}

bool Boss::update(float deltaTime)
{
	manageMessages();

	if (!start) return true;

	timeSinceStart += deltaTime;

	linearPrediction();

	posX = lastPosXMessage + displacementX;
	posY = lastPosYMessage + displacementY;

	//std::cout << "Position MESSAGE: " << lastPosXMessage << " - " << lastPosYMessage << std::endl;

	checkCollision();

	auto it = bullets.begin();
	while (it != bullets.end())
	{
		if (!(it)->update(deltaTime))
		{
			it = bullets.erase(it);
		}
		if (it != bullets.end()) it++;
	}
}

void Boss::render()
{
	if (!start) return;

	for (auto it = bullets.begin(); it != bullets.end(); ++it)
	{
		(it)->render();
	}

	Renderer::Instance()->drawRect(posX, posY, width, height, 0xDDDDDDFF);
}

Boss::~Boss()
{

}