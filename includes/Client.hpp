#pragma once

#include "../includes/matt_daemon.hpp"

class Client {
public:
	Client(Server* server, int socketFd, sockaddr_in clientAddress)
		: _clientSocket(socketFd), _clientAddress(clientAddress), _server(server) {
		char clientInfo[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(_clientAddress.sin_addr), clientInfo, INET_ADDRSTRLEN);
		int clientPort = ntohs(_clientAddress.sin_port);
		_fullAddress = std::string(clientInfo) + ":" + toString(clientPort);
	}

	~Client() { close(_clientSocket); }

	int getSocket() const { return _clientSocket; }

	void reply(std::string replyMessage) {
		replyMessage += "\n";
		send(_clientSocket, replyMessage.c_str(), replyMessage.length(), MSG_NOSIGNAL);
	}

	const std::string& getFullAddress() const { return _fullAddress; }

	std::string _message;

private:
	int _clientSocket;
	sockaddr_in _clientAddress;
	std::string _fullAddress;
	Server* _server;
};
