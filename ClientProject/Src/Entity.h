#pragma once
#include "checkML.h"

#define MAX_LIFE 100.0f

#define DAMAGE 10.f



class Entity
{
protected:
	enum color { none, red, blue };

	void checkCollision();
	virtual void manageMessages() = 0;

	float speed = 1;
	int life = 100;

	float posX = 50;
	float posY = 50;

	float movX = 0;
	float movY = 0;

	int width = 50;
	int height = 50;

	int wWidth = 0;
	int wHeight = 0;

	color entityColor = none;

	bool start = false;
public:
	Entity(int positionX, int positionY, float _speed, int _life);

	virtual bool update(float deltaTime) = 0;

	bool receiveDamage( int bulletID);

	int getWidth();
	int getHeight();
	int getPosX();
	int getPosY();

	virtual void render() = 0;

	~Entity();

};

