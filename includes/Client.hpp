#pragma once

#include "../includes/matt_daemon.hpp"

class Client {
public:
	Client(Server* server, int socketFd, sockaddr_in clientAddress)
		: _clientSocket(socketFd), _clientAddress(clientAddress), _server(server) {}

	~Client() { close(_clientSocket); }

	int getSocket() const { return _clientSocket; }

	void reply(std::string replyMessage) {
		replyMessage += "\n";
		send(_clientSocket, replyMessage.c_str(), replyMessage.length(), MSG_NOSIGNAL);
	}

	std::string _message;

private:
	int _clientSocket;
	sockaddr_in _clientAddress;
	Server* _server;
};
