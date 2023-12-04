#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <ctime> 

#include "Game.h"
#include "checkML.h"

int main(int argc, char* args[]) {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	std::srand(std::time(0));

	Game g;

	if (!g.init())
	{
		printf("Error initialaizing everything\n");
		return -1;
	}

	g.run();

	return 0;
}