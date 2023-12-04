#pragma once
#include <string>
#include <queue>
#include <SDL_ttf.h>
#include "checkML.h"

#define NUM_MESSAGES 20
#define MAXSIZE 50
#define LINETIME 0.5f

union SDL_Event;

struct MessageInfo
{
	std::string name;
	std::string text;
	SDL_Color color;
	MessageInfo( std::string t, SDL_Color c)
	{
		text = t;
		color = c;
	}
};

class Chat
{
private:
	std::string createString(char character, int n); 
	std::string copyCharacters(std::string source, std::string dest, int n);

	std::queue<MessageInfo> messagesQueue;

	std::string myText = "";

	SDL_Color myColor;

	//----Write| end of text----------
	float timer = 0;
	bool line = false;
public:
	Chat();

	void handleEvent(SDL_Event event);

	void update(float deltaTime);

	void render();

	~Chat();

};

