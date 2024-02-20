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
		int newClientSocket;
		struct sockaddr_in newClientAddress;
		socklen_t newClientAddressLen = sizeof(newClientAddress);

		syscall(newClientSocket =
					accept(_serverSocket, (struct sockaddr*)&newClientAddress,
						   (socklen_t*)&newClientAddressLen),
				"accept");
		_clients.push_back(new Client(this, newClientSocket, newClientAddress));
		syscall(fcntl(newClientSocket, F_SETFL, O_NONBLOCK), "fcntl");
		struct epoll_event ev;
		ev.data.fd = newClientSocket;
		ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
		syscall(setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &_reuseAddr,
						   sizeof(_reuseAddr)),
				"setsockopt");
		syscall(epoll_ctl(_epollFd, EPOLL_CTL_ADD, newClientSocket, &ev), "epoll_ctl");
	}

	void readFromClient(Client* client) {
		char buffer[BUFFER_SIZE] = {0};
		int recvSize;

		do {
			syscall(recvSize = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0),
					"recv");
			buffer[recvSize] = '\0';
			client->_message += buffer;
		} while (recvSize == BUFFER_SIZE - 1);

		if (client->_message.empty()) return removeClient(client);

		size_t pos;
		while ((pos = client->_message.find("\n")) != std::string::npos) {
			std::string line = client->_message.substr(0, pos);
			client->_message = client->_message.substr(pos + 1);
			handleMessage(client, line);
		}
		if (!client->_message.empty()) {
			handleMessage(client, client->_message);
			client->_message.clear();
		}
	}

	void handleMessage(Client* client, const std::string& message) {
		(void)client;
		std::string trimmedMessage = trimNewlines(message);
		std::string lowerMessage = toLowerCase(trimmedMessage);
		if (lowerMessage == "quit") throw TerminateSuccess();
		// TODO: authenticate, encrypton, encryptoff, rshon, rshoff...
		logger.log(LogLevel::LOG, trimmedMessage.c_str());
	}

	void removeClient(Client* client) {
		client->_message = "";

		epoll_ctl(_epollFd, EPOLL_CTL_DEL, client->getSocket(), NULL);
		close(client->getSocket());

		for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end();
			 ++it) {
			if ((*it) == client) {
				_clients.erase(it);
				_clientsToDelete.push_back(client);
				break;
			}
		}
	}
};
