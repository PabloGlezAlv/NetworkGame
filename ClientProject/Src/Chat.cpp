#include "Chat.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>

#include <SDL_ttf.h>
#include <SDL.h>

#include "Renderer.h"
#include "NetworkManager.h"

Chat::Chat()
{
	// Just 200 to not be  dark like the background
	Uint8 r = (std::rand() % 200) + 56;
	Uint8 g = (std::rand() % 200) + 56;
	Uint8 b = (std::rand() % 200) + 56;
	myColor = SDL_Color{ r , g , b };

	std::string country = "Welcome to the game player from " + NetworkManager::Instance()->getCountry();
	country.pop_back();

	messagesQueue.push(MessageInfo(country, SDL_Color{255 , 255 , 255}));
}

void Chat::update(float deltaTime)
{
	timer += deltaTime;
	if (timer > LINETIME && myText.size() < MAXSIZE)
	{
		timer = 0;
		line = !line;
		if (line)
			myText += "|";
		else
			myText.pop_back();
	}
}

void Chat::handleEvent(SDL_Event events)
{
	std::queue<std::string> messages = NetworkManager::Instance()->getMessages(ChatL);

	while (messages.size() != 0)
	{
		std::string type = NetworkManager::Instance()->findSubstr(messages.front(), 3);

		if (type == "cht")
		{
			std::string content = messages.front();

			SDL_Color aux;
			aux.r = 0;
			aux.g = 0;
			aux.b = 0;
			for (int i = 3; i < 6; i++)
			{
				int number = (messages.front()[i] - '0');
				aux.r += number * pow(10, 5 - i);
			}
			for (int i = 6; i < 9; i++)
			{
				int number = (messages.front()[i] - '0');
				aux.g += number * pow(10, 8 - i);
			}
			for (int i = 9; i < 12; i++)
			{
				int number = (messages.front()[i] - '0');
				aux.b += number * pow(10, 11 - i);
			}

			int size = (messages.front()[12] - '0') * 10 + (messages.front()[13] - '0');

			std::string text = "";
			for (int i = 14; i < 14 + size && i < messages.front().size(); i++)
			{
				text += messages.front()[i];
			}

			messagesQueue.push(MessageInfo(text, aux));
			if (messagesQueue.size() > NUM_MESSAGES)
			{
				messagesQueue.pop();
			}
		}

		messages.pop();
	}

	switch (events.type) {
	case SDL_TEXTINPUT:
		if (myText.size() < MAXSIZE && !line)
		{
			myText += events.text.text;
			timer = 0;
			line = 0;
		}
		else if (myText.size() < MAXSIZE + 1 && line)
		{
			myText.pop_back();
			if (myText.empty()) myText = "";
			myText += events.text.text;

			timer = 0;
			line = false;

			if (myText.size() == MAXSIZE + 1)
			{
				myText.pop_back();
				line = false;
				timer = 0;
			}
		}
	case SDL_KEYDOWN:
		if (events.key.keysym.sym == SDLK_BACKSPACE)
		{
			if (myText.size() > 0 && !line)
			{
				myText.pop_back();
			}
			else if (myText.size() > 1 && line)
			{
				myText.pop_back();
				myText.pop_back();
			}
			timer = 0;
			line = false;
		}
		else if (events.key.keysym.sym == SDLK_RETURN)
		{
			if (myText.size() > 0 && !line)
			{
				messagesQueue.push(MessageInfo(myText, myColor));
				if (messagesQueue.size() > NUM_MESSAGES)
				{
					messagesQueue.pop();
				}

				//Send the message

				std::stringstream ss1;
				ss1 << std::setw(3) << std::setfill('0') << (int)myColor.r;
				std::string r = ss1.str();
				std::stringstream ss2;
				ss2 << std::setw(3) << std::setfill('0') << (int)myColor.g;
				std::string g = ss2.str();
				std::stringstream ss3;
				ss3 << std::setw(3) << std::setfill('0') << (int)myColor.b;
				std::string b = ss3.str();

				//Remove empty chracters at the end
				while (myText.length() > 0 && myText[myText.length() - 1] == ' ') {
					myText.pop_back();
				}

				std::stringstream ss4;
				ss4 << std::setw(2) << std::setfill('0') << (int)myText.size();
				std::string si = ss4.str();

				std::string message = "cht" + r + g + b + si + myText;
				std::string messageHashed = NetworkManager::Instance()->hashString(message);

				std::stringstream ss6;
				ss6 << std::setw(2) << std::setfill('0') << (int)messageHashed.size();
				std::string siH = ss6.str();


				//std::cout << "Mensaje en chat: " << message << std::endl;
				//std::cout << "Mensaje en chat hash: " << messageHashed << std::endl;
				
				

				int size = 3 + 9 + myText.size() + 2 + 2 + messageHashed.size();
				std::stringstream ss5;
				ss5 << std::setw(2) << std::setfill('0') << size;
				std::string s = ss5.str();
				std::string messageSize = "siz" + s;
				//std::cout << "Message size: " << messageSize << std::endl;
				NetworkManager::Instance()->SendMessageTCP(messageSize);

				//std::cout << "Mensaje final: " << message + siH + messageHashed << std::endl;
				NetworkManager::Instance()->SendMessageTCP(message + siH + messageHashed);

				myText = "";
				timer = 0;
				line = false;
			}
			else if (myText.size() > 1 && line)
			{
				myText.pop_back();
				if (messagesQueue.size() > NUM_MESSAGES)
				{
					messagesQueue.pop();
				}

				//Send the message

				std::stringstream ss1;
				ss1 << std::setw(3) << std::setfill('0') << (int)myColor.r;
				std::string r = ss1.str();
				std::stringstream ss2;
				ss2 << std::setw(3) << std::setfill('0') << (int)myColor.g;
				std::string g = ss2.str();
				std::stringstream ss3;
				ss3 << std::setw(3) << std::setfill('0') << (int)myColor.b;
				std::string b = ss3.str();

				//Remove empty chracters at the end
				while (myText.length() > 0 && myText[myText.length() - 1] == ' ') {
					myText.pop_back();
				}

				std::stringstream ss4;
				ss4 << std::setw(2) << std::setfill('0') << (int)myText.size();
				std::string si = ss4.str();

				std::string message = "cht" + r + g + b + si + myText;
				std::string messageHashed = NetworkManager::Instance()->hashString(message);

				std::stringstream ss6;
				ss6 << std::setw(2) << std::setfill('0') << (int)messageHashed.size();
				std::string siH = ss6.str();



				int size = 3 + 9 + myText.size() + 2 + 2 + messageHashed.size();
				std::stringstream ss5;
				ss5 << std::setw(2) << std::setfill('0') << size;
				std::string s = ss5.str();
				std::string messageSize = "siz" + s;

				//std::cout << "Message size: " << messageSize << std::endl;
				NetworkManager::Instance()->SendMessageTCP(messageSize);

				//std::cout << "Mensaje final: " << message + siH + messageHashed << std::endl;
				NetworkManager::Instance()->SendMessageTCP(message + siH + messageHashed);
				//std::cout << "------------------------ " << std::endl;
				myText = "";
				timer = 0;
				line = false;
			}
		}
		break;
	default:
		break;
	}
}

std::string Chat::createString(char character, int n) {
	std::string resultado = "";
	for (int i = 0; i < n; ++i) {
		resultado += character;
	}
	return resultado;
}

std::string Chat::copyCharacters(std::string source, std::string dest, int n) {
	for (int i = 0; i < n && i < source.size(); ++i) {
		dest[i] = source[i];
	}
	return dest;
}

void Chat::render()
{
	//---------------List of Messages-------------
	std::queue<MessageInfo> copy;
	int i = 0;
	while (!messagesQueue.empty())
	{
		Renderer::Instance()->drawText(messagesQueue.front().name + messagesQueue.front().text, messagesQueue.front().color, 10, 10 + i * 20);

		copy.push(messagesQueue.front());
		messagesQueue.pop();
		i++;
	}
	messagesQueue = copy;

	//-----------Message Im writing---------------
	Renderer::Instance()->drawText(myText, myColor, 10, Renderer::Instance()->getHeight() - 40);
}


Chat::~Chat()
{

}