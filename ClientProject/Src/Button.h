#pragma once
#include <string>
#include "checkML.h"

struct SDL_Texture;
union SDL_Event;

enum buttonState{outside, inside, pressed};

class Button
{
private:
	void checkCollision(int mouseX, int mouseY);

	float posX = 50;
	float posY = 50;

	int width = 50;
	int height = 50;

	int margin = 3;

	buttonState state = outside;

	SDL_Texture* message = nullptr;
	
	bool background = true;

	bool alreadyPressed = false;
public:
	Button(int positionX, int positionY, float w, int h,const  char* textRoute, bool _background);

	void update(float deltaTime, SDL_Event event);

	void render();

	bool getPressed();
	void setPressed(bool set);

	buttonState getState();
	void setState(buttonState set);

	~Button();

};

