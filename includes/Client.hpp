#pragma once

#include "../includes/matt_daemon.hpp"

class Client {
public:
	Client(Server* server, int socketFd, sockaddr_in clientAddress);
	~Client();

	int getSocket() const;
	sockaddr_in getClientAddress() const;
	Server* getServer() const;
	void reply(std::string replyMessage) const;

	std::string _message;

private:
	int _clientSocket;
	sockaddr_in _clientAddress;
	Server* _server;
};
