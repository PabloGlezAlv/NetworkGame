#pragma once
#include <string>
#include <SDL_net.h>
#include <List>
#include <queue>
#include <string>

#define SERVERPORT_UDP 1234
#define SERVERPORT_TCP 1235

#define MESSAGESIZE 99

#define NUMCLIENTS 20

#define MATCHMAKING_TIME 2

struct ConnectionData
{
	Uint32 host;
	Uint16 port;
	std::string country;

	bool operator==(ConnectionData a) {
		return (host == a.host && port == a.port);
	}
};

class NetworkManager
{
private:
	~NetworkManager();

	bool AcceptSocket(int index);
	void ManageMessageUDP();
	void ManageMessageTCP(int ind, char* messag);

	std::string hashString(std::string message);

	Uint32 GetIPToUint32(const std::string& ipAddress);
	Uint32 swapEndian(Uint32 value);

	UDPsocket sd = NULL; /* Socket descriptor */
	UDPpacket* p = nullptr; /* Pointer to packet memory */

	int nClients = 0;
	TCPsocket serverTCP = NULL;
	TCPsocket clientTCP[NUMCLIENTS];
	int messageSizeTCP[NUMCLIENTS];
	SDLNet_SocketSet sSet;
	IPaddress ip;

	char buffer[MESSAGESIZE];

	//Player in the game

	std::list<ConnectionData> playersInMMUDP;
	std::list<int> playersInMMTCP;

	std::list<ConnectionData> playersInGameUDP;
	std::list<int> playersInGameTCP;

	std::queue<std::string> messagesBoss;
	std::queue<int> messagesSenderBoss;

	bool match = false;

	bool startMatchmaking = 0;
	float matchmakingTimer = 0;
protected:
	NetworkManager();

	static NetworkManager* netManager;
public:

	static bool Init();

	static NetworkManager* Instance();

	void CheckMessagesUDP();
	void CheckMessagesTCP();

	void Update(float deltaTime);

	void SendMessagesTCP(int ind, std::string message);
	void SendMessageAllTCP(std::string message, int sender);
	void SendMessagesTCPInGameAll(std::string message);
	void SendMessagesTCPInGame(int ind, std::string message);

	void SendMessagesUDP(ConnectionData data, std::string message);
	void SendMessageAllUDP(std::string message);
	void SendMessagesUPDInGame(std::string message);

	std::list<int> getPlayersTCPID();

	void ClearGame();

	bool inGame();

	static void Release();

	std::string findSubstr(std::string const& str, int n);

	std::queue<std::string> getBossMessages();
	std::queue<int> getBossSenderMessages();


};
