#pragma once

#include "../includes/matt_daemon.hpp"

class Server {
public:
	Server(uint16_t port);
	~Server();

	void init();
	void loop();

	Client* getClient(int socketFd) const;
	std::vector<Client*> getClients() const;
	uint16_t getPort() const;

private:
	int _serverSocket;
	int _epollFd;
	int _reuseAddr;
	struct sockaddr_in _serverAddress;
	struct epoll_event _eventList[MAX_CLIENTS];
	uint16_t _port;

	std::vector<Client*> _clients;
	std::vector<Client*> _clientsToDelete;

	void acceptNewClient();
	void readFromClient(Client* client);
	void handleMessage(Client* client, std::string command);
	void removeClient(Client* client);
};
