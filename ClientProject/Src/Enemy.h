#pragma once
#include "Entity.h"
#include "checkML.h"
class Player;
class Boss;

class Enemy: public Entity
{
private:
	void manageMessages();
	void linearPrediction();
protected:
	Player* player;
	Boss* boss;

	float timeSinceStart = 0;

	int lastPosXMessage = 0;
	int lastPosYMessage = 0;
	int newPosXMessage = 0;
	int newPosYMessage = 0;

	float timeActualMessage = 0;
	float timeLastMessage = 0;

	float displacementX = 0;
	float displacementY = 0;
	float timeLastPosition = 0;

	//Predict conexion
	float sendLostPackages = 0;
	int numpackages = 0;
	float fixSendRate = 0;

	bool firstMessage = true;
public:
	Enemy(int positionX, int positionY, float _speed, int _life, Player* p, Boss* b );

	void Reset();
	
	bool update(float deltaTime);

	void render();

	~Enemy();
};

