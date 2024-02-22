#pragma once

#include "../includes/matt_daemon.hpp"

extern Tintin_reporter logger;

class Server {
public:
	Server(uint16_t port) : _port(port) {}

	~Server() {
		deleteVector(&_clients);
		deleteVector(&_clientsToDelete);
		close(_serverSocket);
		close(_epollFd);
	}

	void init() {
		std::memset(&_serverAddress, 0, sizeof(_serverAddress));
		_serverAddress.sin_family = AF_INET;
		_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		_serverAddress.sin_port = htons(_port);
		syscall(_serverSocket = socket(_serverAddress.sin_family, SOCK_STREAM, 0),
				"socket");
		fcntl(_serverSocket, F_SETFL, O_NONBLOCK);
		_reuseAddr = 1;
		syscall(setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &_reuseAddr,
						   sizeof(_reuseAddr)),
				"setsockopt");
		syscall(bind(_serverSocket, (const struct sockaddr*)&_serverAddress,
					 sizeof(_serverAddress)),
				"bind");
		syscall(listen(_serverSocket, BACKLOG), "listen");
		struct epoll_event ev;
		ev.data.fd = _serverSocket;
		ev.events = EPOLLIN;
		syscall(_epollFd = epoll_create1(0), "epoll_create1");
		syscall(epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverSocket, &ev), "epoll_ctl");
	}

	void loop() {
		while (true) {
			int nfds;
			syscall(nfds = epoll_wait(_epollFd, _eventList, MAX_CLIENTS, -1),
					"epoll_wait");
			for (int i = 0; i < nfds; ++i) {
				if (_eventList[i].data.fd == _serverSocket) {
					this->acceptNewClient();
				} else {
					Client* client = getClient(_eventList[i].data.fd);
					if (client != NULL) {
						if (_eventList[i].events & EPOLLIN) readFromClient(client);
						else if (_eventList[i].events & (EPOLLRDHUP | EPOLLHUP))
							removeClient(client);
					}
				}
			}
			deleteVector(&_clientsToDelete);
		}
	}

	Client* getClient(int socketFd) const {
		for (std::vector<Client*>::const_iterator it = _clients.begin();
			 it != _clients.end(); ++it) {
			if ((*it)->getSocket() == socketFd) return (*it);
		}
		return (NULL);
	}

	static const size_t maxClients = 3;

private:
	int _serverSocket;
	int _epollFd;
	int _reuseAddr;
	struct sockaddr_in _serverAddress;
	struct epoll_event _eventList[MAX_CLIENTS];
	uint16_t _port;

	std::vector<Client*> _clients;
	std::vector<Client*> _clientsToDelete;

	void acceptNewClient() {
		struct sockaddr_in clientAddress;
		socklen_t addressLen = sizeof(clientAddress);
		int clientSocket =
			accept(_serverSocket, (struct sockaddr*)&clientAddress, &addressLen);
		syscall(clientSocket, "accept");
		Client* client = new Client(clientSocket, clientAddress);
		if (_clients.size() >= Server::maxClients) {
			logger.log(LogLevel::ERROR,
					   "[%s] The server is being DDOSed. Connection refused.",
					   client->getFullAddress().c_str());
			delete client;
		} else {
			_clients.push_back(client);
			syscall(fcntl(clientSocket, F_SETFL, O_NONBLOCK), "fcntl");
			struct epoll_event ev;
			ev.data.fd = clientSocket;
			ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
			syscall(setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &_reuseAddr,
							   sizeof(_reuseAddr)),
					"setsockopt");
			syscall(epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSocket, &ev), "epoll_ctl");
			logger.log(LogLevel::INFO, "[%s] Client connected",
					   client->getFullAddress().c_str());
		}
	}

	void readFromClient(Client* client) {
		static const size_t bufferSize = 1024;
		char buffer[bufferSize] = {0};
		std::string message;

		int recvSize;
		do {
			recvSize = recv(client->getSocket(), buffer, bufferSize - 1, 0);
			if (recvSize == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					break;
				} else {
					syscall(recvSize, "recv");
				}
			} else if (recvSize > 0) {
				buffer[recvSize] = '\0';
				message += buffer;
			}
		} while (recvSize == bufferSize - 1);

		if (message.empty()) return removeClient(client);

		size_t pos;
		while ((pos = message.find("\n")) != std::string::npos) {
			std::string line = message.substr(0, pos);
			message = message.substr(pos + 1);
			handleMessage(client, line);
		}
		if (!message.empty()) handleMessage(client, message);
	}

	void handleMessage(Client* client, const std::string& message) {
		(void)client;
		std::string trimmedMessage = trimNewlines(message);
		std::string lowerMessage = toLowerCase(trimmedMessage);
		if (lowerMessage == "quit") exitWithLog(EXIT_SUCCESS);
		logger.log(LogLevel::LOG, "[%s] %s", client->getFullAddress().c_str(),
				   trimmedMessage.c_str());
	}

	void removeClient(Client* client) {
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, client->getSocket(), NULL);
		close(client->getSocket());
		for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end();
			 ++it) {
			if ((*it) == client) {
				logger.log(LogLevel::INFO, "[%s] Client disconnected",
						   (*it)->getFullAddress().c_str());
				_clients.erase(it);
				_clientsToDelete.push_back(client);
				break;
			}
		}
	}
};
