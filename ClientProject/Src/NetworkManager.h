#pragma once
#include <string>
#include <queue>
#include <SDL_net.h>
#include <curl/curl.h>
#include "checkML.h"

#define SERVERIP "127.0.0.1"
#define SERVERPORT_UDP 1234
#define SERVERPORT_TCP 1235

#define MESSAGESIZE 70

//Enum of entitities that can receive messages
enum Listener { PlayerL, EnemyL, MenuL, GameL, BossL, ChatL };

class NetworkManager
{
private:
	~NetworkManager();
	void CheckMessagesUDP();
	void CheckMessagesTCP();

	void ManageMessages(char* message, bool tcp);

	//----------GET LOCATION-----------
	std::string GetIPCountry();
	static size_t GetInfo(void* contents, size_t size, size_t nmemb, std::string* output);
	//---------------------------------

	UDPsocket sdUDP;
	IPaddress srvaddUDP;
	UDPpacket* pUDP;

	TCPsocket serverTCP;
	IPaddress srvaddTCP;
	SDLNet_SocketSet sSet;
	char buffer[MESSAGESIZE];

	int messageSizeTCP = 5;

	std::vector<std::queue<std::string>> messages;

	std::string country = "--";
	std::string myIP = "";
protected:
	NetworkManager();

	static NetworkManager* netManager;
public:

	static bool Init();

	std::string getMyIp();

	static NetworkManager* Instance();

	void CheckMessages();

	void CleanGameMessages();

	void SendMessageUDP(std::string message);
	void SendMessageTCP(std::string message);

	std::string hashString(std::string message);

	std::string getCountry();

	std::string findMyIP();

	static void Release();

	std::string getUDPPort();

	std::string findSubstr(std::string const& str, int n);

	std::queue<std::string> getMessages(Listener type);

};
