#pragma once
#include "Entity.h"
#include "checkML.h"

#define PREDICTION_TIME 0.1f
#define PACKAGES_LOST_TIME 2.f


class Player: public Entity
{
private:
	void manageMessages();
	
	float lastMovX = 0;
	float lastMovY = 0;

	float startingPositionY = 0;

	bool closeGame = false;

	float timeSinceStart = 0;
	float sendPos = 0;

	float fixSendRate = 0;

	float confirmLose = false;
public:
	Player(int positionX, int positionY, float _speed, int _life);

	bool update(float deltaTime);

	void Reset();

	void render();
	
	int getLife();

	bool endGame();

	bool getConfirmedLose();

	~Player();

};

