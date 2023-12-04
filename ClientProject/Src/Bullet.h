#pragma once
#include "checkML.h"
class Player;

#define BULLET_SIZE 5

class Bullet
{
protected:
	Player* player;

	int posX;
	int posY;

	int movX;
	int movY;

	float speed;

	float lifeTime;
	float timer;

	int bulletID;
public:
	Bullet(int positionX, int positionY, float _speed, float dirX, float dirY, int id, Player* p);

	bool update(float deltaTime);

	void render();

	bool playerCollision();

	int getPosX();
	int getPosY();
	int getBulletID();

	~Bullet();
};

