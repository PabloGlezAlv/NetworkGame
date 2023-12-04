#pragma once
#include <string>

#define IDENTIFIER_SIZE 3
#define MESSAGE_SIZE 50
struct Message
{
	char* mID[IDENTIFIER_SIZE];
	uint16_t entityID;
};

struct MessageChat : public Message
{
	char* message[MESSAGE_SIZE];
};

struct MessagePos: public Message
{
	int dirX;
	int dirY;
};