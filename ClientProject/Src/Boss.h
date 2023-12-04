#pragma once
#include "Entity.h"
#include <list>
#include "checkML.h"
class Player;
class Bullet;

class Boss: public Entity
{
private:
	void manageMessages();
	void linearPrediction();

	int lastPosXMessage = 0;
	int lastPosYMessage = 0;
	int newPosXMessage = 0;
	int newPosYMessage = 0;

	float timeActualMessage = 0;
	float timeLastMessage = 0;

	float displacementX = 0;
	float displacementY = 0;
	float timeLastPosition = 0;
protected:
	Player* player;
	std::list<Bullet> bullets;
	
	float timeSinceStart = 0;


	bool firstMessage = true;
public:
	Boss(int positionX, int positionY, float _speed, int _life, Player* p);

	void Reset();

	bool update(float deltaTime);

	void render();

	void destroyBullet(int id);

	~Boss();
};

