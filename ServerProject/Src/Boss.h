#pragma once
#include "Entity.h"
#include <vector>
#include <list>

#define LIFETIME 5.f

#define PREDICTION_TIME 0.1f

#define CHANGE_DIRECTION 1.0f //Change boss direction

#define SPAWN_TIME 0.5f //Spawn time of bullet
#define SPAWN_AMOUNT 8 //Number of bullets spawned
#define MAX_ID 999//Max number of bullet id


struct dir {
	int x = 0;
	int y = 0;
};

struct bullet{
	int id;
	float lifeTime;
};

class BossBehaviour: public Entity
{
private:

	std::vector<std::pair<float, float>> directions = { {1,0},{1, -1}, {0,-1},{-1,-1}, {-1,0},{-1,1}, {0,1},{1,1} };

	int start = 0;

	float timer = 0;

	float bulletTimer = 0;
	int idCount = 0;

	float timeSinceStart = 0;
	float sendPos = 0;

	std::pair<float, float> initPos;

	std::list<bullet> bulletsAlive;

	int playerLife1 = 100;
	int playerLife2 = 100;

public:
	BossBehaviour(int positionX, int positionY, float _speed, float wWidth);

	bool update(float deltaTime);

	void Reset();

	~BossBehaviour();

};

