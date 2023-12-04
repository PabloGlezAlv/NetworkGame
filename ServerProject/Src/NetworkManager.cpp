#include "NetworkManager.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

NetworkManager* NetworkManager::netManager = nullptr;

NetworkManager::NetworkManager()
{

}

NetworkManager* NetworkManager::Instance()
{
	return netManager;
}

bool NetworkManager::Init()
{
	netManager = new NetworkManager();

	/* Initialize SDL_net */
	if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		delete netManager;
		netManager = nullptr;
		return false;
	}

	//---------------------TCP CONEXION------------------------
	// Resolving the host using NULL make network interface to listen
	if (SDLNet_ResolveHost(&netManager->ip, NULL, SERVERPORT_TCP) < 0)
	{
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError()); exit(EXIT_FAILURE);
		delete netManager;
		netManager = nullptr;
		return false;
	}

	// Open a connection with the IP provided (listen on the host's port) 
	if (!(netManager->serverTCP = SDLNet_TCP_Open(&netManager->ip)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
		delete netManager;
		netManager = nullptr;
		return false;
	}

	//Setup the SDL version of select
	netManager->sSet = SDLNet_AllocSocketSet(NUMCLIENTS + 1);
	if (netManager->sSet == NULL) {
		fprintf(stderr, "ER: SDLNet_AllocSocketSet: %sn", SDLNet_GetError());
		SDLNet_Quit();
		delete netManager;
		netManager = nullptr;
		return false;
	}

	//Adding the server
	if (SDLNet_TCP_AddSocket(netManager->sSet, netManager->serverTCP) == -1) {
		fprintf(stderr, "ER: SDLNet_TCP_AddSocket: %sn", SDLNet_GetError());
		SDLNet_Quit();
		delete netManager;
		netManager = nullptr;
		return false;
	}

	for (int i = 0; i < NUMCLIENTS; i++)
	{
		netManager->messageSizeTCP[i] = 5;
	}

	//-------------UDP CONEXION-------------------------
	// Open a socket UDP
	if (!(netManager->sd = SDLNet_UDP_Open(SERVERPORT_UDP)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		SDLNet_Quit();
		delete netManager;
		netManager = nullptr;
		return false;
	}

	// Make space for the packet 
	if (!(netManager->p = SDLNet_AllocPacket(MESSAGESIZE)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		SDLNet_Quit();
		delete netManager;
		netManager = nullptr;
		return false;
	}

	return true;
}

bool NetworkManager::AcceptSocket(int index) {
	netManager->clientTCP[index] = SDLNet_TCP_Accept(netManager->serverTCP);
	if (netManager->clientTCP[index] == NULL) return false;

	if (SDLNet_TCP_AddSocket(sSet, netManager->clientTCP[index]) == -1) {
		fprintf(stderr, "Error: SDLNet_TCP_AddSocket: %sn", SDLNet_GetError());
		return false;
	}

	return true;
}

void NetworkManager::CheckMessagesTCP()
{
	int readySockets = SDLNet_CheckSockets(sSet, 1);

	if (readySockets <= 0) {
		// NOTE: none of the sockets are ready
	}
	else {
		// Check ready conexions
		if (SDLNet_SocketReady(serverTCP)) {
			bool socket = AcceptSocket(nClients);
			if (!socket) {
				readySockets--;
			}
			else
			{
				// Save in first free position
				int firstFree;
				for (firstFree = 0; firstFree < NUMCLIENTS; ++firstFree) {
					if (netManager->clientTCP[(nClients + firstFree) % NUMCLIENTS] == NULL) break;
				}
				if (firstFree == NUMCLIENTS)
				{
					//std::cout << "Server is full\n";
				}
				else
				{
					nClients = (nClients + firstFree) % NUMCLIENTS;
					//printf("DB: new connection, next free %s)\n", nClients);

					readySockets--;
				}
			}	
		}

		for (int i = 0; i < NUMCLIENTS && readySockets; i++)
		{
			if (netManager->clientTCP[i] == NULL) continue;
			if (!SDLNet_SocketReady(netManager->clientTCP[i])) continue;

			char* message = new char[netManager->messageSizeTCP[i]];

			if (SDLNet_TCP_Recv(netManager->clientTCP[i], message, netManager->messageSizeTCP[i]) > 0)
			{
				ManageMessageTCP(i, message);
				readySockets--;
			}
			delete[] message;
		}
		
	}
}

void NetworkManager::CheckMessagesUDP()
{
	if (SDLNet_UDP_Recv(netManager->sd, netManager->p))
	{
		//printf("UDP Packet incoming\n");
		//printf("\tChan: %d\n", netManager->p->channel);
		//printf("\tData: %s\n", (char*)netManager->p->data);
		//printf("\tLen: %d\n", netManager->p->len);
		//printf("\tMaxlen: %d\n", netManager->p->maxlen);
		//printf("\tStatus: %d\n", netManager->p->status);
		//printf("\tAddress: %x %x\n", netManager->p->address.host, netManager->p->address.port);

		ManageMessageUDP();
	}
}

std::string NetworkManager::findSubstr(std::string const& str, int n) {
	if (str.length() < n) {
		return str;
	}

	return str.substr(0, n);
}

std::string NetworkManager::hashString(std::string message)
{
	//https://www.scaler.com/topics/hashing-in-cpp-stl/

	std::hash<std::string> hasher;

	// Calcular el hash del string
	size_t hashValue = hasher(message);

	std::stringstream ss;
	ss << hashValue;
	std::string hashValueString = ss.str();

	return hashValueString;
}

void NetworkManager::Update(float deltaTime)
{
	if (startMatchmaking)
	{
		matchmakingTimer += deltaTime;

		if (matchmakingTimer > MATCHMAKING_TIME)
		{
			matchmakingTimer = 0;
			startMatchmaking = false;

			int eu = 0;
			int na = 0;
			auto it = playersInMMUDP.begin();

			// The matchmaking works mixing only player for same continent
			// The players that go to the game are one with more time in the game
			while (it != playersInMMUDP.end())
			{
				if ((*it).country == "GB" || (*it).country == "ES")
				{
					eu++;
					if (eu == 2) break;
				}
				else if ((*it).country == "US" || (*it).country == "CA")
				{
					na++;
					if (na == 2) break;
				}

				it++;
			}

			if (eu == 2)
			{
				eu = 0;

				auto it = playersInMMUDP.begin();
				auto it2 = playersInMMTCP.begin();
				while (it != playersInMMUDP.end())
				{
					if ((*it).country == "GB" || (*it).country == "ES")
					{
						playersInGameUDP.push_back((*it));
						playersInGameTCP.push_back((*it2));

						playersInMMUDP.erase(it);
						playersInMMTCP.erase(it2);

						eu++;
						if (eu == 2) break;
					}
					else
					{
						it++;
						it2++;
					}
				}
			}
			else if (na == 2)
			{
				na = 0;

				auto it = playersInMMUDP.begin();
				auto it2 = playersInMMTCP.begin();
				while (it != playersInMMUDP.end())
				{
					if ((*it).country == "GB" || (*it).country == "ES")
					{
						playersInGameUDP.push_back((*it));
						playersInGameTCP.push_back((*it2));

						playersInMMUDP.erase(it);
						playersInMMTCP.erase(it2);

						na++;
						if (na == 2) break;
					}
					else
					{
						it++;
						it2++;
					}
				}
			}

			if (eu == 2 || na == 2)
			{
				auto currentTime = std::chrono::system_clock::now();

				auto durationInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch());
				long long milliseconds = durationInMilliseconds.count();

				int lastData = milliseconds % 1000000; // Grab 2 characters of seconds and 3 of the milisecs

				//std::cout <<"Player 1 ID: " << playersInGameTCP.front() << std::endl;
				//std::cout <<"Player 2 ID: " << playersInGameTCP.back() << std::endl;

				SendMessagesTCP(playersInGameTCP.front(), "siz09");
				SendMessagesTCP(playersInGameTCP.front(), "sta1" + std::to_string(lastData));
				SendMessagesTCP(playersInGameTCP.back(), "siz09");
				SendMessagesTCP(playersInGameTCP.back(), "sta2" + std::to_string(lastData));

				match = true;
			}
		}
	}
}

void NetworkManager::SendMessagesTCP(int i, std::string message)
{
	char* bufferTCP = new char[message.size()];
	memcpy(bufferTCP, message.c_str(), message.size());

	//std::cout << "Sending chat " << message << " with size " << message.size() << "\n";
	int sent = SDLNet_TCP_Send(netManager->clientTCP[i], (void*)bufferTCP, message.size());
	if (sent > message.size())
	{
		fprintf(stderr, "SDLNet_TCP_Send Error: %s\n", SDLNet_GetError());
	}

	delete[] bufferTCP;
}

void NetworkManager::SendMessageAllTCP(std::string message, int sender)
{
	int size = message.size();
	std::stringstream ss1;
	ss1 << std::setw(2) << std::setfill('0') << size;
	std::string tam = ss1.str();

	for (int i = 0; i < NUMCLIENTS; i++)
	{
		if (i != sender && netManager->clientTCP[i] != NULL)
		{

			SendMessagesTCP(i, "siz" + tam);
			SendMessagesTCP(i, message);
			
		}
	}
}

void NetworkManager::SendMessagesTCPInGameAll(std::string message)
{
	SendMessagesTCPInGame(0, message);
	SendMessagesTCPInGame(1, message);
}

void NetworkManager::SendMessagesTCPInGame(int ind, std::string message)
{
	if(ind == 1)
		SendMessagesTCP(netManager->playersInGameTCP.back(), message);
	else
		SendMessagesTCP(netManager->playersInGameTCP.front(), message);
}

void NetworkManager::ManageMessageTCP(int ind, char* messag)
{
	std::string type = findSubstr(messag, 3);

	if (type == "new")
	{
		if (playersInGameUDP.size() < 10) // Only 2 players in the game
		{
			std::string message = findSubstr(messag, netManager->messageSizeTCP[ind]);
			//std::cout << message << std::endl;
			std::string host = "";
			for (int i = 3; i < 12; i++)
			{
				host = host + message[i];
			}
			std::string port = "";
			for (int i = 12; i < 17; i++)
			{
				port = port + message[i];
			}
			Uint32 h = swapEndian(GetIPToUint32(host));

			std::string country = message.substr(17, 19);

			//Save also connection UDP 
			ConnectionData aux;
			aux.host = h;
			aux.port = std::stoi(port);
			aux.country = country;

			//std::cout << "New Player Host: " << aux.host << "Port: " << aux.port << std::endl;

			if (country == "GB" || country == "ES" || country == "US" || country == "CA") 
			{
				//Save player
				playersInMMTCP.push_back(ind);
				playersInMMUDP.push_back(aux);

				SendMessagesTCP(ind, "siz03");
				SendMessagesTCP(ind, "goG");
				if (playersInMMTCP.size() == 2)
				{
					matchmakingTimer = 0;
					startMatchmaking = true;
				}
			}
			else
			{
				SendMessagesTCP(ind, "siz03");
				SendMessagesTCP(ind, "nco");
			}
		}
		else
		{
			SendMessagesTCP(ind, "siz03");
			SendMessagesTCP(ind, "del");
		}
		netManager->messageSizeTCP[ind] = 5;
	}
	else if (type == "out")
	{
		match = false;
		//Send to go out to the other player
		if (ind == netManager->playersInGameTCP.front())
		{
			SendMessagesTCP(netManager->playersInGameTCP.back(), "siz03");
			SendMessagesTCP(netManager->playersInGameTCP.back(), messag);
		}
		else
		{
			SendMessagesTCP(netManager->playersInGameTCP.front(), "siz03");
			SendMessagesTCP(netManager->playersInGameTCP.front(), messag);
		}

		//All players left the server
		match = false;
		playersInGameUDP.clear();
		playersInGameTCP.clear();

		messagesBoss.push(messag);
		messagesSenderBoss.push(ind);

		printf("Server vacio Tamaño actual: %s\n", playersInGameUDP.size());
		netManager->messageSizeTCP[ind] = 5;
	}
	else if (type == "dam")
	{
		netManager->messageSizeTCP[ind] = 5;

		messagesBoss.push(messag);
		messagesSenderBoss.push(ind);

		//std::cout << "Damage from player with ID: " << ind << std::endl;
	}
	else if (type == "pkg")
	{
		if (ind == netManager->playersInGameTCP.front())
		{
			SendMessagesTCP(netManager->playersInGameTCP.back(), "siz05");
			SendMessagesTCP(netManager->playersInGameTCP.back(), messag);
		}
		else
		{
			SendMessagesTCP(netManager->playersInGameTCP.front(), "siz05");
			SendMessagesTCP(netManager->playersInGameTCP.front(), messag);
		}
		netManager->messageSizeTCP[ind] = 5;
	}
	else if (type == "siz")
	{
		netManager->messageSizeTCP[ind] = (messag[3] - '0') * 10 + (messag[4] - '0');
	}
	else if (type == "lmm")
	{
		//Remove from the TCP part
		int posDel = 0;
		for (auto it = netManager->playersInMMTCP.begin(); it != netManager->playersInMMTCP.end(); ++it) {
			if (*it == ind) {
				it = netManager->playersInMMTCP.erase(it);
				break; 
			}
			posDel++;
		}
		//Remove from the UDP part
		auto it = netManager->playersInMMUDP.begin();
		std::advance(it, posDel);
		netManager->playersInMMUDP.erase(it);

		if (netManager->playersInMMTCP.size() < 2)
		{
			matchmakingTimer = 0;
			startMatchmaking = false;
		}

		netManager->messageSizeTCP[ind] = 5;
	}
	else if (type == "dis")
	{
		netManager->clientTCP[ind] = NULL;
		netManager->messageSizeTCP[ind] = 5;
	}
	else if (type == "cht")
	{
		std::string message = findSubstr(messag, netManager->messageSizeTCP[ind]);

		int size = (message[12] - '0') * 10 + (message[13] - '0');

		std::string text = "";
		for (int i = 0; i < 14 + size; i++)
		{
			text += message[i];
		}

		std::string hashHereText = NetworkManager::Instance()->hashString(text);
		int sizeH = (message[14 + size] - '0') * 10 + (message[15 + size] - '0');

		std::string hashText = "";
		for (int i = 16 + size; i < sizeH + 16 + size; i++)
		{
			hashText += message[i];
		}

		//std::cout << "Message: " << text << std::endl;
		//std::cout << "Message hashed from the client: " << hashText << std::endl;
		//std::cout << "Message hashed in the server  : " << hashHereText << std::endl;


		if (hashText == hashHereText)
		{
			SendMessageAllTCP(findSubstr(messag, netManager->messageSizeTCP[ind]), ind);
			netManager->messageSizeTCP[ind] = 5;
		}
	}
}

void NetworkManager::ManageMessageUDP()
{

	std::string type = findSubstr((char*)netManager->p->data, 3);

	if (type == "pos"  && match)
	{
		ConnectionData aux;
		aux.host = netManager->p->address.host;
		aux.port = netManager->p->address.port;

		if (aux == playersInGameUDP.front()) //Send data to the other player
		{
			aux.host = playersInGameUDP.back().host;
			aux.port = playersInGameUDP.back().port;
			SendMessagesUDP(aux, (char*)netManager->p->data);
		}
		else
		{
			aux.host = playersInGameUDP.front().host;
			aux.port = playersInGameUDP.front().port;
			SendMessagesUDP(aux, (char*)netManager->p->data);
		}
	}
}

//Translate the IP received 
Uint32 NetworkManager::swapEndian(Uint32 value) {
	return ((value >> 24) & 0xFF) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | ((value << 24) & 0xFF000000);
}

//Use the IP translated to the save it
Uint32 NetworkManager::GetIPToUint32(const std::string& ipAddress) {
	std::vector<std::string> octets;
	std::stringstream ss(ipAddress);
	std::string octet;

	//Get each number
	while (std::getline(ss, octet, '.')) {
		octets.push_back(octet);
	}

	Uint32 address = 0;
	for (int i = 0; i < 4; ++i) {
		Uint32 value = std::stoi(octets[i]);
		if (value < 0 || value > 255) {
			return 0;
		}
		address |= (value << ((3 - i) * 8)); // |= OR operation of bits
	}

	return address;
}

void NetworkManager::SendMessagesUDP(ConnectionData data, std::string message)
{
	memset(netManager->p->data, '-', MESSAGESIZE);
	memcpy(netManager->p->data, message.c_str(), std::min(message.size(), (size_t)MESSAGESIZE));
	netManager->p->address.host = data.host;
	netManager->p->address.port = data.port;
	if (SDLNet_UDP_Send(netManager->sd, -1, netManager->p) == 0) /* This sets the p->channel */
	{
		printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
	}
}

void NetworkManager::SendMessageAllUDP(std::string message)
{
	for (auto it = playersInGameUDP.begin(); it != playersInGameUDP.end(); ++it)
	{
		ConnectionData aux;
		aux.host = it->host;
		aux.port = it->port;
		SendMessagesUDP(aux, message);
	}

}

void NetworkManager::SendMessagesUPDInGame(std::string message)
{
	SendMessagesUDP(netManager->playersInGameUDP.front(), message);
	SendMessagesUDP(netManager->playersInGameUDP.back(), message);
}

std::list<int> NetworkManager::getPlayersTCPID()
{
	return netManager->playersInGameTCP;
}

std::queue<std::string> NetworkManager::getBossMessages()
{
	std::queue<std::string> aux = messagesBoss;
	while (!messagesBoss.empty()) messagesBoss.pop();
	return aux;
}

std::queue<int> NetworkManager::getBossSenderMessages()
{
	std::queue<int> aux = messagesSenderBoss;
	while (!messagesSenderBoss.empty()) messagesSenderBoss.pop();
	return aux;
}

void NetworkManager::ClearGame()
{
	match = false;
	playersInGameUDP.clear();
	playersInGameTCP.clear();

	printf("Server vacio Tamaño actual: %s\n", playersInGameUDP.size());
}

bool NetworkManager::inGame()
{
	return match;
}

void NetworkManager::Release()
{
	SDLNet_FreeSocketSet(netManager->sSet);
	for (int i = 0; i < NUMCLIENTS; ++i) {
		if (netManager->clientTCP[i] == NULL) continue;
		SDLNet_TCP_Close(netManager->clientTCP[i]);
	}
	SDLNet_TCP_Close(netManager->serverTCP);
	SDLNet_UDP_Close(netManager->sd);

	SDLNet_FreePacket(netManager->p);
	SDLNet_Quit();

	delete netManager;
	netManager = nullptr;
}

NetworkManager::~NetworkManager()
{

}