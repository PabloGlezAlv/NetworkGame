#pragma once
class Entity
{
protected:
	float speed = 1;

	float posX = 50;
	float posY = 50;

	float movX = 0;
	float movY = 0;

	int width = 50;
	int height = 50;

	int wWidth;
	int wHeight;
public:
	Entity(int positionX, int positionY, float _speed, int screenWidth);

	virtual bool update(float deltaTime) = 0;

	~Entity();

	void checkCollision();

};

