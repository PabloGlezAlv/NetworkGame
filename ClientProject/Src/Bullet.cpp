#include "Bullet.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#include "Renderer.h"
#include "Player.h"

Bullet::Bullet(int positionX, int positionY, float _speed, float dirX, float dirY, int id, Player* p)
{
	speed = _speed;
	posX = positionX;
	posY = positionY;

	movX = dirX;
	movY = dirY;

	lifeTime = 5.f;
	timer = 0;

	player = p;

	bulletID = id;
}

int Bullet::getPosX()
{
	return  posX;
}
int Bullet::getPosY()
{
	return posY;
}

int Bullet::getBulletID()
{
	return bulletID;
}

bool  Bullet::playerCollision()
{
	if (posX + BULLET_SIZE < player->getPosX() ||
		posX > player->getPosX() + player->getWidth() ||
		posY + BULLET_SIZE < player->getPosY() ||
		posY > player->getPosY() + player->getHeight()) {

		return false;
	}
	// Collision

	player->receiveDamage(bulletID);
	return true;
}

bool Bullet::update(float deltaTime)
{
	if (movX != 0 && movY != 0)
	{
		//0.707 Solution of normalized vector
		posX += speed * deltaTime * 0.707 * movX;
		posY += speed * deltaTime * 0.707 * movY;
	}
	else
	{
		posX += speed * deltaTime * movX;
		posY += speed * deltaTime * movY;
	}

	timer += deltaTime;
	if (timer >= lifeTime)
	{
		return false;
	}

	return !playerCollision();
}

void Bullet::render()
{
	Renderer::Instance()->drawRect(posX, posY, BULLET_SIZE, BULLET_SIZE, 255, 255, 255, 255);
}



Bullet::~Bullet()
{

}