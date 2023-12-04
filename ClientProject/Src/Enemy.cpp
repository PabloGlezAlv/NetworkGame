#include "Enemy.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "Player.h"
#include "Boss.h"
#include "Renderer.h"
#include "NetworkManager.h"
#include <iostream>
#include <chrono>

Enemy::Enemy(int positionX, int positionY, float _speed, int _life, Player* p, Boss* b) : Entity(positionX, positionY, _speed, _life)
{
	player = p;
	boss = b;
}

//Set default values when restart the game
void Enemy::Reset()
{
	start = false;
	life = 100;

	movX = 0;
	movY = 0;

	posY = Renderer::Instance()->getHeight() * 2 / 3;
	lastPosYMessage = posY;
	newPosYMessage = posY;
	timeSinceStart = 0;

	firstMessage = true;
}

void Enemy::manageMessages()
{
	std::queue<std::string> messages = NetworkManager::Instance()->getMessages(EnemyL);

	while (messages.size() != 0)
	{
		std::string type = NetworkManager::Instance()->findSubstr(messages.front(), 3);

		if (type == "sta")
		{
			start = true;
			if (messages.front()[3] == '1')
			{
				posX = wWidth * 3 / 4;
				entityColor = red;
			}
			else
			{
				posX = wWidth / 4;
				entityColor = blue;
			}
			lastPosXMessage = posX;
			newPosXMessage = posX;
			life = 100;

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

			//std::cout << "Server time " << actualSecs << std::endl;
			//std::cout << "Client time " << lastData << std::endl;
			//std::cout << "Start point " << timeSinceStart << std::endl;
		}
		else if (type == "pos") //Position message
		{
			numpackages++;
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
		else if (type == "dam")
		{
			int damage = (messages.front()[3] - '0') * 10 + (messages.front()[4] - '0');

			life -= damage;

			int id = (messages.front()[5] - '0') * 100 + (messages.front()[6] - '0') * 10 + (messages.front()[7] - '0');

			boss->destroyBullet(id);
		}
		messages.pop();
	}
}

void Enemy::linearPrediction()
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

bool Enemy::update(float deltaTime)
{
	timeSinceStart += deltaTime; 

	manageMessages();

	if (!start) return true;
	sendLostPackages += deltaTime;

	linearPrediction();

	posX = lastPosXMessage + displacementX;
	posY = lastPosYMessage + displacementY;

	checkCollision();

	//Predict lostpackages
	if (PACKAGES_LOST_TIME < sendLostPackages)
	{
		sendLostPackages = 0;

		float expectedMessages = PACKAGES_LOST_TIME / (PREDICTION_TIME - fixSendRate); //Get number of packages I shoud have received

		float rate = numpackages / expectedMessages;

		float losep = 1.f - rate;

		float lose = losep * 100;

		if (lose > 7.5f && lose < 50.f) //7.5% is just 1.5 packages, not overload of messages
		{
			std::stringstream ss1;
			ss1 << std::setw(2) << std::setfill('0') << (int)lose;
			std::string percentage = ss1.str();

			NetworkManager::Instance()->SendMessageTCP("siz05");
			NetworkManager::Instance()->SendMessageTCP("pck" + percentage);

			float newPackagesSec = (1 / PREDICTION_TIME) + ((lose / 2) / 100) * (1 / PREDICTION_TIME);// - /2 lost is in 2 seconds

			float newTime = (1 / newPackagesSec);

			fixSendRate = PREDICTION_TIME - newTime;
		}

		numpackages = 0;
	}

	return true;
}

void Enemy::render()
{
	if (!start) return;

	if (entityColor == blue)
		Renderer::Instance()->drawRect(posX, posY, width, height, 0, 0, life / MAX_LIFE * 255, 255);
	else if(entityColor == red)
		Renderer::Instance()->drawRect(posX, posY, width, height, life / MAX_LIFE * 255, 0, 0, 255);
}

Enemy::~Enemy()
{

}