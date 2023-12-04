#include "Entity.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#include "Renderer.h"
#include <NetworkManager.h>

Entity::Entity(int positionX, int positionY, float _speed, int _life)
{
	speed = _speed;
	life = _life;
	posX = positionX;
	posY = positionY;

	wWidth = Renderer::Instance()->getWidth();
	wHeight = Renderer::Instance()->getHeight();
}

bool Entity::receiveDamage(int bulletID)
{
	life -= DAMAGE;

	//std::cout << "Damage from bullet: " << bulletID << std::endl;

	std::stringstream ss1;
	ss1 << std::setw(2) << std::setfill('0') << (int)DAMAGE;
	std::string sD = ss1.str();

	std::stringstream ss2;
	ss2 << std::setw(3) << std::setfill('0') << (int)bulletID;
	std::string id = ss2.str();

	std::string data = "dam" + sD + id;
	NetworkManager::Instance()->SendMessageTCP("siz08");
	NetworkManager::Instance()->SendMessageTCP(data);

	return true;
}

int Entity::getWidth() { return width; }
int Entity::getHeight() { return height; }

int Entity::getPosX() { return posX; }
int Entity::getPosY() { return posY; }

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