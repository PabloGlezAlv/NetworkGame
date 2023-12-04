#include "Entity.h"
#include <sstream>
#include <iomanip>
#include <iostream>

Entity::Entity(int positionX, int positionY, float _speed, int screenWidth)
{
	speed = _speed;
	posX = positionX;
	posY = positionY;

	wWidth = screenWidth;
	wHeight = screenWidth * 9 / 16;
}


void Entity::checkCollision()
{
	// right boundary
	if (posX + width > wWidth)
	{
		posX = wWidth - width;
	}
	// left boundary
	else if (posX < 0)
	{
		posX = 0;
	}
	// bottom boundary
	if (posY + height > wHeight)
	{
		posY = wHeight - height;
	}
	// upper boundary
	else if (posY < 0)
	{
		posY = 0;
	}
}

Entity::~Entity()
{

}