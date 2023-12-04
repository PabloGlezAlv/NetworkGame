#include "Boss.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#include "NetworkManager.h"

BossBehaviour::BossBehaviour(int positionX, int positionY, float _speed, float wWidth) : Entity(positionX, positionY, _speed, wWidth)
{
	posX = wWidth / 3.5f;
	posY = wWidth / 6;

	initPos.first = posX;
	initPos.second = posY;
	

	movX = directions[start].first;
	movY = directions[start].second;

	playerLife1 = 100;
	playerLife2 = 100;
	//std::cout << "New game" << std::endl;
}

bool BossBehaviour::update(float deltaTime)
{
	std::queue<std::string> messages = NetworkManager::Instance()->getBossMessages();
	std::queue<int> senderMessages = NetworkManager::Instance()->getBossSenderMessages();

	while (messages.size() != 0)
	{
		std::string type = NetworkManager::Instance()->findSubstr(messages.front(), 3);

		if (type == "out")
		{
			posX = initPos.first;
			posY = initPos.second;

			start = 0;

			timer = 0;

			bulletTimer = 0;

			movX = directions[start].first;
			movY = directions[start].second;

			playerLife1 = 100;
			playerLife2 = 100;
		}
		else if (type == "dam")
		{
			int damage = (messages.front()[3] - '0') * 10 + (messages.front()[4] - '0');
			int bulletID = (messages.front()[5] - '0') * 100 + (messages.front()[6] - '0') * 10 + (messages.front()[7] - '0');

			//std::cout << "Message: " << messages.front() << std::endl;
			//std::cout << "Damage received from bullet: " << bulletID << std::endl;


			bool bulletFound = false;
			auto it = bulletsAlive.begin();
			while (it != bulletsAlive.end())
			{
				if (bulletID == (*it).id)
				{
					bulletsAlive.erase(it);
					bulletFound = true;
					break;
				}

				if (it != bulletsAlive.end())
					it++;
			}

			std::list<int> players = NetworkManager::Instance()->getPlayersTCPID();
			if (bulletFound) //Send the message to the other player
			{

				//std::cout << "Boss::Damage received from player with ID: " << senderMessages.front() << std::endl;
				//std::cout << "Boss::Player 1: " << players.front() << std::endl;
				//std::cout << "Boss::Player 2: " << players.back() << std::endl;

				if (senderMessages.front() == players.front())
				{
					playerLife1 -= damage;
					//std::cout << "Life player 1 " << playerLife1 << std::endl;
					if (playerLife1 <= 0)
					{
						NetworkManager::Instance()->SendMessagesTCPInGame(1, "siz03");
						NetworkManager::Instance()->SendMessagesTCPInGame(1, "win");
						NetworkManager::Instance()->SendMessagesTCPInGame(0, "siz03");
						NetworkManager::Instance()->SendMessagesTCPInGame(0, "los");
						NetworkManager::Instance()->ClearGame();
						Reset();
					}
					else
					{
						NetworkManager::Instance()->SendMessagesTCPInGame(1, "siz08");
						NetworkManager::Instance()->SendMessagesTCPInGame(1, messages.front());
					}
				}
				else //Player 2
				{
					playerLife2 -= damage;
					//std::cout << "Life player 2 " << playerLife2 << std::endl;
					if (playerLife2 <= 0)
					{
						NetworkManager::Instance()->SendMessagesTCPInGame(0, "siz03");
						NetworkManager::Instance()->SendMessagesTCPInGame(0, "win");
						NetworkManager::Instance()->SendMessagesTCPInGame(1, "siz03");
						NetworkManager::Instance()->SendMessagesTCPInGame(1, "los");
						NetworkManager::Instance()->ClearGame();
						Reset();
					}
					else
					{
						NetworkManager::Instance()->SendMessagesTCPInGame(0, "siz08");
						NetworkManager::Instance()->SendMessagesTCPInGame(0, messages.front());
					}
				}
			}
			else //Return the life to the player
			{
				if (senderMessages.front() == players.front())
				{
					NetworkManager::Instance()->SendMessagesTCPInGame(0, "siz03");
					NetworkManager::Instance()->SendMessagesTCPInGame(0, "lgb");
				}
				else
				{
					NetworkManager::Instance()->SendMessagesTCPInGame(1, "siz03");
					NetworkManager::Instance()->SendMessagesTCPInGame(1, "lgb");
				}
			}
		}
		messages.pop();
		senderMessages.pop();
	}
	//Do the movement

	posX += speed * deltaTime * 0.707 * movX;
	posY += speed * deltaTime * 0.707 * movY;
	
	checkCollision();

	timer += deltaTime;
	bulletTimer += deltaTime;
	sendPos += deltaTime;
	timeSinceStart += deltaTime;
	// Move boss
	if (timer >= CHANGE_DIRECTION)
	{
		timer = 0;

		movX = directions[start].first;
		movY = directions[start].second;

		start++;

		start = start % directions.size();
	}
	//Update bullets

	bulletsAlive.remove_if([deltaTime](bullet& b) {
		b.lifeTime -= deltaTime;
		return b.lifeTime <= 0;
		});

	// Create bullets
	if (bulletTimer >= SPAWN_TIME)
	{
		bulletTimer = 0;

		int firstDir = rand() % directions.size();

		int nSpawn = 1 + rand() % (SPAWN_AMOUNT - 1);

		for (int i = 0; i < nSpawn; i++)
		{
			bullet aux;
			aux.id = idCount;
			aux.lifeTime = LIFETIME;
			bulletsAlive.push_back(aux);

			std::stringstream s1;
			s1 << std::setw(2) << std::setfill('0') << directions[firstDir].first;
			std::string sX = s1.str();

			std::stringstream s2;
			s2 << std::setw(2) << std::setfill('0') << directions[firstDir].second;
			std::string sY = s2.str();

			std::stringstream s3;
			s3 << std::setw(3) << std::setfill('0') << idCount++;
			std::string sID = s3.str();

			std::string data = "bul" + sX + sY + sID;
			if (!(data.length() < 10))
			{
				NetworkManager::Instance()->SendMessagesTCPInGameAll("siz10");
				NetworkManager::Instance()->SendMessagesTCPInGameAll(data);
			}

			firstDir++;

			firstDir = firstDir % directions.size();
			
			if (idCount == MAX_ID) idCount = 0;
		}
	}

	if (PREDICTION_TIME < sendPos)
	{
		std::stringstream ss1;
		ss1 << std::setw(4) << std::setfill('0') << (int)posX;
		std::string sX = ss1.str();
		std::stringstream ss2;
		ss2 << std::setw(4) << std::setfill('0') << (int)posY;
		std::string sY = ss2.str();

		//std::cout << "Position: " << sX << "-" << sY << std::endl;
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

		std::string data = "bpo" + sX + sY + sTs + sTM;
		NetworkManager::Instance()->SendMessagesUPDInGame(data);

		sendPos = 0;
	}

	return true;
}

void BossBehaviour::Reset()
{
	posX = initPos.first;
	posY = initPos.second;

	start = 0;

	timer = 0;

	bulletTimer = 0;


	movX = directions[start].first;
	movY = directions[start].second;

	playerLife1 = 100;
	playerLife2 = 100;

	//std::cout << "Reset" << "\n";
}

BossBehaviour::~BossBehaviour()
{

}