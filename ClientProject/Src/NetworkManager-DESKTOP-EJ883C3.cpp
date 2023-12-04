#include "NetworkManager.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>

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

	//----------TCP CONEXION-----------------------
	// Resolve the host we are connecting to 
	if (SDLNet_ResolveHost(&netManager->srvaddTCP, SERVERIP, SERVERPORT_TCP) < 0)
	{
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	// Conecting to the server
	if (!(netManager->serverTCP = SDLNet_TCP_Open(&netManager->srvaddTCP)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	// Make TCP non blocking (The select that SDL library offer)
	netManager->sSet = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(netManager->sSet, netManager->serverTCP);

	//----------UDP CONEXION-----------------------
	// Open a socket on random port 
	if (!(netManager->sdUDP = SDLNet_UDP_Open(0)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		SDLNet_Quit();
		delete netManager;
		netManager = nullptr;
		return false;
	}

	// Resolve server name
	if (SDLNet_ResolveHost(&netManager->srvaddUDP, SERVERIP, SERVERPORT_UDP))
	{
		fprintf(stderr, "SDLNet_ResolveHost(%s %d): %s\n", SERVERIP, SERVERPORT_UDP), SDLNet_GetError();
		SDLNet_Quit();
		delete netManager;
		netManager = nullptr;
		return false;
	}

	// Allocate memory for the packet 
	if (!(netManager->pUDP = SDLNet_AllocPacket(MESSAGESIZE)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		SDLNet_Quit();
		delete netManager;
		netManager = nullptr;
		return false;
	}

	//Create the system to manage messages
	for (int i = 0; i < ChatL + 1; i++)
	{
		std::queue<std::string> queue;
		netManager->messages.push_back(queue);
	}

	//Making sure I get the country package lost
	do
	{
		netManager->country = netManager->GetIPCountry();
	} while (netManager->country == "--");

	std::cout << "Your country: " << netManager->country << std::endl;

	return true;
}

//Receive the new messages
void NetworkManager::CheckMessages()
{
	CheckMessagesTCP();
	CheckMessagesUDP();
}

void NetworkManager::CleanGameMessages()
{
	while (!netManager->messages[PlayerL].empty()) netManager->messages[PlayerL].pop();
	while (!netManager->messages[EnemyL].empty()) netManager->messages[EnemyL].pop();
	while (!netManager->messages[BossL].empty()) netManager->messages[BossL].pop();
}

void NetworkManager::SendMessageTCP(std::string message)
{
	char* bufferTCP = new char[message.size()];
	memcpy(bufferTCP, message.c_str(), message.size());
	//std::cout << "Send message: " << bufferTCP << std::endl;
	if (SDLNet_TCP_Send(netManager->serverTCP, (void*)bufferTCP, message.size()) < message.size())
	{
		fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
	}

	delete[] bufferTCP;
}

void NetworkManager::SendMessageUDP(std::string message)
{
	// Copy the line into the buffer, filling the rest with dashes.
	memset(netManager->pUDP->data, '-', MESSAGESIZE);
	size_t size = message.size();
	if (size > MESSAGESIZE)
	{
		size = (size_t)MESSAGESIZE;
	}
	memcpy(netManager->pUDP->data, message.c_str(), size);

	netManager->pUDP->address.host = netManager->srvaddUDP.host; /* Set the destination host */
	netManager->pUDP->address.port = netManager->srvaddUDP.port; /* And destination port */

	//printf("SDLNet_UDP_Send: %s  &x\n", netManager->pUDP->address.host, netManager->pUDP->address.port);

	netManager->pUDP->len = MESSAGESIZE;
	if (SDLNet_UDP_Send(netManager->sdUDP, -1, netManager->pUDP) == 0) /* This sets the p->channel */
	{
		printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
	}
}

std::string NetworkManager::getUDPPort()
{
	IPaddress* aux = SDLNet_UDP_GetPeerAddress(netManager->sdUDP, -1);

	// Transform port to 5 characters (No port of 6 characters)
	std::stringstream ss;
	ss << std::setw(5) << std::setfill('0') << std::to_string(aux->port);
	std::string port = ss.str();

	return port;
}

void NetworkManager::CheckMessagesTCP()
{
	//SELECT
	if (SDLNet_CheckSockets(netManager->sSet, 0) > 0 && SDLNet_SocketReady(netManager->serverTCP))
	{
		char* bufferTCP = new char[netManager->messageSizeTCP];

		if (SDLNet_TCP_Recv(netManager->serverTCP, bufferTCP, netManager->messageSizeTCP) > 0)
		{
			//std::cout << bufferTCP << std::endl;
			ManageMessages(bufferTCP, true);
		}

		delete[] bufferTCP;
	}
}

void NetworkManager::CheckMessagesUDP()
{
	if (SDLNet_UDP_Recv(netManager->sdUDP, netManager->pUDP))
	{
		ManageMessages((char*)netManager->pUDP->data, false);
	}
}

std::string NetworkManager::findSubstr(std::string const& str, int n) {
	if (str.length() < n) {
		return str;
	}

	return str.substr(0, n);
}

void NetworkManager::ManageMessages(char* messag, bool tcp)
{
	std::string type = findSubstr(messag, 3);

	std::string message;
	if(tcp)
		message = findSubstr(messag, netManager->messageSizeTCP);
	else
		message = messag;

	if (type == "sta" || type == "los" || type == "lgb" || type == "pkg") //Start the game || Lost the game || Recover life (bullet didnt exist anymore) || % udp packages lost
	{
		netManager->messages[PlayerL].push(message);
	}
	if (type == "sta" || type == "pos" || type == "dam" || type == "itE") // Start the game || Position movement || Receive damage
	{
		netManager->messages[EnemyL].push(message);
	}
	if (type == "goG" || type == "nco" || type == "del") // Go game scene || No in country || Room is full
	{
		netManager->messages[MenuL].push(message);
	}
	if (type == "sta" || type == "out" || type == "win") // Start game || Leave room disconected || Win game
	{
		netManager->messages[GameL].push(message);
	}
	if (type == "sta" || type == "bpo" || type == "bul") // Start the game || Boss movement || Spawn bullet
	{
		netManager->messages[BossL].push(message);
	}
	if (type == "cht")
	{
		netManager->messages[ChatL].push(message);
	}
	if (type == "siz")
	{
		messageSizeTCP = (message[3] - '0') * 10 + (message[4] - '0');
	}

	if (tcp && type != "siz") //resetMessage size
	{
		messageSizeTCP = 5;
	}
}


std::queue<std::string> NetworkManager::getMessages(Listener type)
{
	std::queue<std::string> aux = netManager->messages[type];
	while (!netManager->messages[type].empty()) netManager->messages[type].pop();
	return aux;
}

size_t NetworkManager::GetInfo(void* content, size_t size, size_t nmemb, std::string* output) {
	size_t totalSize = size * nmemb;
	output->append(static_cast<char*>(content), totalSize);
	return totalSize;
}

std::string NetworkManager::GetIPCountry() {
	std::string sol;

	return "GB";

	CURL* curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "ipinfo.io/country?token=9df91f2f5e395c");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetInfo);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sol);

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
	}

	if (sol.size() < 2) //Check error
	{
		sol = "--";
	}
	return sol;
}

std::string NetworkManager::getCountry()
{
	return netManager->country;
}

void NetworkManager::Release()
{
	SDLNet_FreeSocketSet(netManager->sSet);
	SDLNet_UDP_Close(netManager->sdUDP);
	SDLNet_FreePacket(netManager->pUDP);
	SDLNet_Quit();

	delete netManager;
	netManager = nullptr;
}

NetworkManager::~NetworkManager()
{

}