#include "Button.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <SDL_events.h>
#include <SDL_image.h>

#include "Renderer.h"
#include "NetworkManager.h"

Button::Button(int positionX, int positionY, float w, int h, const char* textRoute, bool _background)
{
	posX = positionX;
	posY = positionY;

	width = w;
	height = h;

	background = _background;

	message = IMG_LoadTexture(Renderer::Instance()->getRenderer(), textRoute);
	if (message == NULL)
		printf("Error creating button with route %s\n", textRoute);
}

void Button::checkCollision(int mouseX, int mouseY)
{
	
	if (mouseX > posX && mouseX < posX + width &&
		mouseY> posY && mouseY < posY + height)
	{
		state = inside;
	}
	else
	{
		state = outside;
	}
}

bool Button::getPressed()
{
	return alreadyPressed;
}
void Button::setPressed(bool set)
{
	alreadyPressed = set;
}

buttonState Button::getState()
{
	return state;
}

void Button::setState(buttonState set)
{
	state = set;
}

void Button::update(float deltaTime, SDL_Event event)
{
	int x, y;

	if (event.type == SDL_MOUSEMOTION)
	{
		SDL_GetMouseState(&x, &y);
		checkCollision(x, y);
	}
	if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		if (state == inside)
		{
			state = pressed;
		}
	}
}

void Button::render()
{
	if (background)
	{
		Renderer::Instance()->drawRect(posX, posY, width, height, 0xFFFFFFFF);

		if (state == inside)
			Renderer::Instance()->drawRect(posX + margin, posY + margin, width - (margin * 2), height - (margin * 2), 0xFF4444FF);
		else
			Renderer::Instance()->drawRect(posX + margin, posY + margin, width - (margin * 2), height - (margin * 2), 0xFF0000FF);
	}

	Renderer::Instance()->drawImage(message, 0, 0, 0, 0, posX, posY, posX + width, posY + height);
}


Button::~Button()
{

}